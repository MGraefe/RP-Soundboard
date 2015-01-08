#ifndef ts3log_H__
#define ts3log_H__

#include "common.h"

void logMessage(const char *msg, LogLevel level,  ...);

#define logError(msg, ...) logMessage(msg, LogLevel_ERROR, __VA_ARGS__)
#define logInfo(msg, ...) logMessage(msg, LogLevel_INFO, __VA_ARGS__)
#define logWarning(msg, ...) logMessage(msg, LogLevel_WARNING, __VA_ARGS__)


UINT checkError(UINT code, const char *msg, ...);

#endif // ts3log_H__
