#!/usr/bin/env bash

set -e

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	make -C build/ -j 2
	#TODO tests
fi

if [[ $TRAVIS_OS_NAME == 'linux' ]]; then
	
	make -C build/ -j 2
	if [[ $TRAVIS_TAG == '' ]]; then  # Regular commit, run tests
		xvfb-run ./build/giada --run-tests	
	fi     	
fi