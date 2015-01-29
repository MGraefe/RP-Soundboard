
#ifndef buildinfo_H__
#define buildinfo_H__

#include "version/version.h"

#ifdef NDEBUG
#define BUILD_TARGET "Release"
#else
#define BUILD_TARGET "Debug"
#endif

#if defined(_WIN64)
#define BUILD_NAME "Win 64-bit"
#elif defined(_WIN32)
#define BUILD_NAME "Win 32-bit"
#else
#define BUILD_NAME "Unknown Build"
#endif

#define PLUGIN_NAME "RP Soundboard"

#define PLUGIN_VERSION (TS3SB_VERSION_S " " BUILD_TARGET " " BUILD_NAME)
#define PLUGIN_AUTHOR ("Marius Gr\xC3\xA4" "fe")
#define PLUGIN_DESCRIPTION "Easy to use Soundboard.\nThis software uses libraries from the FFmpeg project under the LGPLv2.1. A copy of the used source code along with the relevant configuration files can be downloaded here: http://mgraefe.de/rpsb/ffmpeg-source.zip"

#endif