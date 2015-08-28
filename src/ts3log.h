// src/ts3log.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__ts3log_H__
#define rpsbsrc__ts3log_H__

#include "common.h"

void logMessage(const char *msg, LogLevel level,  ...);

#define logError(msg, ...) logMessage(msg, LogLevel_ERROR, __VA_ARGS__)
#define logInfo(msg, ...) logMessage(msg, LogLevel_INFO, __VA_ARGS__)
#define logWarning(msg, ...) logMessage(msg, LogLevel_WARNING, __VA_ARGS__)
#define logDebug(msg, ...) logMessage(msg, LogLevel_DEBUG, __VA_ARGS__)
#define logCritical(msg, ...) logMessage(msg, LogLevel_CRITICAL, __VA_ARGS__)


UINT checkError(UINT code, const char *msg, ...);

#endif // rpsbsrc__ts3log_H__
