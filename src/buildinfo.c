
#include "buildinfo.h"
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
#elif defined(LINUX) && defined(__x86_64__)
#define BUILD_NAME "Linux 64-bit"
#elif defined(LINUX) && defined(__i386__)
#define BUILD_NAME "Linux 32-bit"
#elif defined(MACOS)
#define BUILD_NAME "MacOS 64-bit"
#else
#define BUILD_NAME "Unknown Build"
#endif

#define PLUGIN_NAME "RP Soundboard"

#define PLUGIN_VERSION (TS3SB_VERSION_S " " BUILD_TARGET " " BUILD_NAME)
#define PLUGIN_AUTHOR ("Marius Gr\xC3\xA4" "fe")
#define PLUGIN_DESCRIPTION ("Easy to use Soundboard.\n" \
	"This software uses libraries from the FFmpeg project under the LGPLv2.1. " \
	"A copy of the used source code along with the relevant configuration files " \
	"can be downloaded here: https://github.com/MGraefe/RP-Soundboard")


const char* buildinfo_getPluginVersion()
{
	return PLUGIN_VERSION;
}


const char* buildinfo_getPluginName()
{
	return PLUGIN_NAME;
}


const char* buildinfo_getPluginAuthor()
{
	return PLUGIN_AUTHOR;
}


const char* buildinfo_getPluginDescription()
{
	return PLUGIN_DESCRIPTION;
}


const char* buildinfo_getBuildName()
{
	return BUILD_NAME;
}


const char* buildinfo_getPluginVersionShort()
{
	return TS3SB_VERSION_S;
}


// index: 0 = major, 1 = minor, 2 = revision, 3 = build
int buildinfo_getVersionNumber(int index)
{
	switch (index)
	{
	case 0:
		return TS3SB_VERSION_MAJOR;
	case 1:
		return TS3SB_VERSION_MINOR;
	case 2:
		return TS3SB_VERSION_REVISION;
	case 3:
		return TS3SB_VERSION_BUILD;
	default:
		return -1;
	}
}


const char* buildinfo_getBuildTarget()
{
	return BUILD_TARGET;
}


int buildinfo_getBuildNumber()
{
	return TS3SB_VERSION_BUILD;
}


const char* buildinfo_getBuildDate()
{
	return __DATE__;
}


const char* buildinfo_getBuildTime()
{
	return __TIME__;
}