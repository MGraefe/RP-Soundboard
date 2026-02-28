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

#include <QObject>
#include <QMessageBox>
#include <QString>

#include "main.h"
#include "ts3log.h"
#include "inputfile.h"
#include "samples.h"
#include "MainWindow.h"
#include "About.h"
#include "ConfigModel.h"
#include "UpdateChecker.h"
#include "SoundInfo.h"
#include "TalkStateManager.h"
#include "SpeechBubble.h"

class ModelObserver_Prog : public ConfigModel::Observer
{
public:
	void notify(ConfigModel &model, ConfigModel::notifications_e what, int data) override;
};


static uint64 activeServerId = 1;

ConfigModel *configModel = nullptr;
SpeechBubble *notConnectedBubble = nullptr;
MainWindow *configDialog = nullptr;
AboutQt *aboutDialog = nullptr;
Sampler *sampler = nullptr;
TalkStateManager *tsMgr = nullptr;

bool hotkeysTemporarilyDisabled = false;

ModelObserver_Prog *modelObserver = nullptr;
UpdateChecker *updateChecker = nullptr;
std::map<uint64, int> connectionStatusMap;
typedef std::lock_guard<std::mutex> Lock;


void ModelObserver_Prog::notify(ConfigModel &model, ConfigModel::notifications_e what, int data)
{
	switch(what)
	{
	case ConfigModel::NOTIFY_SET_VOLUME_LOCAL:
		sampler->setVolumeLocal(data);
		break;
	case ConfigModel::NOTIFY_SET_VOLUME_REMOTE:
		sampler->setVolumeRemote(data);
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


void sb_handlePlaybackData(uint64 serverConnectionHandlerID, short* samples, int sampleCount,
	int channels, const unsigned int *channelSpeakerArray, unsigned int *channelFillMask)
{
	if (serverConnectionHandlerID != activeServerId)
		return; //Ignore other servers

	sampler->fetchOutputSamples(samples, sampleCount, channels, channelSpeakerArray, channelFillMask);
}


void sb_handleCaptureData(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited)
{
	if (serverConnectionHandlerID != activeServerId)
		return; //Ignore other servers

	int written = sampler->fetchInputSamples(samples, sampleCount, channels, nullptr);
	if(written > 0)
		*edited |= 0x1;
}


int sb_playFile(const SoundInfo &sound)
{
	if (activeServerId == 0)
		return 2;
	return sampler->playFile(sound) ? 0 : 1;
}


Sampler *sb_getSampler()
{
	return sampler;
}


void sb_enableInterface(bool enabled) 
{
	if (!enabled)
	{
		if (!notConnectedBubble)
		{
			notConnectedBubble = new SpeechBubble(configDialog);
			notConnectedBubble->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
			notConnectedBubble->setFixedSize(350, 80);
			notConnectedBubble->setBackgroundColor(QColor(255, 255, 255));
			notConnectedBubble->setBubbleStyle(false);
			notConnectedBubble->setClosable(false);
			notConnectedBubble->setText("You are not connected to a server.\n"
				"RP Soundboard is disabled until you are connected properly.");
			notConnectedBubble->attachTo(configDialog);
			if (configDialog->isVisible())
				notConnectedBubble->show();
		}
	}
	else if (notConnectedBubble)
	{
		delete notConnectedBubble;
		notConnectedBubble = nullptr;
	}

	configDialog->setEnabled(enabled);
}

void sb_init()
{
#ifdef _DEBUG
	QMessageBox::information(nullptr, "", "rp soundboard plugin init, attach debugger now");
#endif

	QTimer::singleShot(10, []{
		configModel = new ConfigModel();
		configModel->readConfig();

		/* This if first QObject instantiated, it will load the resources */
		sampler = new Sampler();
		sampler->init();

		tsMgr = new TalkStateManager();
		QObject::connect(sampler, &Sampler::onStartPlaying, tsMgr, &TalkStateManager::onStartPlaying, Qt::QueuedConnection);
		QObject::connect(sampler, &Sampler::onStopPlaying, tsMgr, &TalkStateManager::onStopPlaying, Qt::QueuedConnection);
		QObject::connect(sampler, &Sampler::onPausePlaying, tsMgr, &TalkStateManager::onPauseSound, Qt::QueuedConnection);
		QObject::connect(sampler, &Sampler::onUnpausePlaying, tsMgr, &TalkStateManager::onUnpauseSound, Qt::QueuedConnection);

		configDialog = new MainWindow(configModel);

		modelObserver = new ModelObserver_Prog();
		configModel->addObserver(modelObserver);

		configModel->notifyAllEvents();

		updateChecker = new UpdateChecker();
		updateChecker->startCheck(false, configModel);
	});
}


void sb_kill()
{
	configModel->remObserver(modelObserver);
	delete modelObserver; 
	modelObserver = nullptr;

	sampler->shutdown();
	delete sampler;
	sampler = nullptr;

	configDialog->close();
	delete configDialog;
	configDialog = nullptr;

	configModel->writeConfig();
	delete configModel;
	configModel = nullptr;

	if(aboutDialog)
	{
		aboutDialog->close();
		delete aboutDialog;
		aboutDialog = nullptr;
	}

	delete updateChecker;
	updateChecker = nullptr;
}


void sb_onServerChange(uint64 serverID)
{
	if (connectionStatusMap.find(serverID) == connectionStatusMap.end())
		connectionStatusMap[serverID] = STATUS_DISCONNECTED;
	bool connected = connectionStatusMap[serverID] == STATUS_CONNECTION_ESTABLISHED;

	tsMgr->setActiveServerId(serverID);
	activeServerId = serverID;
	logInfo("Server Id: %ull", (unsigned long long)serverID);
	sb_enableInterface(connected);
}


void sb_openDialog()
{
	if(!configDialog)
		configDialog = new MainWindow(configModel);
	configDialog->showNormal();
	configDialog->raise();
	configDialog->activateWindow();

	sb_enableInterface(connectionStatusMap[activeServerId]);
}


void sb_stopPlayback()
{
	sampler->stopPlayback();
}


void sb_pauseButtonPressed()
{
	if (sampler->getState() == Sampler::ePLAYING)
		sampler->pausePlayback();
	else if (sampler->getState() == Sampler::ePAUSED)
		sampler->unpausePlayback();
}

/** play button by name or index(strtol), return 0 on success */
int sb_playButtonEx(const char* button)
{
	long arg1 = strtol(button, nullptr, 10);

	if ((nullptr != configDialog) && (configDialog->hotkeysEnabled()))
	{
		if (arg1 <= 0)
		{
			//TODO search by name, too lazy right now
		}
		else
		{
			const SoundInfo *sound = configModel->getSoundInfo(arg1);
			if (sound)
				sb_playFile(*sound);
			else
				return 1;
		}
	}
	return 0;
}

void sb_playButton(int btn)
{
    if ((nullptr != configDialog) && (configDialog->hotkeysEnabled()))
    {
        const SoundInfo *sound = configModel->getSoundInfo(btn);
        if (sound)
            sb_playFile(*sound);
    }
}

void sb_setConfig(int cfg)
{
    if (configDialog)
        configDialog->setConfiguration(cfg);
}

void sb_openAbout()
{
	if(!aboutDialog)
		aboutDialog = new AboutQt();
	aboutDialog->show();
}


void sb_onConnectStatusChange(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) 
{
    Q_UNUSED(errorNumber)

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


void sb_getInternalHotkeyName(int buttonId, char *buf)
{
	sprintf(buf, "button_%i", buttonId + 1);
}


void sb_getInternalConfigHotkeyName(int configId, char *buf)
{
	sprintf(buf, "config_%i", configId);
}


void sb_onHotkeyRecordedEvent(const char *keyword, const char *key)
{
	if (configDialog)
		configDialog->onHotkeyRecordedEvent(keyword, key);
}


void sb_onStopTalking()
{
	tsMgr->onClientStopsTalking();
}

void sb_onHotkeyPressed(const char * keyword)
{
	if (hotkeysTemporarilyDisabled)
		return;

	int btn = -1;
	if (sscanf(keyword, "button_%i", &btn) > 0)
	{
		sb_playButton(btn - 1);
	}
	else if (sscanf(keyword, "config_%i", &btn) > 0)
	{
		sb_setConfig(btn);
	}
	else if (strcmp(keyword, HOTKEY_STOP_ALL) == 0)
	{
		sb_stopPlayback();
	}
	else if (strcmp(keyword, HOTKEY_PAUSE_ALL) == 0)
	{
		sb_pauseButtonPressed();
	}
	else if (strcmp(keyword, HOTKEY_MUTE_MYSELF) == 0)
	{
		configModel->setMuteMyselfDuringPb(!configModel->getMuteMyselfDuringPb());
	}
	else if (strcmp(keyword, HOTKEY_MUTE_ON_MY_CLIENT) == 0)
	{
		configModel->setPlaybackLocal(!configModel->getPlaybackLocal());
	}
	else if (strcmp(keyword, HOTKEY_VOLUME_INCREASE) == 0)
	{
		configModel->setVolumeRemote(std::min(configModel->getVolumeRemote() + 20, 100));
		configModel->setVolumeLocal(std::min(configModel->getVolumeLocal() + 20, 100));
	}
	else if (strcmp(keyword, HOTKEY_VOLUME_DECREASE) == 0)
	{
		configModel->setVolumeRemote(std::max(configModel->getVolumeRemote() - 20, 0));
		configModel->setVolumeLocal(std::max(configModel->getVolumeLocal() - 20, 0));
	}
}


void sb_checkForUpdates()
{
	if (!updateChecker)
		updateChecker = new UpdateChecker();
	updateChecker->startCheck(true);
}

/** return 0 if the command was handled, 1 otherwise */
int sb_parseCommand(char** args, int argc)
{
	if (argc >= 3)
		ts3Functions.printMessageToCurrentTab("Too many arguments");
	else if (argc == 0)
		sb_openDialog();
	else if (argc == 1)
	{
		long arg1 = strtol(args[0], nullptr, 10);
		if (strcmp(args[0], "stop")==0)
			sb_stopPlayback();
		else if (strcmp(args[0], "-?") == 0)
			ts3Functions.printMessageToCurrentTab("Arguments: 'stop' to stop playback or '[configuration number] <button number>'");
		else if (sb_playButtonEx(args[0]) != 0)
			ts3Functions.printMessageToCurrentTab("No such button found");
	}
	else if (argc == 2)
	{
		long arg0 = strtol(args[0], nullptr, 10);
		int pconfig = configModel->getConfiguration(); //TODO ConfigModel::getConfiguration() { return m_activeConfig; }
		if (arg0 < 1 || arg0 > 4)
			ts3Functions.printMessageToCurrentTab("Invalid configuration number");
		configModel->setConfiguration((int)arg0); //switch to specified configuration
		if (sb_playButtonEx(args[0]) != 0)
			ts3Functions.printMessageToCurrentTab("No such button found");
		configModel->setConfiguration(pconfig); //return to previous configuration

	}
	return 0;
}


void sb_disableHotkeysTemporarily(bool disable)
{
	hotkeysTemporarilyDisabled = disable;
}
