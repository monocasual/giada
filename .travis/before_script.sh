#!/usr/bin/env bash

set -e

mkdir build

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	cmake -S . -B build/ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-ObjC++ -DWITH_VST=ON

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

	/usr/bin/cmake -S . -B build/ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-Wno-class-memaccess -DWITH_VST=ON

fi