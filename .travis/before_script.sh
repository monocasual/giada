#!/usr/bin/env bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

  ./autogen.sh
  ./configure --target=osx --enable-vst MACOSX_DEPLOYMENT_TARGET=10.12

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

  ./autogen.sh
  ./configure --target=linux --enable-vst

fi