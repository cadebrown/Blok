#!/bin/sh
# Blok/build_libs.sh - a simple script that should be ran once to initialize all the dependencies for Blok

JOBS=16
START_DIR=$PWD
PREFIX_DIR=$PWD/build/prefix
BUILD_DIR=./build

LIB_GLFW=./libs/glfw-*.tar.gz
LIB_ASSIMP=./libs/assimp-*.tar.gz
LIB_PORTAUDIO=./libs/pa_*.tar.gz
LIB_ARCHIVE=./libs/libarchive-*.tar.gz
LIB_FREETYPE=./libs/freetype-*.tar.gz

mkdir -p $BUILD_DIR

tar xf $LIB_GLFW -C $BUILD_DIR 
tar xf $LIB_ASSIMP -C $BUILD_DIR 
tar xf $LIB_PORTAUDIO -C $BUILD_DIR
tar xf $LIB_ARCHIVE -C $BUILD_DIR
tar xf $LIB_FREETYPE -C $BUILD_DIR

GLFW_DIR=$PWD/build/glfw-*
ASSIMP_DIR=$PWD/build/assimp-*
PORTAUDIO_DIR=$PWD/build/portaudio*
ARCHIVE_DIR=$PWD/build/libarchive-*
FREETYPE_DIR=$PWD/build/freetype-*


# -*- CMAKE-based dependencies

# build glfw
cd $GLFW_DIR
cmake . -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX_DIR -DBUILD_SHARED_LIBS=OFF \
    -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
make -j$JOBS
make install
GLFW_SLIB=$PWD/src/libglfw3.a
strip --strip-debug $GLFW_SLIB

# build assimp
cd $ASSIMP_DIR
cmake . -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX_DIR -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release \
    -DASSIMP_BUILD_ZLIB=ON -DASSIMP_BUILD_TESTS=OFF -DASSIMP_INSTALL_PDB=OFF -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ASSIMP_TOOLS=OFF \
    -DINJECT_DEBUG_POSTFIX=OFF -DASSIMP_BUILD_AMF_IMPORTER=OFF -DASSIMP_BUILD_AC_IMPORTER=OFF -DASSIMP_BUILD_ASE_IMPORTER=OFF \
    -DASSIMP_BUILD_ASSBIN_IMPORTER=OFF -DASSIMP_BUILD_ASSXML_IMPORTER=OFF -DASSIMP_BUILD_CSM_IMPORTER=OFF -DASSIMP_BUILD_HMP_IMPORTER=OFF \
    -DASSIMP_BUILD_LWS_IMPORTER=OFF -DASSIMP_BUILD_LWO_IMPORTER=OFF -DASSIMP_BUILD_MDL_IMPORTER=OFF -DCMAKE_BUILD_NFF_IMPORTER=OFF \
    -DCMAKE_BUILD_NDO_IMPORTER=OFF -DCMAKE_BUILD_OFF_IMPORTER=OFF -DCMAKE_BUILD_COB_IMPORTER=OFF -DCMAKE_BUILD_IFC_IMPORTER=OFF \
    -DCMAKE_BUILD_SIB_IMPORTER=OFF -DCMAKE_BUILD_3MF_IMPORTER=OFF -DCMAKE_BUILD_MMD_IMPORTER=OFF
make -j$JOBS
make install
ASSIMP_SLIB=$PWD/lib/libassimp.a
strip --strip-debug $ASSIMP_SLIB


# -*- AUTOMAKE/configurescript-based dependencies

# build portaudio
cd $PORTAUDIO_DIR
./configure --prefix=$PREFIX_DIR --enable-static --disable-shared
make -j$JOBS
make install
PORTAUDIO_SLIB=$PWD/lib/.libs/libportaudio.a
strip --strip-debug $PORTAUDIO_SLIB

# build libarchive
cd $ARCHIVE_DIR
./configure --prefix=$PREFIX_DIR --enable-static --disable-shared
make -j$JOBS
make install
ARCHIVE_SLIB=$PWD/.libs/libarchive.a
strip --strip-debug $ARCHIVE_SLIB

# build freetype for rendering fonts
cd $FREETYPE_DIR
# configure without external requirements
./configure --prefix=$PREFIX_DIR --enable-static --disable-shared --with-png=no --with-zlib=no --with-harfbuzz=no 
make -j$JOBS
make install
FREETYPE_SLIB=$PWD/objs/.libs/libfreetype.a
strip --strip-debug $FREETYPE_SLIB

echo "built static glfw3: $GLFW_SLIB"
echo "built static assimp: $ASSIMP_SLIB"
echo "built static portaudio: $PORTAUDIO_SLIB"
echo "built static libarchive: $ARCHIVE_SLIB"
echo "built static freetype: $FREETYPE_SLIB"

echo "Installed All Dependencies to $PREFIX_DIR"

