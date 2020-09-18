#!/usr/bin/env bash

set -e

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	make -C build/ -j 2
	#TODO tests
fi

if [[ $TRAVIS_OS_NAME == 'linux' ]]; then
	
	make -C build/ -j 2
	#TODO tests
fi