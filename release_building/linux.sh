#!/bin/bash
set -e
cd "$(dirname "$0")"
. ../linux/shell/globals.sh
cd "$(dirname "$0")"

# STANDALONE BUILDING
BUILD_DIR="./output/linux"
mkdir -p $BUILD_DIR
../linux/shell/del.sh
cd "$(dirname "$0")"
../linux/build.sh "Release"
cd "$(dirname "$0")"
cp ../build/linux/$PROJECTNAME $BUILD_DIR
cd "$(dirname "$0")"
cd output
tar -czf $PROJECTNAME-linux-standalone.tar.gz -C linux $PROJECTNAME
cd ..

# APPIMAGE BUILDING
cp ./output/linux/$PROJECTNAME ./$PROJECTNAME.AppDir/usr/bin/$PROJECTNAME

if [ ! -f ./appimagetool-x86_64.AppImage ]; then
        curl -LO https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
        chmod 700 ./appimagetool-x86_64.AppImage
fi

ARCH=x86_64 ./appimagetool-x86_64.AppImage $PROJECTNAME.AppDir $PROJECTNAME-linux-x86_64.AppImage
