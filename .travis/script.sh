#!/usr/bin/env bash

set -e

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	xcodebuild -quiet -project build/giada.xcodeproj -scheme 'giada' -configuration Release build
	#TODO tests
fi

if [[ $TRAVIS_OS_NAME == 'linux' ]]; then
	
	make -C build/ -j 2
	if [[ $TRAVIS_TAG == '' ]]; then  # Regular commit, run tests
		xvfb-run ./build/giada --run-tests	
	fi

elif [[ $TRAVIS_OS_NAME == 'windows' ]]; then

	"/c/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin/MSBuild.exe" -t:giada -maxCpuCount -p:CL_MPCount=2  -p:Platform=x64 -p:Configuration="Release" build/giada.sln

fi