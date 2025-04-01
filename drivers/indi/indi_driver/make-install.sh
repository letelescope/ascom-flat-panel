#!/bin/sh
#
# Build script Indi AuxDevice, LightBox and DustCap driver for LeTelescopeFFFPV1
#
# Copyright(C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
# Licensed under the MIT License. See the accompanying LICENSE file for terms.
#
# Authors:	    Florian Thibaud	
#               Florian Gautier

mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug ../
make
sudo make install