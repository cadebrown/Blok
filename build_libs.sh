#!/bin/sh
# Blok/build_libs.sh - a simple script that should be ran once to initialize all the dependencies for Blok
# this should work on anything with:
#  - cmake
#  - make
#  - c++/g++

# first, ensure all the utilities are satisfied
cmake --help > /dev/null || { echo "No 'cmake' found"; exit 1; }
make --help > /dev/null || { echo "No 'make' found"; exit 1; }
g++ --help > /dev/null || { echo "No 'g++' found"; exit 1; }
tar --help > /dev/null || { echo "No 'tar' found"; exit 1; }

# now, define some standard variables

# the library source directory
TARDIR=$PWD/libs
# where all of this will be built
DEPDIR=$PWD/build
# number of compiler jobs to pass to make -j
JOBS=16
# the prefix directory for installation
PREFIX=$DEPDIR/out


# glob for file names, get anmy matches
TAR_GLFW=$TARDIR/glfw-*.tar.gz
TAR_ASSIMP=$TARDIR/assimp-*.tar.gz
TAR_PORTAUDIO=$TARDIR/pa_*.tar.gz
TAR_ARCHIVE=$TARDIR/libarchive-*.tar.gz
TAR_FREETYPE=$TARDIR/freetype-*.tar.gz

# make sure they all exist
if [ ! -f $TAR_GLFW ]; then
    echo "No 'glfw' tarfile"
    exit 1
fi

if [ ! -f $TAR_ASSIMP ]; then
    echo "No 'assimp' tarfile"
    exit 1
fi

if [ ! -f $TAR_PORTAUDIO ]; then
    echo "No 'portaudio' tarfile"
    exit 1
fi

if [ ! -f $TAR_ARCHIVE ]; then
    echo "No 'archive' tarfile"
    exit 1
fi

if [ ! -f $TAR_FREETYPE ]; then
    echo "No 'freetype' tarfile"
    exit 1
fi

# built them to the dep dir
mkdir -p $DEPDIR

# untar them all
tar xf $TAR_GLFW -C $DEPDIR || { echo "Untarring 'glfw' failed"; exit 1; }
tar xf $TAR_ASSIMP -C $DEPDIR || { echo "Untarring 'assimp' failed"; exit 1; }
tar xf $TAR_PORTAUDIO -C $DEPDIR || { echo "Untarring 'portaudio' failed"; exit 1; }
tar xf $TAR_ARCHIVE -C $DEPDIR || { echo "Untarring 'archive' failed"; exit 1; }
tar xf $TAR_FREETYPE -C $DEPDIR || { echo "Untarring 'freetype' failed"; exit 1; }

# find their file results
GLFW_DIR=$DEPDIR/glfw-*
ASSIMP_DIR=$DEPDIR/assimp-*
PORTAUDIO_DIR=$DEPDIR/portaudio*
ARCHIVE_DIR=$DEPDIR/libarchive-*
FREETYPE_DIR=$DEPDIR/freetype-*


# -*- CMAKE-based dependencies

# build glfw
cd $GLFW_DIR
cmake . -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX -DBUILD_SHARED_LIBS=OFF \
    -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF && \
    make -j$JOBS && \
    make install || { echo "Compiling 'glfw' failed"; exit 1; }

#GLFW_SLIB=$PWD/src/libglfw3.a
#strip --strip-debug $GLFW_SLIB

# build assimp
cd $ASSIMP_DIR
cmake . -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release \
    -DASSIMP_BUILD_ZLIB=ON -DASSIMP_BUILD_TESTS=OFF -DASSIMP_INSTALL_PDB=OFF -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ASSIMP_TOOLS=OFF \
    -DINJECT_DEBUG_POSTFIX=OFF -DASSIMP_BUILD_AMF_IMPORTER=OFF -DASSIMP_BUILD_AC_IMPORTER=OFF -DASSIMP_BUILD_ASE_IMPORTER=OFF \
    -DASSIMP_BUILD_ASSBIN_IMPORTER=OFF -DASSIMP_BUILD_ASSXML_IMPORTER=OFF -DASSIMP_BUILD_CSM_IMPORTER=OFF -DASSIMP_BUILD_HMP_IMPORTER=OFF \
    -DASSIMP_BUILD_LWS_IMPORTER=OFF -DASSIMP_BUILD_LWO_IMPORTER=OFF -DASSIMP_BUILD_MDL_IMPORTER=OFF -DCMAKE_BUILD_NFF_IMPORTER=OFF \
    -DCMAKE_BUILD_NDO_IMPORTER=OFF -DCMAKE_BUILD_OFF_IMPORTER=OFF -DCMAKE_BUILD_COB_IMPORTER=OFF -DCMAKE_BUILD_IFC_IMPORTER=OFF \
    -DCMAKE_BUILD_SIB_IMPORTER=OFF -DCMAKE_BUILD_3MF_IMPORTER=OFF -DCMAKE_BUILD_MMD_IMPORTER=OFF && \
    make -j$JOBS && \
    make install || { echo "Compiling 'assimp' failed"; exit 1; }
#ASSIMP_SLIB=$PWD/lib/libassimp.a
#strip --strip-debug $ASSIMP_SLIB

# -*- AUTOMAKE/configurescript-based dependencies

# build portaudio
cd $PORTAUDIO_DIR
./configure --prefix=$PREFIX --enable-static --disable-shared &&
    make -j$JOBS && \
    make install || { echo "Compiling 'portaudio' failed"; exit 1; }
#PORTAUDIO_SLIB=$PWD/lib/.libs/libportaudio.a
#strip --strip-debug $PORTAUDIO_SLIB

# build libarchive
cd $ARCHIVE_DIR
./configure --prefix=$PREFIX --enable-static --disable-shared && \
    make -j$JOBS && \
    make install || { echo "Compiling 'archive' failed"; exit 1; }
#ARCHIVE_SLIB=$PWD/.libs/libarchive.a
#strip --strip-debug $ARCHIVE_SLIB

# build freetype for rendering fonts
cd $FREETYPE_DIR
# configure without external requirements
./configure --prefix=$PREFIX --enable-static --disable-shared --with-png=no --with-zlib=no --with-harfbuzz=no && \
    make -j$JOBS && \
    make install || { echo "Compiling 'freetype' failed"; exit 1; }
#FREETYPE_SLIB=$PWD/objs/.libs/libfreetype.a
#strip --strip-debug $FREETYPE_SLIB

echo "Installed All Dependencies to $PREFIX"

