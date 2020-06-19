#!/usr/bin/env bash

./autogen.sh

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

  ./configure --target=osx --enable-vst

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

  ./configure --target=linux --enable-vst

fi