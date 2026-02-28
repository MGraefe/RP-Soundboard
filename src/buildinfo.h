// src/buildinfo.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const char *buildinfo_getPluginVersion();
const char *buildinfo_getPluginName();
const char *buildinfo_getPluginAuthor();
const char *buildinfo_getPluginDescription();
const char *buildinfo_getBuildName();
const char *buildinfo_getPluginVersionShort();
const char *buildinfo_getBuildTarget();
const char *buildinfo_getBuildDate();
const char *buildinfo_getBuildTime();


//index: 0 = major, 1 = minor, 2 = revision, 3 = build
int buildinfo_getVersionNumber(int index);
int buildinfo_getBuildNumber();


#ifdef __cplusplus
} // extern "C"
#endif

