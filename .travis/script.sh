#!/usr/bin/env bash

make -j 2
make rename

if [[ $TRAVIS_OS_NAME == 'linux' ]]; then
	
	xvfb-run make check -j 2

else

	make check -j 2

fi