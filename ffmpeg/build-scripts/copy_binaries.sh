#!/bin/sh

mkdir lib_win_x64
mkdir lib_win_x86

cp ffmpeg/x64/lib/libavcodec.a lib_win_x64/avcodec.lib
cp ffmpeg/x64/lib/libavformat.a lib_win_x64/avformat.lib
cp ffmpeg/x64/lib/libavutil.a lib_win_x64/avutil.lib
cp ffmpeg/x64/lib/libswresample.a lib_win_x64/swresample.lib

cp ffmpeg/x86/lib/libavcodec.a lib_win_x86/avcodec.lib
cp ffmpeg/x86/lib/libavformat.a lib_win_x86/avformat.lib
cp ffmpeg/x86/lib/libavutil.a lib_win_x86/avutil.lib
cp ffmpeg/x86/lib/libswresample.a lib_win_x86/swresample.lib