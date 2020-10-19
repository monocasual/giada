#!/usr/bin/env bash

set -e

mkdir build

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	cmake -S . -B build/ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-ObjC++ -DWITH_VST3=ON

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

	if [[ $TRAVIS_TAG != '' ]]; then  # Tagged release, no tests
		/usr/bin/cmake -S . -B build/ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_CXX_FLAGS=-Wno-class-memaccess -DWITH_VST3=ON
	else                              # Regular commit, run tests
		/usr/bin/cmake -S . -B build/ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_CXX_FLAGS=-Wno-class-memaccess -DWITH_VST3=ON -DWITH_TESTS=ON
	fi

elif [[ $TRAVIS_OS_NAME == 'windows' ]]; then

	cmake -S . -B build/ -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE="${TRAVIS_BUILD_DIR}"/vcpkg/scripts/buildsystems/vcpkg.cmake \
		-DCMAKE_PREFIX_PATH="${TRAVIS_BUILD_DIR}"/vcpkg/installed/x64-windows \
		-DINCLUDE_DIRS="${TRAVIS_BUILD_DIR}"/vcpkg/installed/x64-windows/include \
		-DWITH_VST3=ON

fi