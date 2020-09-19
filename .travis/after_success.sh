#!/usr/bin/env bash

set -e

mkdir dist

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	upx --best ./build/giada
	# TODO - collect libraries, make Bundle
	cp ./build/giada ./dist

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

	# Strip binary.

	strip ./build/giada

	# Create temporary dir. Binary and desktop files must go in there. This is 
	# necessary for linuxdeploy to work correctly.

	mkdir ./temp/
	mv ./build/giada ./temp/

	# Create .desktop file.

cat << EOF > ./temp/giada.desktop
[Desktop Entry]
Name=Giada
Name[es]=Giada
GenericName=Drum machine and loop sequencer
GenericName[es]=Caja de ritmos y secuenciador de loops
Icon=giada
Type=Application
Exec=giada
Terminal=false
Categories=AudioVideo;Audio;X-Digital_Processing;X-Jack;X-MIDI;Midi;
Keywords=Giada;
EOF

	# Prepare logo. 
	
	mv extras/giada-logo.svg ./temp/giada.svg

	# Run linuxdeploy to make the AppImage, then move it to ./dist dir. 
	# For some reasons linuxdeploy uses the commit hash in the filename, so
	# rename it first.

	./linuxdeploy-x86_64.AppImage -e ./temp/giada -d ./temp/giada.desktop -i ./temp/giada.svg --output appimage --appdir ./temp
	mv Giada-*-x86_64.AppImage Giada-x86_64.AppImage 
	cp Giada-x86_64.AppImage ./dist

fi