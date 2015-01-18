
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <cstdarg>

#include <QtWidgets/qmessagebox.h>
#include <QtCore/QString>

#include "device.h"
#include "ts3log.h"
#include "inputfile.h"
#include "samples.h"

#include "config_qt.h"
#include "about_qt.h"
#include "ConfigModel.h"

class ModelObserver_Prog : public ConfigModel::Observer
{
public:
	void notify(ConfigModel &model, ConfigModel::notifications_e what, int data) override;
};


static uint64 activeServerId = 1;

bool pttActive = false;
bool vadActive = false;
bool inputActive = false;

ConfigModel *configModel = NULL;
ConfigQt *configDialog = NULL;
AboutQt *aboutDialog = NULL;
Sampler *sampler = NULL;
ModelObserver_Prog *modelObserver = NULL;


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

//bool setPushToTalk(uint64 scHandlerID, bool shouldTalk)
//{
//	// If PTT is inactive, store the current settings
//	if(!pttActive)
//	{
//		// Get the current VAD setting
//		char* vad;
//		if(checkError(ts3Functions.getPreProcessorConfigValue(scHandlerID, "vad", &vad), "Error retrieving vad setting"))
//			return false;
//		vadActive = !strcmp(vad, "true");
//		ts3Functions.freeMemory(vad);
//
//		// Get the current input setting, this will indicate whether VAD is being used in combination with PTT
//		int input;
//		if(checkError(ts3Functions.getClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED, &input), "Error retrieving input setting"))
//			return false;
//		inputActive = !input; // We want to know when it is active, not when it is inactive 
//	}
//
//	// If VAD is active and the input is active, disable VAD, restore VAD setting afterwards
//	if(checkError(ts3Functions.setPreProcessorConfigValue(scHandlerID, "vad",
//		(shouldTalk && (vadActive && inputActive)) ? "false" : (vadActive)?"true":"false"), "Error toggling vad"))
//		return false;
//
//	// Activate the input, restore the input setting afterwards
//	if(checkError(ts3Functions.setClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED, 
//		(shouldTalk || inputActive) ? INPUT_ACTIVE : INPUT_DEACTIVATED), "Error toggling input"))
//		return false;
//
//	// Update the client
//	ts3Functions.flushClientSelfUpdates(scHandlerID, NULL);
//
//	// Commit the change
//	pttActive = shouldTalk;
//
//	return true;
//}


bool setVoiceActivation(uint64 scHandlerID)
{
	// Activate Voice Activity Detection
	if(checkError(ts3Functions.setPreProcessorConfigValue(scHandlerID, "vad", "true"), "Error toggling vad"))
		return false;

	if(checkError(ts3Functions.setClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED, 
		INPUT_ACTIVE), "Error toggling input"))
		return false;

	ts3Functions.flushClientSelfUpdates(scHandlerID, NULL);

	vadActive = true;
	inputActive = true;

	return true;
}


bool setContinuousTransmission(uint64 scHandlerID)
{
	//Turn off vad
	if(checkError(ts3Functions.setPreProcessorConfigValue(scHandlerID, "vad", "false"), "Error toggling vad"))
		return false;

	if(checkError(ts3Functions.setClientSelfVariableAsInt(scHandlerID, CLIENT_INPUT_DEACTIVATED, 
		INPUT_ACTIVE), "Error toggling input"))
		return false;

	ts3Functions.flushClientSelfUpdates(scHandlerID, NULL);

	inputActive = true;
	vadActive = false;

	return true;
}


CAPI void sb_handlePlaybackData(uint64 serverConnectionHandlerID, short* samples, int sampleCount,
	int channels, const unsigned int *channelSpeakerArray, unsigned int *channelFillMask)
{
	sampler->fetchOutputSamples(samples, sampleCount, channels, channelSpeakerArray, channelFillMask);
}


CAPI void sb_handleCaptureData(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited)
{
	bool finished = false;
	int written = sampler->fetchInputSamples(samples, sampleCount, channels, &finished);
	if(finished)
		setVoiceActivation(activeServerId);
	if(written > 0)
		*edited |= 0x1;
}


CAPI int sb_playFile(const char *filename)
{
	sampler->playFile(filename);
	setContinuousTransmission(activeServerId);
	return 0;
}


CAPI void sb_init()
{
	InitFFmpegLibrary();

	configModel = new ConfigModel();
	configDialog = new ConfigQt(configModel);
	sampler = new Sampler();
	sampler->init();
	modelObserver = new ModelObserver_Prog();
	configModel->addObserver(modelObserver);
	configModel->readConfig();
}


CAPI void sb_saveConfig()
{
	configModel->writeConfig();
}

CAPI void sb_readConfig()
{
	configModel->readConfig();
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
}


CAPI void sb_onServerChange(uint64 serverID)
{
	activeServerId = serverID;
}


CAPI void sb_openDialog()
{
	if(!configDialog)
		configDialog = new ConfigQt(configModel);
	configDialog->show();
}


CAPI void sb_stopPlayback()
{
	sampler->stopPlayback();
	setVoiceActivation(activeServerId);
}


//CAPI void sb_setVolume(int vol)
//{
//	sampler->setVolume(vol);
//}
//
//
//CAPI void sb_setLocalPlayback( int enabled )
//{
//	sampler->setLocalPlayback(enabled == 1);
//}


CAPI void sb_playButton(int btn)
{
	const char *fn = configModel->getFileName(btn);
	if(fn)
		sb_playFile(fn);
}


CAPI void sb_openAbout()
{
	if(!aboutDialog)
		aboutDialog = new AboutQt();
	aboutDialog->show();
}