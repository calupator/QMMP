#!/bin/sh

NAME=enca
VERSION=1.19

case $1 in
  --download)
    mkdir -p temp
    cd temp
    #wget -nc http://dl.cihar.com/enca/$NAME-$VERSION.tar.gz
    wget -nc --no-check-certificate https://github.com/nijel/enca/archive/$VERSION.tar.gz \
    -O $NAME-$VERSION.tar.gz
  ;;
  --install)
    cd temp
    tar xvzf $NAME-$VERSION.tar.gz
    cd $NAME-$VERSION
    cat ../../configure.patch | patch -p1
    cp README.md README
    ./autogen.sh
    ./configure --prefix=$PREFIX --enable-shared --disable-static
    make -j${JOBS}
    make install

  ;;
  --clean)
    cd temp
    rm -rf $NAME-$VERSION
    cd ..
  ;;
esac
