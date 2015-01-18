
#ifndef buildinfo_H__
#define buildinfo_H__

#include "version/version.h"

#if defined(_WIN64)
#define BUILD_NAME "Windows 64-bit"
#elif defined(_WIN32)
#define BUILD_NAME "Windows 32-bit"
#else
#define BUILD_NAME "Unknown Build"
#endif

#define PLUGIN_NAME "RP Soundboard"
#define PLUGIN_VERSION (TS3SB_VERSION_S " " BUILD_NAME)
#define PLUGIN_AUTHOR ("Marius Gr\xC3\xA4" "fe")
#define PLUGIN_DESCRIPTION "Easy to use Soundboard"

#endif