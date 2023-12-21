#!/usr/bin/env bash

set -e

if [ -z "$1" ]; then
    echo "Usage: make-package.sh [release-version]"
    exit 1
fi

RELEASE_VERSION=$1

echo "Create working dirs"

mkdir dist
mkdir temp

echo "Strip binary and move it to temp/"

strip --strip-all ./build/giada
cp ./build/giada temp/

echo "Prepare .desktop file"

cp extras/com.giadamusic.Giada.desktop temp/giada.desktop

echo "Prepare logo"

cp extras/giada-logo.svg temp/giada.svg

echo "Generate AppImage file"

# Run linuxdeploy to make the AppImage, then move it to dist/ dir. 
# For some reason linuxdeploy uses the commit hash in the filename, so
# rename it first.

./linuxdeploy-x86_64.AppImage -e temp/giada -d temp/giada.desktop -i temp/giada.svg --output appimage --appdir temp/
mv Giada-*-x86_64.AppImage Giada-$RELEASE_VERSION-x86_64.AppImage 
cp Giada-$RELEASE_VERSION-x86_64.AppImage dist/