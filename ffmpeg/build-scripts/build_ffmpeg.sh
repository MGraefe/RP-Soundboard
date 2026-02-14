#!/bin/bash

# exit on error
set -ex

# expected to be executed in ffmpeg/build-scripts

unameOut="$(uname -s)"
case "${unameOut}" in
	Linux*) 	machine=Linux;;
	Darwin*) 	machine=Mac;;
	*) 		machine=Windows
esac

if [[ "$machine" == "Windows" ]]; then
	toolchainopt="--toolchain=msvc"
else
	toolchainopt=""
fi

# On macOS, build universal binary (arm64 + x86_64) by default
if [[ "$machine" == "Mac" && "$1" == "" ]]; then
	echo "Machine = Mac (Universal Binary: arm64 + x86_64)"
	build_mac_universal=true
elif [[ "$1" == "x86" ]]; then
	archprefix="x86"
	arch="x86"
	build_mac_universal=false
elif [[ "$1" == "arm64" || "$1" == "aarch64" ]]; then
	archprefix="arm64"
	arch="aarch64"
	build_mac_universal=false
elif [[ "$1" == "x64" || "$1" == "x86_64" || "$1" == "amd64" || "$(uname -m)" == "x86_64" ]]; then
	archprefix="x64"
	arch="x86_64"
	build_mac_universal=false
else
	archprefix="x86"
	arch="x86"
	build_mac_universal=false
fi

if [[ "$build_mac_universal" != "true" ]]; then
	echo "Machine =" $machine $arch
fi

opts="\
--enable-pic \
--disable-programs \
--disable-doc \
--disable-avdevice \
--disable-swscale \
--disable-postproc \
--disable-avfilter \
--disable-pixelutils \
--disable-encoders \
--disable-muxers \
--disable-devices \
--disable-filters \
--disable-debug"

if [[ "$(uname -s)" == "Linux" ]]; then
	opts="$opts --disable-asm"
fi

# we need no video or subtitle decoders
# obtain list of video and subtitle decoders via:
# ffmpeg -decoders | egrep '^ [VS][\.A-Z]{5} [^=]' | sed -E 's/^ [VS][\.A-Z]{5} +([[:graph:]]+) .*$/--disable-decoder=\1 \\/g'
disabled_decs="\
--disable-decoder=012v \
--disable-decoder=4xm \
--disable-decoder=8bps \
--disable-decoder=aasc \
--disable-decoder=aic \
--disable-decoder=alias_pix \
--disable-decoder=amv \
--disable-decoder=anm \
--disable-decoder=ansi \
--disable-decoder=apng \
--disable-decoder=asv1 \
--disable-decoder=asv2 \
--disable-decoder=aura \
--disable-decoder=aura2 \
--disable-decoder=avrn \
--disable-decoder=avrp \
--disable-decoder=avs \
--disable-decoder=avui \
--disable-decoder=ayuv \
--disable-decoder=bethsoftvid \
--disable-decoder=bfi \
--disable-decoder=binkvideo \
--disable-decoder=bintext \
--disable-decoder=bitpacked \
--disable-decoder=bmp \
--disable-decoder=bmv_video \
--disable-decoder=brender_pix \
--disable-decoder=c93 \
--disable-decoder=cavs \
--disable-decoder=cdgraphics \
--disable-decoder=cdxl \
--disable-decoder=cfhd \
--disable-decoder=cinepak \
--disable-decoder=clearvideo \
--disable-decoder=cljr \
--disable-decoder=cllc \
--disable-decoder=eacmv \
--disable-decoder=cpia \
--disable-decoder=camstudio \
--disable-decoder=cyuv \
--disable-decoder=dds \
--disable-decoder=dfa \
--disable-decoder=dirac \
--disable-decoder=dnxhd \
--disable-decoder=dpx \
--disable-decoder=dsicinvideo \
--disable-decoder=dvvideo \
--disable-decoder=dxa \
--disable-decoder=dxtory \
--disable-decoder=dxv \
--disable-decoder=escape124 \
--disable-decoder=escape130 \
--disable-decoder=exr \
--disable-decoder=ffv1 \
--disable-decoder=ffvhuff \
--disable-decoder=fic \
--disable-decoder=flashsv \
--disable-decoder=flashsv2 \
--disable-decoder=flic \
--disable-decoder=flv \
--disable-decoder=fmvc \
--disable-decoder=fraps \
--disable-decoder=frwu \
--disable-decoder=g2m \
--disable-decoder=gif \
--disable-decoder=h261 \
--disable-decoder=h263 \
--disable-decoder=h263i \
--disable-decoder=h263p \
--disable-decoder=h264 \
--disable-decoder=h264_qsv \
--disable-decoder=libopenh264 \
--disable-decoder=h264_cuvid \
--disable-decoder=hap \
--disable-decoder=hevc \
--disable-decoder=hevc_qsv \
--disable-decoder=hevc_cuvid \
--disable-decoder=hnm4video \
--disable-decoder=hq_hqa \
--disable-decoder=hqx \
--disable-decoder=huffyuv \
--disable-decoder=idcinvideo \
--disable-decoder=idf \
--disable-decoder=iff \
--disable-decoder=indeo2 \
--disable-decoder=indeo3 \
--disable-decoder=indeo4 \
--disable-decoder=indeo5 \
--disable-decoder=interplayvideo \
--disable-decoder=jpeg2000 \
--disable-decoder=libopenjpeg \
--disable-decoder=jpegls \
--disable-decoder=jv \
--disable-decoder=kgv1 \
--disable-decoder=kmvc \
--disable-decoder=lagarith \
--disable-decoder=loco \
--disable-decoder=m101 \
--disable-decoder=eamad \
--disable-decoder=magicyuv \
--disable-decoder=mdec \
--disable-decoder=mimic \
--disable-decoder=mjpeg \
--disable-decoder=mjpeg_cuvid \
--disable-decoder=mjpegb \
--disable-decoder=mmvideo \
--disable-decoder=motionpixels \
--disable-decoder=mpeg1video \
--disable-decoder=mpeg1_cuvid \
--disable-decoder=mpeg2video \
--disable-decoder=mpegvideo \
--disable-decoder=mpeg2_qsv \
--disable-decoder=mpeg2_cuvid \
--disable-decoder=mpeg4 \
--disable-decoder=mpeg4_cuvid \
--disable-decoder=msa1 \
--disable-decoder=mscc \
--disable-decoder=msmpeg4v1 \
--disable-decoder=msmpeg4v2 \
--disable-decoder=msmpeg4 \
--disable-decoder=msrle \
--disable-decoder=mss1 \
--disable-decoder=mss2 \
--disable-decoder=msvideo1 \
--disable-decoder=mszh \
--disable-decoder=mts2 \
--disable-decoder=mvc1 \
--disable-decoder=mvc2 \
--disable-decoder=mxpeg \
--disable-decoder=nuv \
--disable-decoder=paf_video \
--disable-decoder=pam \
--disable-decoder=pbm \
--disable-decoder=pcx \
--disable-decoder=pgm \
--disable-decoder=pgmyuv \
--disable-decoder=pictor \
--disable-decoder=pixlet \
--disable-decoder=png \
--disable-decoder=ppm \
--disable-decoder=prores \
--disable-decoder=prores_lgpl \
--disable-decoder=psd \
--disable-decoder=ptx \
--disable-decoder=qdraw \
--disable-decoder=qpeg \
--disable-decoder=qtrle \
--disable-decoder=r10k \
--disable-decoder=r210 \
--disable-decoder=rawvideo \
--disable-decoder=rl2 \
--disable-decoder=roqvideo \
--disable-decoder=rpza \
--disable-decoder=rscc \
--disable-decoder=rv10 \
--disable-decoder=rv20 \
--disable-decoder=rv30 \
--disable-decoder=rv40 \
--disable-decoder=sanm \
--disable-decoder=scpr \
--disable-decoder=screenpresso \
--disable-decoder=sgi \
--disable-decoder=sgirle \
--disable-decoder=sheervideo \
--disable-decoder=smackvid \
--disable-decoder=smc \
--disable-decoder=smvjpeg \
--disable-decoder=snow \
--disable-decoder=sp5x \
--disable-decoder=speedhq \
--disable-decoder=srgc \
--disable-decoder=sunrast \
--disable-decoder=svq1 \
--disable-decoder=svq3 \
--disable-decoder=targa \
--disable-decoder=targa_y216 \
--disable-decoder=tdsc \
--disable-decoder=eatgq \
--disable-decoder=eatgv \
--disable-decoder=theora \
--disable-decoder=thp \
--disable-decoder=tiertexseqvideo \
--disable-decoder=tiff \
--disable-decoder=tmv \
--disable-decoder=eatqi \
--disable-decoder=truemotion1 \
--disable-decoder=truemotion2 \
--disable-decoder=truemotion2rt \
--disable-decoder=camtasia \
--disable-decoder=tscc2 \
--disable-decoder=txd \
--disable-decoder=ultimotion \
--disable-decoder=utvideo \
--disable-decoder=v210 \
--disable-decoder=v210x \
--disable-decoder=v308 \
--disable-decoder=v408 \
--disable-decoder=v410 \
--disable-decoder=vb \
--disable-decoder=vble \
--disable-decoder=vc1 \
--disable-decoder=vc1_qsv \
--disable-decoder=vc1_cuvid \
--disable-decoder=vc1image \
--disable-decoder=vcr1 \
--disable-decoder=xl \
--disable-decoder=vmdvideo \
--disable-decoder=vmnc \
--disable-decoder=vp3 \
--disable-decoder=vp5 \
--disable-decoder=vp6 \
--disable-decoder=vp6a \
--disable-decoder=vp6f \
--disable-decoder=vp7 \
--disable-decoder=vp8 \
--disable-decoder=libvpx \
--disable-decoder=vp8_cuvid \
--disable-decoder=vp8_qsv \
--disable-decoder=vp9 \
--disable-decoder=libvpx-vp9 \
--disable-decoder=vp9_cuvid \
--disable-decoder=webp \
--disable-decoder=wmv1 \
--disable-decoder=wmv2 \
--disable-decoder=wmv3 \
--disable-decoder=wmv3image \
--disable-decoder=wnv1 \
--disable-decoder=vqavideo \
--disable-decoder=xan_wc3 \
--disable-decoder=xan_wc4 \
--disable-decoder=xbin \
--disable-decoder=xbm \
--disable-decoder=xface \
--disable-decoder=xpm \
--disable-decoder=xwd \
--disable-decoder=y41p \
--disable-decoder=ylc \
--disable-decoder=yop \
--disable-decoder=yuv4 \
--disable-decoder=zerocodec \
--disable-decoder=zlib \
--disable-decoder=zmbv \
--disable-decoder=ssa \
--disable-decoder=ass \
--disable-decoder=dvbsub \
--disable-decoder=dvdsub \
--disable-decoder=cc_dec \
--disable-decoder=pgssub \
--disable-decoder=jacosub \
--disable-decoder=microdvd \
--disable-decoder=mov_text \
--disable-decoder=mpl2 \
--disable-decoder=pjs \
--disable-decoder=realtext \
--disable-decoder=sami \
--disable-decoder=stl \
--disable-decoder=srt \
--disable-decoder=subrip \
--disable-decoder=subviewer \
--disable-decoder=subviewer1 \
--disable-decoder=text \
--disable-decoder=vplayer \
--disable-decoder=webvtt \
--disable-decoder=xsub"

njobs=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 8)
if [ "$njobs" -gt 16 ]; then
	njobs=16
fi

build_single_arch() {
	local build_arch="$1"
	local prefix="$2"
	local cc_override="$3"
	local extra_configure="$4"

	local -a configure_args=(--arch=$build_arch --prefix=$prefix)
	if [[ -n "$cc_override" ]]; then
		configure_args+=(--cc="$cc_override")
	fi
	if [[ "$machine" == "Mac" ]]; then
		configure_args+=(--extra-cflags="-Wno-incompatible-function-pointer-types")
	fi

	pushd ../ffmpeg
	echo "=== Configuring FFmpeg for $build_arch ==="
	./configure "${configure_args[@]}" \
		$extra_configure $toolchainopt $opts $disabled_decs
	make clean && make -j$njobs && make install
	popd
}

if [[ "$build_mac_universal" == "true" ]]; then
	host_arch="$(uname -m)"
	cross_compile_flags="--enable-cross-compile --target-os=darwin"

	# Build arm64 (cross-compile on Intel, native on Apple Silicon)
	echo "=== Building FFmpeg for arm64 ==="
	if [[ "$host_arch" == "x86_64" ]]; then
		build_single_arch "aarch64" "arm64" "clang -arch arm64" "$cross_compile_flags"
	else
		build_single_arch "aarch64" "arm64" "clang -arch arm64" ""
	fi

	# Build x86_64 (native on Intel, cross-compile on Apple Silicon)
	echo "=== Building FFmpeg for x86_64 ==="
	if [[ "$host_arch" == "arm64" ]]; then
		build_single_arch "x86_64" "x64" "clang -arch x86_64" "$cross_compile_flags"
	else
		build_single_arch "x86_64" "x64" "clang -arch x86_64" ""
	fi

	# Combine into universal binaries using lipo
	echo "=== Creating universal binaries with lipo ==="
	mkdir -p ../ffmpeg/universal/lib
	cp -R ../ffmpeg/x64/include ../ffmpeg/universal/include

	for lib in libavcodec.a libavformat.a libavutil.a libswresample.a; do
		lipo -create \
			"../ffmpeg/arm64/lib/$lib" \
			"../ffmpeg/x64/lib/$lib" \
			-output "../ffmpeg/universal/lib/$lib"
		echo "Created universal $lib"
	done

	./copy_binaries.sh
else
	# Remove stale universal artifacts to prevent copy_binaries.sh from
	# copying outdated libs into lib_mac_universal
	rm -rf ../ffmpeg/universal
	build_single_arch "$arch" "$archprefix" "" ""
	./copy_binaries.sh
fi
