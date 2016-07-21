// src/common.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__common_H__
#define rpsbsrc__common_H__

#include <stddef.h>
#include <stdint.h>

#include "public_errors.h"
#include "public_errors_rare.h"
#include "public_definitions.h"
#include "public_rare_definitions.h"
#include "ts3_functions.h"


#ifndef CAPI
#ifdef __cplusplus
#define CAPI extern "C"
#else
#define CAPI extern
#endif
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


#endif // rpsbsrc__common_H__
