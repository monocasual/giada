#!/usr/bin/env bash

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	export MACOSX_DEPLOYMENT_TARGET=10.14
	make -j 2 CXXFLAGS+=-stdlib=libc++ 
fi

if [[ $TRAVIS_OS_NAME == 'linux' ]]; then
	
	make -j 2 CXXFLAGS+=-Wno-class-memaccess
	xvfb-run make check -j 2 CXXFLAGS+=-Wno-class-memaccess

else

	make check -j 2

fi

make rename