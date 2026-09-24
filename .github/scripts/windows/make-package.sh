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

echo "Copy binary and DLLs to temp/"

cp build/Release/giada.exe temp/
cp build/Release/*.dll temp/

# Let's grab extra dll explicitly from the vcpkg installed tree to make sure they
# end up in the package.
cp build/vcpkg_installed/x64-windows/bin/*.dll temp/

echo "Make zip archive, save it to to dist/"

7z a -tzip dist/giada-$RELEASE_VERSION-x86_64-windows.zip ./temp/*