#!/usr/bin/env bash

set -e

# Install apt dependencies 

sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y 
sudo apt-get update -qq
sudo apt-get install xvfb libasound2-dev libxpm-dev libpulse-dev libjack-dev libxft-dev libxrandr-dev libx11-dev libxinerama-dev libxcursor-dev libfontconfig1-dev libgl-dev libfuse2

# Download linuxdeploy for building AppImages

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20230713-1/linuxdeploy-x86_64.AppImage
chmod a+x linuxdeploy-x86_64.AppImage