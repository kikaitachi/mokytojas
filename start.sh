#!/usr/bin/env bash

# Exit when any command fails
set -e

# Update to latest version
git pull

# Initialize build if it was never done before
[[ -d builddir ]] || meson builddir

# Build
ninja -C builddir

# Update address and port of target robot
builddir/mokytojas 192.168.1.4 7252
