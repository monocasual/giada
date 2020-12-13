#!/usr/bin/env bash

set -e

echo "Create working dirs"

mkdir dist
mkdir temp

echo "Copy binary and dll files to temp/"

cp build/Release/giada.exe build/Release/*.dll temp/

echo "Make zip archive, save it to to dist/"

# Note: $RELEASE_VERSION is an environment variable set in the
# packaging.yml script.

7z a -tzip dist/giada-$RELEASE_VERSION-x86_64-windows.zip ./temp/*