#!/usr/bin/env bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	: # null command - nothing to do 

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y # For more recent GCC versions
  sudo apt-get update -qq

fi