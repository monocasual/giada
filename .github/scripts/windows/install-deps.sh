#!/usr/bin/env bash

# Install Visual Studio 2019 build tools

choco install -y visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64"

# Install vcpkg dependencies

vcpkg install libsndfile:x64-windows
vcpkg install libsamplerate:x64-windows
vcpkg install fltk:x64-windows
vcpkg install rtmidi:x64-windows
vcpkg install catch2:x64-windows