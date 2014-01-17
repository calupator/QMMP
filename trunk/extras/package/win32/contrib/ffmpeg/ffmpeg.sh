#!/bin/sh

NAME=ffmpeg
VERSION=2.1.3

case $1 in
  --download)
    mkdir -p temp
    cd temp
    wget -nc http://ffmpeg.org/releases/$NAME-$VERSION.tar.bz2
  ;;
  --install)
    cd temp
    tar xvjf $NAME-$VERSION.tar.bz2
    cd $NAME-$VERSION
    ./configure --prefix=$PREFIX --enable-shared --disable-static --disable-doc --disable-avdevice \
    --disable-network --disable-encoders --enable-small --disable-muxers --disable-indevs --disable-outdevs \
    --disable-iconv --disable-bsfs --disable-programs \
    --disable-muxers --disable-demuxers \
    --disable-protocols \
    --disable-parsers \
    --disable-filters \
    --disable-decoders \
    --enable-protocol=file --enable-protocol=data --enable-protocol=data --enable-protocol=pipe \
    --enable-filter=aconvert --enable-filter=atempo --enable-filter=movie --enable-filter=scale \
    --enable-decoder=ape --enable-decoder=wmav1 --enable-decoder=wmav2 --enable-decoder=truehd \
    --enable-decoder=aac --enable-decoder=alac --enable-decoder=shorten --enable-decoder=twinvq \
    --enable-decoder=tta --enable-decoder=ra_288 --enable-decoder=ra_144 \
    --enable-decoder=eac3 --enable-decoder=dts --enable-decoder=flac \
    --enable-demuxer=ape  --enable-demuxer=xwma --enable-demuxer=truehd \
    --enable-demuxer=aac  --enable-demuxer=shorten --enable-demuxer=rtsp \
    --enable-demuxer=tta --enable-demuxer=rtp \
    --enable-demuxer=eac3 --enable-demuxer=dts --enable-demuxer=dtshd --enable-demuxer=flac \
    --enable-demuxer=matroska --enable-demuxer=asf --enable-demuxer=mov
    make -j${JOBS}
    make install

  ;;
  --clean)
    cd temp
    rm -rf $NAME-$VERSION
    cd ..
  ;;
esac
