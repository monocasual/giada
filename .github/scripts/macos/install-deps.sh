#!/usr/bin/env bash

set -e

# Install vcpkg dependencies

vcpkg install libsndfile
vcpkg install libsamplerate
vcpkg install fltk
vcpkg install rtmidi
vcpkg install catch2
vcpkg install fmt