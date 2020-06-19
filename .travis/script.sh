#!/usr/bin/env bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	export MACOSX_DEPLOYMENT_TARGET=10.14
	make -j 2 CXXFLAGS+=-stdlib=libc++ 
fi

make rename

if [[ $TRAVIS_OS_NAME == 'linux' ]]; then
	
	make -j 2
	xvfb-run make check -j 2

else

	make check -j 2

fi