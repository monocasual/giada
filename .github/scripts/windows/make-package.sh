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

echo "Copy binary file to temp/"

cp build/Release/giada.exe temp/

echo "Make zip archive, save it to to dist/"

7z a -tzip dist/giada-$RELEASE_VERSION-x86_64-windows.zip ./temp/*