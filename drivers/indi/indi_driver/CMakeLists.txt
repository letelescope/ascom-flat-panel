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
find_package(ZLIB REQUIRED)
find_package(GSL REQUIRED)

# these will be used to set the version number in config.h and our driver's xml file
set(CDRIVER_VERSION_MAJOR 2)
set(CDRIVER_VERSION_MINOR 0)

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
    ${GSL_LIBRARIES}
)

# tell cmake where to install our executable
install(TARGETS indi_letelescope_fffp_flatpanel RUNTIME DESTINATION bin)

# and where to put the driver's xml file.
install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/indi_letelescope_fffp_flatpanel.xml
    DESTINATION ${INDI_DATA_DIR}
)