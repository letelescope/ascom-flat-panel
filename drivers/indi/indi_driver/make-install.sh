#!/usr/bin/env bash
set -euo pipefail

# Build script Indi AuxDevice, LightBox and DustCap driver for LeTelescopeFFFPV1
# (safe guard for non-root package build/install path)
#
# Copyright(C) 2025 - Present, Le Télescope - Ivry sur Seine - All Rights Reserved
# Licensed under the MIT License. See the accompanying LICENSE file for terms.
#
# Authors:	    Florian Thibaud	
#               Florian Gautier

BUILD_DIR=${1:-build}
SRCDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

echo "[make-install] source directory: ${SRCDIR}"
echo "[make-install] build directory: ${BUILD_DIR}"

mkdir -p "${SRCDIR}/${BUILD_DIR}"
cd "${SRCDIR}/${BUILD_DIR}"

cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug "${SRCDIR}"
make -j"$(nproc)"

if [[ $EUID -ne 0 ]]; then
  echo "[make-install] Not root, using sudo for install"
  sudo make install
else
  make install
fi
