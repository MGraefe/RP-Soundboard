#!/bin/bash


os="$(uname -s)"
case "$os" in 
	Linux*)	prefix="lib_lin" ;;
	Darwin*) prefix="lib_mac" ;;
	*) prefix="lib_win" ;;
esac

mkdir -p "${prefix}_x64"
mkdir -p "${prefix}_x86"

cp ffmpeg/x64/lib/libavcodec.a ${prefix}_x64/avcodec.lib
cp ffmpeg/x64/lib/libavformat.a ${prefix}_x64/avformat.lib
cp ffmpeg/x64/lib/libavutil.a ${prefix}_x64/avutil.lib
cp ffmpeg/x64/lib/libswresample.a ${prefix}_x64/swresample.lib

cp ffmpeg/x86/lib/libavcodec.a ${prefix}_x86/avcodec.lib
cp ffmpeg/x86/lib/libavformat.a ${prefix}_x86/avformat.lib
cp ffmpeg/x86/lib/libavutil.a ${prefix}_x86/avutil.lib
cp ffmpeg/x86/lib/libswresample.a ${prefix}_x86/swresample.lib
