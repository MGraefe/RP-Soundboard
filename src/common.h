#ifndef common_H__
#define common_H__

#include <stdint.h>

#include "public_errors.h"
#include "public_errors_rare.h"
#include "public_definitions.h"
#include "public_rare_definitions.h"
#include "ts3_functions.h"



#ifdef __cplusplus
#define CAPI extern "C"
#else
#define CAPI extern
#endif

#ifdef __cplusplus
	#include <string>
	#ifdef UNICODE
		typedef std::wstring tstring;
	#else
		typedef std::string tstring;
	#endif
#endif // __cplusplus

typedef uint32_t UINT;
typedef uint16_t USHORT;

CAPI struct TS3Functions ts3Functions;

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128


#endif // common_H__
