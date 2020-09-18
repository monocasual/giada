#!/usr/bin/env bash

set -e

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	: # null command - nothing to do 

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

	# Add CMake repository

	wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
	sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'

	# For more recent GCC versions

	sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y 

	sudo apt-get update -qq

	sudo apt-get autoremove -y cmake

fi