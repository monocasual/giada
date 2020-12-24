#!/usr/bin/env bash

set -e

# Install dependencies 

sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y 
sudo apt-get update -qq
sudo apt-get install g++-8 xvfb libsndfile1-dev libsamplerate0-dev libasound2-dev libxpm-dev libpulse-dev libjack-dev libxft-dev libxrandr-dev libx11-dev libxinerama-dev libxcursor-dev libfontconfig1-dev libfltk1.3-dev librtmidi-dev

# Make catch2 on the fly

git clone --depth 1 --branch v2.13.3 https://github.com/catchorg/Catch2.git
cd Catch2
cmake -B build/ -H. -DBUILD_TESTING=OFF
sudo cmake --build build/ --target install -j 2
cd ..

# Symlink gcc in order to use the latest version

sudo ln -f -s /usr/bin/g++-8 /usr/bin/g++

# Download linuxdeploy for building AppImages

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod a+x linuxdeploy-x86_64.AppImage