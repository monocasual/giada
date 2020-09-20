#!/usr/bin/env bash

set -e

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	brew update > /dev/null
	brew install rtmidi
	brew install libsamplerate
	brew install fltk
	brew install libsndfile
	brew install upx

	# Remove dynamic libraries to force static linking.

	rm -rf /usr/local/lib/librtmidi*dylib
	rm -rf /usr/local/lib/libsamplerate*dylib
	rm -rf /usr/local/lib/libfltk*dylib
	rm -rf /usr/local/lib/libfltk_forms*dylib
	rm -rf /usr/local/lib/libfltk_gl*dylib
	rm -rf /usr/local/lib/libfltk_images*dylib
	rm -rf /usr/local/lib/libsndfile*dylib 
	rm -rf /usr/local/lib/libFLAC*dylib
	rm -rf /usr/local/lib/libogg*dylib
	rm -rf /usr/local/lib/libopus*dylib
	rm -rf /usr/local/lib/libvorbis*dylib
	rm -rf /usr/local/lib/libvorbisenc*dylib

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