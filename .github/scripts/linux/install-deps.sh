#!/usr/bin/env bash

set -e

# Install apt dependencies 

sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y 
sudo apt-get update -qq
sudo apt-get install g++-10 xvfb libasound2-dev libxpm-dev libpulse-dev libjack-dev libxft-dev libxrandr-dev libx11-dev libxinerama-dev libxcursor-dev libfontconfig1-dev

# Symlink gcc in order to use the latest version

sudo ln -f -s /usr/bin/g++-10 /usr/bin/g++

# Download linuxdeploy for building AppImages

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20230713-1/linuxdeploy-x86_64.AppImage
chmod a+x linuxdeploy-x86_64.AppImage