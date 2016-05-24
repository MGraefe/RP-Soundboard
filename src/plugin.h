// src/plugin.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

/*
 * TeamSpeak 3 demo plugin
 *
 * Copyright (c) 2008-2014 TeamSpeak Systems GmbH
 */

#pragma once
#ifndef rpsbsrc__plugin_H__
#define rpsbsrc__plugin_H__

#ifdef WIN32
#define PLUGINS_EXPORTDLL __declspec(dllexport)
#else
#define PLUGINS_EXPORTDLL __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern struct TS3Functions ts3Functions;

const char *getPluginID();

/* Required functions */
PLUGINS_EXPORTDLL const char* ts3plugin_name();
PLUGINS_EXPORTDLL const char* ts3plugin_version();
PLUGINS_EXPORTDLL int ts3plugin_apiVersion();
PLUGINS_EXPORTDLL const char* ts3plugin_author();
PLUGINS_EXPORTDLL const char* ts3plugin_description();
PLUGINS_EXPORTDLL void ts3plugin_setFunctionPointers(const struct TS3Functions funcs);
PLUGINS_EXPORTDLL int ts3plugin_init();
PLUGINS_EXPORTDLL void ts3plugin_shutdown();

/* Optional functions */
PLUGINS_EXPORTDLL void ts3plugin_registerPluginID(const char* id);
PLUGINS_EXPORTDLL const char* ts3plugin_commandKeyword();
PLUGINS_EXPORTDLL void ts3plugin_currentServerConnectionChanged(uint64 serverConnectionHandlerID);
PLUGINS_EXPORTDLL void ts3plugin_freeMemory(void* data);
PLUGINS_EXPORTDLL int ts3plugin_requestAutoload();
PLUGINS_EXPORTDLL void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon);
PLUGINS_EXPORTDLL void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys);

/* Clientlib */
PLUGINS_EXPORTDLL void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
PLUGINS_EXPORTDLL void ts3plugin_onEditMixedPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, const unsigned int* channelSpeakerArray, unsigned int* channelFillMask);
PLUGINS_EXPORTDLL void ts3plugin_onEditCapturedVoiceDataEvent(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited);
PLUGINS_EXPORTDLL void ts3plugin_onCustom3dRolloffCalculationClientEvent(uint64 serverConnectionHandlerID, anyID clientID, float distance, float* volume);
PLUGINS_EXPORTDLL void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID);
PLUGINS_EXPORTDLL void ts3plugin_onHotkeyEvent(const char* keyword);

#ifdef __cplusplus
}
#endif

#endif
