#!/usr/bin/env bash

set -e

# TODO - libsamplerate doesn't work on macOS + vcpkg yet. 

brew update > /dev/null
brew install libsamplerate
brew install upx

# Remove dynamic libraries to force static linking.

rm -rf /usr/local/lib/libsamplerate*dylib

# Install vcpkg dependencies

vcpkg install libsndfile
vcpkg install fltk
vcpkg install rtmidi