#!/usr/bin/env bash

set -e

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	brew update > /dev/null
	brew install rtmidi
	brew install libsamplerate
	brew install fltk
	brew install libsndfile
	brew install upx

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

	# Install dependencies 

	sudo apt-get install -y cmake g++-8 libsndfile1-dev libsamplerate0-dev \
		libasound2-dev libxpm-dev libpulse-dev libjack-dev \
		libxft-dev libxrandr-dev libx11-dev libxinerama-dev libxcursor-dev \
	libfontconfig1-dev libfltk1.3-dev librtmidi-dev

	# Symlink gcc in order to use the latest version

	sudo ln -f -s /usr/bin/g++-8 /usr/bin/g++

	# Download linuxdeploy for building AppImages.

	wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
	chmod a+x linuxdeploy-x86_64.AppImage

fi