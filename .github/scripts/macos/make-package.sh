#!/usr/bin/env bash

set -e

echo "Create working dirs"

mkdir dist
mkdir temp

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

# Note: $RELEASE_VERSION is an environment variable set in the
# packaging.yml script.

cp -r temp/giada.app dist/
cd dist
zip -r giada-$RELEASE_VERSION-x86_64-macos.zip giada.app
rm -rf giada.app