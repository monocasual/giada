#!/usr/bin/env bash

set -e

brew update > /dev/null
brew install rtmidi
brew install libsamplerate
brew install fltk
brew install libsndfile
brew install upx

# Remove dynamic libraries to force static linking.

rm -rf /usr/local/lib/librtmidi*dylib
rm -rf /usr/local/lib/libsamplerate*dylib
rm -rf /usr/local/lib/libfltk*dylib
rm -rf /usr/local/lib/libfltk_forms*dylib
rm -rf /usr/local/lib/libfltk_gl*dylib
rm -rf /usr/local/lib/libfltk_images*dylib
rm -rf /usr/local/lib/libsndfile*dylib 
rm -rf /usr/local/lib/libFLAC*dylib
rm -rf /usr/local/lib/libogg*dylib
rm -rf /usr/local/lib/libopus*dylib
rm -rf /usr/local/lib/libvorbis*dylib
rm -rf /usr/local/lib/libvorbisenc*dylib