#!/usr/bin/env bash
set -euo pipefail

# make-release: build and create a DEB package for the indi driver using CPack.
# Usage:
#   ./make-release.sh [build-dir]
# Default build-dir: build

BUILD_DIR=${1:-build}
SRCDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

echo "[make-release] source directory: ${SRCDIR}"
echo "[make-release] build directory: ${BUILD_DIR}"

# Ensure CMake package metadata is available and not stale.
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr "${SRCDIR}"
make -j"$(nproc)"

# Clean old packages that match previous pattern.
rm -f "${SRCDIR}/${BUILD_DIR}"/indi-letelescope-fffp-flatpanel-*.deb

cpack -G DEB

DEBPATH=$(ls -1 "${SRCDIR}/${BUILD_DIR}"/indi-letelescope-fffp-flatpanel-*.deb 2>/dev/null | tail -n 1 || true)
if [[ -z "${DEBPATH}" ]]; then
  echo "[make-release] ERROR: Package not generated"
  exit 1
fi

echo "[make-release] Created package: ${DEBPATH}"

# Optional lintian check, skip if not installed.
if command -v lintian >/dev/null 2>&1; then
  echo "[make-release] Running lintian..."
  lintian "${DEBPATH}" || true
fi

echo "[make-release] Release package ready."
