#!/usr/bin/env bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	export MACOSX_DEPLOYMENT_TARGET=10.2
	
fi

make -j 2
make rename

if [[ $TRAVIS_OS_NAME == 'linux' ]]; then
	
	xvfb-run make check -j 2

else

	make check -j 2

fi