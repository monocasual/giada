#!/usr/bin/env bash

set -e

mkdir dist
mkdir temp

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	echo "UPX binary"

	upx --best ./build/Release/giada

	echo "Create temporary directory giada.app"

	mkdir temp/giada.app
	mkdir temp/giada.app/Contents
	mkdir temp/giada.app/Contents/Resources
	mkdir temp/giada.app/Contents/MacOS
	echo APPLnone > temp/giada.app/Contents/PkgInfo

	echo "Generate Info.plist file"

	cat << EOF > temp/giada.app/Contents/Info.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist SYSTEM "file://localhost/System/Library/DTDs/PropertyList.dtd">
<plist version="0.9">
<dict>
	<key>CFBundleDisplayName</key>
	<string>giada</string>
	<key>CFBundleName</key>
	<string>giada</string>
	<key>CFBundleExecutable</key>
	<string>giada</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleVersion</key>
	<string>1.0</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0</string>
	<key>CFBundleIconFile</key>
	<string>giada.icns</string>
	<key>CFBundleSignature</key>
	<string>none</string>
</dict>
</plist>
EOF

	echo "Copy the binary file into the .app bundle"

	mv ./build/Release/giada temp/giada.app/Contents/MacOS/

	echo "Set executable permissions"

	chmod 755 temp/giada.app/Contents/MacOS/giada

	echo "Copy icon (.icns)"

	cp extras/giada.icns temp/giada.app/Contents/Resources/

	echo "Copy final bundle to dist/, zip it and clean it up"

	cp -r temp/giada.app dist/
	cd dist
	zip -r giada-$TRAVIS_TAG-x86_64-macos.zip giada.app
	rm -rf giada.app

elif [[ $TRAVIS_OS_NAME == 'linux' ]]; then

	echo "Strip binary and move it to temp/"

	strip ./build/giada
	mv ./build/giada temp/

	echo "Create .desktop file"

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

	echo "Prepare logo"
	
	mv extras/giada-logo.svg temp/giada.svg

	# Run linuxdeploy to make the AppImage, then move it to dist/ dir. 
	# For some reason linuxdeploy uses the commit hash in the filename, so
	# rename it first.

	./linuxdeploy-x86_64.AppImage -e temp/giada -d temp/giada.desktop -i temp/giada.svg --output appimage --appdir temp/
	mv Giada-*-x86_64.AppImage Giada-$TRAVIS_TAG-x86_64.AppImage 
	cp Giada-$TRAVIS_TAG-x86_64.AppImage dist/

elif [[ $TRAVIS_OS_NAME == 'windows' ]]; then

	echo "Copy binary and dll files to temp/"

	cp build/Release/giada.exe build/Release/*.dll temp/

	echo "Make zip archive, save it to to dist/"
	
	7z a -tzip dist/giada-$TRAVIS_TAG-x86_64-windows.zip ./temp/*

fi