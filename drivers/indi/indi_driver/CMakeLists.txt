#
# Cmake build template of Indi AuxDevice, LightBox and DustCap driver for LeTelescopeFFFPV1
#
# Copyright(C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
# Licensed under the MIT License. See the accompanying LICENSE file for terms.
#
# Authors:	    Florian Thibaud	
#               Florian Gautier

cmake_minimum_required(VERSION 3.28)

# define the project name
project(indi-letelescope-fffp-flatpanel C CXX)


include(GNUInstallDirs)

# add our cmake_modules folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

# find our required packages
find_package(INDI REQUIRED)
find_package(Nova REQUIRED)

# these will be used to set the version number in config.h and our driver's xml file
set(CDRIVER_VERSION_MAJOR 2)
set(CDRIVER_VERSION_MINOR 0)

# package identification for CPack and docs
set(CPACK_PACKAGE_NAME "indi-letelescope-fffp-flatpanel")
set(CPACK_PACKAGE_VERSION_MAJOR ${CDRIVER_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CDRIVER_VERSION_MINOR})

# default INDI data directory for packaging; can be overridden via -DINDI_DATA_DIR=/path
if(NOT DEFINED INDI_DATA_DIR)
    set(INDI_DATA_DIR "/usr/share/indi" CACHE PATH "INDI data directory")
endif()

# do the replacement in the config.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# do the replacement in the driver's xml file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/indi_letelescope_fffp_flatpanel.xml.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/indi_letelescope_fffp_flatpanel.xml
)

# set our include directories to look for header files
include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${INDI_INCLUDE_DIR})
include_directories( ${NOVA_INCLUDE_DIR})
include_directories( ${EV_INCLUDE_DIR})

include(CMakeCommon)

# tell cmake to build our executable
add_executable(
    indi_letelescope_fffp_flatpanel
    indi_letelescope_fffp_flatpanel.cpp
    indi_letelescope_fffp_hardware_adapter.cpp
)

# and link it to these libraries
target_link_libraries(
    indi_letelescope_fffp_flatpanel
    ${INDI_LIBRARIES}
    ${NOVA_LIBRARIES}
)

# tell cmake where to install our executable
install(TARGETS indi_letelescope_fffp_flatpanel RUNTIME DESTINATION bin)

# and where to put the driver's xml file.
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/indi_letelescope_fffp_flatpanel.xml
    DESTINATION ${INDI_DATA_DIR}
)

# include license in .deb under /usr/share/doc/<package>
set(PACKAGE_COPYRIGHT_SOURCE "")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../LICENSE")
    set(PACKAGE_COPYRIGHT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../../LICENSE")
endif()

if(PACKAGE_COPYRIGHT_SOURCE)
    install(
        FILES "${PACKAGE_COPYRIGHT_SOURCE}"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${CPACK_PACKAGE_NAME}"
        RENAME "copyright"
    )
else()
    message(WARNING "Cannot find LICENSE file for package copyright install; create one in project root or driver folder")
endif()

# install static changelog from project root (avoid on-the-fly generation complexity)
set(CPACK_CHANGELOG_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/changelog")

if(NOT EXISTS "${CPACK_CHANGELOG_SOURCE}")
    message(FATAL_ERROR "changelog file not found: ${CPACK_CHANGELOG_SOURCE}")
endif()

execute_process(COMMAND sh -c "gzip -9 -c '${CPACK_CHANGELOG_SOURCE}' > '${CMAKE_BINARY_DIR}/changelog.gz'")
install(
    FILES "${CMAKE_BINARY_DIR}/changelog.gz"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/${CPACK_PACKAGE_NAME}"
)

# CPack support for DEB package
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "indi-letelescope-fffp-flatpanel")
set(CPACK_PACKAGE_VERSION_MAJOR ${CDRIVER_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CDRIVER_VERSION_MINOR})
set(CPACK_PACKAGE_CONTACT "florian@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "INDI driver for Le Telescope FFFP flat panel")
set(CPACK_PACKAGE_DESCRIPTION "LeTelescope FFFP flat panel driver built as an INDI auxiliary device")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Florian Gautier <fgautier.mail@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "indi, libnova4, libgsl23, zlib1g, libc6, libstdc++6")
set(CPACK_STRIP_FILES "/usr/bin/indi_letelescope_fffp_flatpanel")

include(CPack)
