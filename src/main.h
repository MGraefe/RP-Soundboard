// src/main.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#pragma once

#include "common.h"

#ifdef __cplusplus
class SoundInfo;
int sb_playFile(const SoundInfo& sound);

class Sampler;
Sampler* sb_getSampler();
#endif

void sb_handleCaptureData(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited);
void sb_handlePlaybackData(
	uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels,
	const unsigned int* channelSpeakerArray, unsigned int* channelFillMask
);
void sb_stopPlayback();
void sb_init();
void sb_kill();
void sb_onServerChange(uint64 serverID);
void sb_openDialog();
int sb_playButtonEx(const char* btn);
void sb_playButton(int btn);
void sb_setConfig(int cfg);
void sb_openAbout();
void sb_pauseButtonPressed();
void sb_onConnectStatusChange(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);
void sb_getInternalHotkeyName(int buttonId, char* buf); // buf should be at sized 16
void sb_getInternalConfigHotkeyName(int configId, char* buf);
void sb_onHotkeyRecordedEvent(const char* keyword, const char* key);
void sb_onStopTalking();
void sb_onHotkeyPressed(const char* keyword);
void sb_checkForUpdates();
int sb_parseCommand(char**, int);
void sb_disableHotkeysTemporarily(bool disable);


#define HOTKEY_STOP_ALL "stop_all"
#define HOTKEY_PAUSE_ALL "pause_all"
#define HOTKEY_MUTE_MYSELF "mute_myself"
#define HOTKEY_MUTE_ON_MY_CLIENT "mute_on_my_client"
#define HOTKEY_VOLUME_INCREASE "volume_increase"
#define HOTKEY_VOLUME_DECREASE "volume_decrease"
