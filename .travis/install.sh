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

elif [[ $TRAVIS_OS_NAME == 'windows' ]]; then

	# Install Visual Studio 2019 build tools

	choco install -y visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64"

	# Install vcpkg + Giada dependencies

	git clone https://github.com/Microsoft/vcpkg.git
	cd vcpkg
	./bootstrap-vcpkg.bat
	./vcpkg integrate install

	./vcpkg install libsndfile:x64-windows
	./vcpkg install libsamplerate:x64-windows
	./vcpkg install fltk:x64-windows
	./vcpkg install rtmidi:x64-windows

	# For some reason CMake grabs the debug build of RtMidi library (located in 
	# /c/vcpkg/installed/x64-windows/debug) instead of the release build located 
	# in /c/vcpkg/installed/x64-windows. This screws up building Giada in 
	# release mode. Workaround: remove the debug build of RtMidi from the folder 
	# above.
	# TODO - do this only in release mode

	rm -rf installed/x64-windows/debug/lib/fltk*
fi