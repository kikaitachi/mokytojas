#!/usr/bin/env bash

# Exit when any command fails
set -e

# Update to latest version
git pull

# Initialize build if it was never done before
[[ -d builddir ]] || meson builddir

# Build
meson subprojects update
ninja -C builddir

builddir/mokytojas 7211 7210

