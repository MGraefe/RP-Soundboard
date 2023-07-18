#!/bin/bash

# expected to be executed in ffmpeg/build-scripts

os="$(uname -s)"
case "$os" in 
	Linux*)
		prefix="lib_lin"
		postfix="a"
		infix="lib"
		;;
	Darwin*)
		prefix="lib_mac"
		postfix="a"
		infix="lib"
		;;
	*)
		prefix="lib_win"
		infix=""
		postfix="lib"
		;;
esac

pushd ..

if [ -d "ffmpeg/x64" ]; then
	mkdir -p "${prefix}_x64"
	cp ffmpeg/x64/lib/libavcodec.a ${prefix}_x64/${infix}avcodec.$postfix
	cp ffmpeg/x64/lib/libavformat.a ${prefix}_x64/${infix}avformat.$postfix
	cp ffmpeg/x64/lib/libavutil.a ${prefix}_x64/${infix}avutil.$postfix
	cp ffmpeg/x64/lib/libswresample.a ${prefix}_x64/${infix}swresample.$postfix
	cp -R ffmpeg/x64/include ${prefix}_x64/include
fi

if [ -d "ffmpeg/x86" ]; then
	mkdir -p "${prefix}_x86"
	cp ffmpeg/x86/lib/libavcodec.a ${prefix}_x86/${infix}avcodec.$postfix
	cp ffmpeg/x86/lib/libavformat.a ${prefix}_x86/${infix}avformat.$postfix
	cp ffmpeg/x86/lib/libavutil.a ${prefix}_x86/${infix}avutil.$postfix
	cp ffmpeg/x86/lib/libswresample.a ${prefix}_x86/${infix}swresample.$postfix
	cp -R ffmpeg/x86/include ${prefix}_x86/include
fi

popd
