// src/main.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "common.h"

#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <cstdarg>
#include <map>

#include <QtWidgets/qmessagebox.h>
#include <QtCore/QString>

#include "main.h"
#include "ts3log.h"
#include "inputfile.h"
#include "samples.h"
#include "config_qt.h"
#include "about_qt.h"
#include "ConfigModel.h"
#include "UpdateChecker.h"
#include "SoundInfo.h"


class ModelObserver_Prog : public ConfigModel::Observer
{
public:
	void notify(ConfigModel &model, ConfigModel::notifications_e what, int data) override;
};


static uint64 activeServerId = 1;
static uint64 playingServerId = 1;

ConfigModel *configModel = NULL;
ConfigQt *configDialog = NULL;
AboutQt *aboutDialog = NULL;
Sampler *sampler = NULL;
ModelObserver_Prog *modelObserver = NULL;
UpdateChecker *updateChecker = NULL;
std::map<uint64, int> connectionStatusMap;
volatile bool playing = false;
//std::recursive_mutex playingMutex;
//typedef std::lock_guard<std::recursive_mutex> PlayingMutexLock;


void ModelObserver_Prog::notify(ConfigModel &model, ConfigModel::notifications_e what, int data)
{
	switch(what)
	{
	case ConfigModel::NOTIFY_SET_VOLUME:
		sampler->setVolume(data);
		break;
	case ConfigModel::NOTIFY_SET_PLAYBACK_LOCAL:
		sampler->setLocalPlayback(model.getPlaybackLocal());
		break;
	case ConfigModel::NOTIFY_SET_MUTE_MYSELF_DURING_PB:
		sampler->setMuteMyself(model.getMuteMyselfDuringPb());
	default:
		break;
	}
}


enum talk_state_e
{
	TS_INVALID,
	TS_PTT_WITHOUT_VA,
	TS_PTT_WITH_VA,
	TS_VOICE_ACTIVATION,
	TS_CONT_TRANS,
};

talk_state_e previousTalkState;

talk_state_e getTalkState(uint64 scHandlerID)
{
	char *vadStr;
	if(checkError(ts3Functions.getPreProcessorConfigValue(scHandlerID, "vad", &vadStr), "Error retrieving vad setting"))
		return TS_INVALID;
	bool vad = strcmp(vadStr, "true") == 0;
	ts3Functions.freeMemory(vadStr);

	int input;
	if(checkError(ts3Functions.getClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED, &input), "Error retrieving input setting"))
		return TS_INVALID;
	bool ptt = input == INPUT_DEACTIVATED;
	
	if(ptt)
		return vad ? TS_PTT_WITH_VA : TS_PTT_WITHOUT_VA;
	else
		return vad ? TS_VOICE_ACTIVATION : TS_CONT_TRANS;
}


bool setTalkState(uint64 scHandlerID, talk_state_e state)
{
	if (scHandlerID == 0)
		return false;

	bool va = state == TS_PTT_WITH_VA || state == TS_VOICE_ACTIVATION;
	bool in = state == TS_CONT_TRANS  || state == TS_VOICE_ACTIVATION;

	if(checkError(ts3Functions.setPreProcessorConfigValue(
		scHandlerID, "vad", va ? "true" : "false"), "Error toggling vad"))
		return false;

	if(checkError(ts3Functions.setClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED, 
		in ? INPUT_ACTIVE : INPUT_DEACTIVATED), "Error toggling input"))
		return false;

	ts3Functions.flushClientSelfUpdates(scHandlerID, NULL);
	return true;
}


bool setPushToTalk(uint64 scHandlerID, bool voiceActivation)
{
	return setTalkState(scHandlerID, voiceActivation ? TS_PTT_WITH_VA : TS_PTT_WITHOUT_VA);
}


bool setVoiceActivation(uint64 scHandlerID)
{
	return setTalkState(scHandlerID, TS_VOICE_ACTIVATION);
}


bool setContinuousTransmission(uint64 scHandlerID)
{
	return setTalkState(scHandlerID, TS_CONT_TRANS);
}


CAPI void sb_handlePlaybackData(uint64 serverConnectionHandlerID, short* samples, int sampleCount,
	int channels, const unsigned int *channelSpeakerArray, unsigned int *channelFillMask)
{
	if (serverConnectionHandlerID != activeServerId)
		return; //Ignore other servers

	sampler->fetchOutputSamples(samples, sampleCount, channels, channelSpeakerArray, channelFillMask);
}


CAPI void sb_handleCaptureData(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited)
{
	if (serverConnectionHandlerID != activeServerId)
		return; //Ignore other servers

	bool finished = false;
	int written = sampler->fetchInputSamples(samples, sampleCount, channels, &finished);
	if(playing && finished)
	{
		playing = false;
		setTalkState(playingServerId, previousTalkState);
	}
	if(written > 0)
		*edited |= 0x1;
}


int sb_playFile(const SoundInfo &sound)
{
	playingServerId = activeServerId;

	if(!playing)
	{
		talk_state_e s = getTalkState(activeServerId);
		if(s != TS_INVALID)
			previousTalkState = s;
	}

	if(sampler->playFile(sound))
	{
		playing = true;
		setContinuousTransmission(activeServerId);
	}

	return 0;
}


Sampler *sb_getSampler()
{
	return sampler;
}


void sb_enableInterface(bool enabled) 
{
	configDialog->setEnabled(enabled);
}


CAPI void sb_init()
{
#ifdef _DEBUG
	QMessageBox::information(NULL, "", "rp soundboard plugin init, attach debugger now");
#endif

	InitFFmpegLibrary();
	
	configModel = new ConfigModel();
	configModel->readConfig();

	sampler = new Sampler();
	sampler->init();

	configDialog = new ConfigQt(configModel);
	configDialog->showMinimized();
	configDialog->hide();

	modelObserver = new ModelObserver_Prog();
	configModel->addObserver(modelObserver);

	configModel->notifyAllEvents();

	updateChecker = new UpdateChecker();
	updateChecker->startCheck();
}


CAPI void sb_saveConfig()
{
	configModel->writeConfig();
}


CAPI void sb_kill()
{
	configModel->remObserver(modelObserver);
	delete modelObserver; 
	modelObserver = NULL;

	sampler->shutdown();
	delete sampler;
	sampler = NULL;

	configDialog->close();
	delete configDialog;
	configDialog = NULL;

	configModel->writeConfig();
	delete configModel;
	configModel = NULL;

	if(aboutDialog)
	{
		aboutDialog->close();
		delete aboutDialog;
		aboutDialog = NULL;
	}

	delete updateChecker;
	updateChecker = NULL;
}


CAPI void sb_onServerChange(uint64 serverID)
{
	talk_state_e talkState = getTalkState(activeServerId);
	if (connectionStatusMap.find(serverID) == connectionStatusMap.end())
		connectionStatusMap[serverID] = STATUS_DISCONNECTED;
	bool connected = connectionStatusMap[serverID] == STATUS_CONNECTION_ESTABLISHED;

	if (playing && activeServerId != serverID)
	{
		setTalkState(activeServerId, previousTalkState);
		if (connected && talkState != TS_INVALID)
			setTalkState(serverID, talkState);
		else
			sb_stopPlayback();
	}

	activeServerId = serverID;
	sb_enableInterface(connected);
}


CAPI void sb_openDialog()
{
	if(!configDialog)
		configDialog = new ConfigQt(configModel);
	configDialog->showNormal();
	configDialog->raise();
	configDialog->activateWindow();

	if (connectionStatusMap[activeServerId] != STATUS_CONNECTION_ESTABLISHED)
		QMessageBox::information(configDialog, "No server connection",
		"You are not connected to a server.\n"
		"RP Soundboard is disabled until you are connected properly.");
}


CAPI void sb_stopPlayback()
{
	if(playing)
	{
		sampler->stopPlayback();
		playing = false;
		setTalkState(activeServerId, previousTalkState);
	}
}


CAPI void sb_playButton(int btn)
{
	const SoundInfo *sound = configModel->getSoundInfo(btn);
	if(sound)
		sb_playFile(*sound);
}


CAPI void sb_openAbout()
{
	if(!aboutDialog)
		aboutDialog = new AboutQt();
	aboutDialog->show();
}


CAPI void sb_onConnectStatusChange(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) 
{
	if(newStatus == STATUS_DISCONNECTED)
		connectionStatusMap.erase(serverConnectionHandlerID);
	else
		connectionStatusMap[serverConnectionHandlerID] = newStatus;

	if (serverConnectionHandlerID == activeServerId)
	{
		if (newStatus == STATUS_DISCONNECTED)
			sb_stopPlayback();
		sb_enableInterface(newStatus == STATUS_CONNECTION_ESTABLISHED);
	}
}


CAPI void sb_getInternalHotkeyName(int buttonId, char *buf)
{
	sprintf(buf, "button_%i", buttonId + 1);
}

CAPI void sb_onHotkeyRecordedEvent(const char *keyword, const char *key)
{
	if (configDialog)
		configDialog->onHotkeyRecordedEvent(keyword, key);
}
