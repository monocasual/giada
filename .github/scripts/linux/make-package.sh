#!/usr/bin/env bash

set -e

if [ -z "$1" ]; then
    echo "Usage: make-package.sh [release-version]"
    exit 1
fi

RELEASE_VERSION=$1

echo "Create working dirs"

mkdir dist
mkdir temp

echo "Strip binary and move it to temp/"

strip --strip-all ./build/giada
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
mv Giada-*-x86_64.AppImage Giada-$RELEASE_VERSION-x86_64.AppImage 
cp Giada-$RELEASE_VERSION-x86_64.AppImage dist/