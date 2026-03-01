// src/ts3log.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "common.h"

#include <cstdarg>
#include <string>


void logMessage(const char* msg, LogLevel level, ...)
{
	char buf[512];
	va_list argptr;

	va_start(argptr, level);
	vsnprintf(buf, 512, msg, argptr);
	va_end(argptr);

	ts3Functions.logMessage(buf, level, "SB", 0);
}

#define logError(msg, ...) logMessage(msg, LogLevel_ERROR, __VA_ARGS__)
#define logInfo(msg, ...) logMessage(msg, LogLevel_INFO, __VA_ARGS__)
#define logWarning(msg, ...) logMessage(msg, LogLevel_WARNING, __VA_ARGS__)


UINT checkError(UINT code, const char* msg, ...)
{
	if (code != ERROR_ok)
	{
		char buf[512];
		va_list argptr;

		va_start(argptr, msg);
		vsnprintf(buf, 512, msg, argptr);
		va_end(argptr);

		ts3Functions.logMessage(buf, LogLevel_ERROR, "SB", 0);
	}

	return code;
}
