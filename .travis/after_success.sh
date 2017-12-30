#!/usr/bin/env bash

mkdir build

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

  cp giada_osx ./build
  upx --best ./build/giada_osx

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

	# TODO  
  # cp giada_lin ./build

fi