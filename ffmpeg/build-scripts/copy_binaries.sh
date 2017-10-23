#!/bin/bash


os="$(uname -s)"
case "$os" in 
	Linux*)	prefix="lib_lin"
		postfix="a" ;;
	Darwin*) prefix="lib_mac" 
		postfix="a" ;;
	*) prefix="lib_win" 
	   postfix="lib" ;;
esac

mkdir -p "${prefix}_x64"
mkdir -p "${prefix}_x86"

cp ffmpeg/x64/lib/libavcodec.a ${prefix}_x64/avcodec.$postfix
cp ffmpeg/x64/lib/libavformat.a ${prefix}_x64/avformat.$postfix
cp ffmpeg/x64/lib/libavutil.a ${prefix}_x64/avutil.$postfix
cp ffmpeg/x64/lib/libswresample.a ${prefix}_x64/swresample.$postfix
cp -R ffmpeg/x64/include ${prefix}_x64/include

cp ffmpeg/x86/lib/libavcodec.a ${prefix}_x86/avcodec.$postfix
cp ffmpeg/x86/lib/libavformat.a ${prefix}_x86/avformat.$postfix
cp ffmpeg/x86/lib/libavutil.a ${prefix}_x86/avutil.$postfix
cp ffmpeg/x86/lib/libswresample.a ${prefix}_x86/swresample.$postfix
cp -R ffmpeg/x86/include ${prefix}_x86/include
