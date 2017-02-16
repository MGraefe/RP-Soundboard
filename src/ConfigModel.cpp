// src/ConfigModel.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "common.h"

#include <QtCore/QSettings>

#include "ConfigModel.h"
#include "main.h"
#include "buildinfo.h"
#include "plugin.h"



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigModel::ConfigModel()
{
    m_rows = 2;
	m_cols = 5;
	m_volume = 80;
	m_playbackLocal = true;
	m_muteMyselfDuringPb = false;
	m_windowWidth = 600;
	m_windowHeight = 240;

	m_bubbleButtonsBuild = 0;
	m_bubbleStopBuild = 0;
	m_bubbleColsBuild = 0;

	m_showHotkeysOnButtons = false;
	m_hotkeysEnabled = true;

    m_activeConfig = 0;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::readConfig(const QString &file)
{
    QString path;
    if (file.isEmpty())
        path = GetFullConfigPath();
    else
        path = file;

    QSettings settings(path, QSettings::IniFormat);

	for (int i = 0; i < NUM_CONFIGS; i++)
		m_sounds[i] = readConfiguration(settings, i == 0 ? QString("files") : QString("files%1").arg(i+1));

    m_rows = settings.value("num_rows", 2).toInt();
	m_cols = settings.value("num_cols", 5).toInt();
	m_volume = settings.value("volume", 50).toInt();
	m_playbackLocal = settings.value("playback_local", true).toBool();
	m_muteMyselfDuringPb = settings.value("mute_myself_during_pb", false).toBool();
	m_windowWidth = settings.value("window_width", 600).toInt();
	m_windowHeight = settings.value("window_height", 240).toInt();
	m_bubbleButtonsBuild = settings.value("bubble_buttons_build", 0).toInt();
	m_bubbleStopBuild = settings.value("bubble_stop_build", 0).toInt();
	m_bubbleColsBuild = settings.value("bubble_cols_build", 0).toInt();
	m_showHotkeysOnButtons = settings.value("show_hotkeys_on_buttons", false).toBool();
	m_hotkeysEnabled = settings.value("hotkeys_enabled", true).toBool();

	notifyAllEvents();
}

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::writeConfig(const QString &file)
{
    QString path;
    if (file.isEmpty())
        path = GetFullConfigPath();
    else
        path = file;

    QSettings settings(path, QSettings::IniFormat);

    settings.setValue("config_build", buildinfo_getBuildNumber());
    settings.setValue("num_rows", m_rows);
    settings.setValue("num_cols", m_cols);
    settings.setValue("volume", m_volume);
    settings.setValue("playback_local", m_playbackLocal);
    settings.setValue("mute_myself_during_pb", m_muteMyselfDuringPb);
    settings.setValue("window_width", m_windowWidth);
    settings.setValue("window_height", m_windowHeight);
    settings.setValue("bubble_buttons_build", m_bubbleButtonsBuild);
    settings.setValue("bubble_stop_build", m_bubbleStopBuild);
    settings.setValue("bubble_cols_build", m_bubbleColsBuild);
    settings.setValue("show_hotkeys_on_buttons", m_showHotkeysOnButtons);
	settings.setValue("hotkeys_enabled", m_hotkeysEnabled);

	for (int i = 0; i < NUM_CONFIGS; i++)
		writeConfiguration(settings, i == 0 ? QString("files") : QString("files%1").arg(i + 1), m_sounds[i]);
}


std::vector<SoundInfo> ConfigModel::readConfiguration(QSettings &settings, const QString &name)
{
	std::vector<SoundInfo> sounds;
    int size = settings.beginReadArray(name);
    if (size == 0)
        sounds = getInitialSounds();
    else
    {
        sounds.resize(size);
        for (int i = 0; i < size; i++)
        {
            settings.setArrayIndex(i);
            sounds[i].readFromConfig(settings);
        }
    }
    settings.endArray();
	return sounds;
}


void ConfigModel::writeConfiguration(QSettings & settings, const QString &name, const std::vector<SoundInfo> &sounds)
{
    settings.beginWriteArray(name);
    for (int i = 0; i < sounds.size(); i++)
    {
        settings.setArrayIndex(i);
        sounds[i].saveToConfig(settings);
    }
    settings.endArray();
}


void ConfigModel::setConfiguration(int config)
{
	m_activeConfig = config;

    /* Tell observers that our data changed */
    notifyAllEvents();
}

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString ConfigModel::getFileName( int itemId ) const
{
	if(itemId >= 0 && itemId < sounds().size())
		return sounds()[itemId].filename;
	return QString();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setFileName( int itemId, const QString &fn )
{
	if(itemId >= 0)
	{
		if(itemId < 1000 && itemId >= sounds().size())
			sounds().resize(itemId + 1);
		sounds()[itemId].filename = fn;
		writeConfig();
		notify(NOTIFY_SET_SOUND, itemId);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
const SoundInfo *ConfigModel::getSoundInfo(int itemId) const
{
	if(itemId >= 0 && itemId < sounds().size())
		return &sounds()[itemId];
	return NULL;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setSoundInfo( int itemId, const SoundInfo &info )
{
	if(itemId < 1000 && itemId >= sounds().size())
		sounds().resize(itemId + 1);
	sounds()[itemId] = info;
	writeConfig();
	notify(NOTIFY_SET_SOUND, itemId);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString ConfigModel::GetConfigPath()
{
	// Find config path for config class
	char* configPath = (char*)malloc(PATH_BUFSIZE);
	ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);
	return QString::fromUtf8(configPath);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString ConfigModel::GetFullConfigPath()
{
	QString fullPath = GetConfigPath();
	QChar last = fullPath[fullPath.count() - 1];
	if (last != '/' && last != '\\')
		fullPath.append('/');
	fullPath.append("rp_soundboard.ini");
	return fullPath;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setRows( int n )
{
	m_rows = n;
	writeConfig();
	notify(NOTIFY_SET_ROWS, n);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setCols( int n )
{
    //if (n > m_cols)
    //{
    //    for (int i = m_rows - 1; i >= 1; i--)
    //        for (int k = m_cols - 1; k >= 0; k--)
    //            if (m_sounds->size() > (i * m_cols + k))
    //            {
    //                int index = i * n + k;
    //                if (m_sounds->size() <= index)
    //                    m_sounds->resize(index + 1);
    //                (*m_sounds)[index] = (*m_sounds)[i * m_cols + k];
    //            }
    //}
    //else if (n < m_cols)
    //{
    //    for (int i = 1; i < m_rows; i++)
    //        for (int k = 0; k < m_cols; k++)
    //            if (m_sounds->size() > (i * m_cols + k))
    //                (*m_sounds)[i * n + k] = (*m_sounds)[i * m_cols + k];
    //}

	m_cols = n;
	writeConfig();
	notify(NOTIFY_SET_COLS, n);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setVolume( int val )
{
	m_volume = val;
	notify(NOTIFY_SET_VOLUME, val);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setPlaybackLocal( bool val )
{
	m_playbackLocal = val;
	writeConfig();
	notify(NOTIFY_SET_PLAYBACK_LOCAL, val ? 1 : 0);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setMuteMyselfDuringPb(bool val)
{
	m_muteMyselfDuringPb = val;
	writeConfig();
	notify(NOTIFY_SET_MUTE_MYSELF_DURING_PB, val ? 1 : 0);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::getWindowSize(int *width, int *height) const
{
	if(width)
		*width = m_windowWidth;
	if(height)
		*height = m_windowHeight;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setWindowSize(int width, int height)
{
	m_windowWidth = width;
	m_windowHeight = height;
	notify(NOTIFY_SET_WINDOW_SIZE, 0);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::notify(notifications_e what, int data)
{
	for(Observer *obs : m_obs)
		obs->notify(*this, what, data);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setHotkeysEnabled(bool enabled)
{
	m_hotkeysEnabled = enabled;
	writeConfig();
	notify(NOTIFY_SET_HOTKEYS_ENABLED, enabled ? 1 : 0);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::addObserver(Observer *obs)
{
	m_obs.push_back(obs);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::remObserver(Observer *obs)
{
	m_obs.erase(std::remove(m_obs.begin(), m_obs.end(), obs), m_obs.end());
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setBubbleButtonsBuild(int build)
{
	m_bubbleButtonsBuild = build;
	writeConfig();
	notify(NOTIFY_SET_BUBBLE_BUTTONS_BUILD, build);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setBubbleStopBuild(int build)
{
	m_bubbleStopBuild = build;
	writeConfig();
	notify(NOTIFY_SET_BUBBLE_STOP_BUILD, build);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setBubbleColsBuild(int build)
{
	m_bubbleColsBuild = build;
	writeConfig();
	notify(NOTIFY_SET_BUBBLE_COLS_BUILD, build);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
std::vector<SoundInfo> ConfigModel::getInitialSounds()
{
	char* pluginPath = (char*)malloc(PATH_BUFSIZE);
	ts3Functions.getPluginPath(pluginPath, PATH_BUFSIZE, getPluginID());
	QString fullPath = QString::fromUtf8(pluginPath);
	QChar last = fullPath[fullPath.count() - 1];
	if (last != '/' && last != '\\')
		fullPath.append('/');
	fullPath.append("rp_soundboard/");

	static const char* files[] = {
		"Airhorn Sonata.mp3",
		"Airhorn.mp3",
		"Airporn.mp3",
		"Peter Griffin Laugh.mp3",
		"Spooky.mp3",
		NULL,
	};

	std::vector<SoundInfo> sounds;
	for (int i = 0; files[i] != NULL; i++)
	{
		SoundInfo info;
		info.filename = fullPath + files[i];
		sounds.push_back(info);
	}
	return sounds;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::notifyAllEvents()
{
	//Notify all changes
	for(int i = 0; i < sounds().size(); i++)
		notify(NOTIFY_SET_SOUND, i);
	notify(NOTIFY_SET_COLS, m_cols);
	notify(NOTIFY_SET_ROWS, m_rows);
	notify(NOTIFY_SET_VOLUME, m_volume);
	notify(NOTIFY_SET_PLAYBACK_LOCAL, m_playbackLocal);
	notify(NOTIFY_SET_MUTE_MYSELF_DURING_PB, m_muteMyselfDuringPb);
	notify(NOTIFY_SET_WINDOW_SIZE, 0);
	notify(NOTIFY_SET_BUBBLE_BUTTONS_BUILD, m_bubbleButtonsBuild);
	notify(NOTIFY_SET_BUBBLE_STOP_BUILD, m_bubbleStopBuild);
	notify(NOTIFY_SET_BUBBLE_COLS_BUILD, m_bubbleColsBuild);
	notify(NOTIFY_SET_SHOW_HOTKEYS_ON_BUTTONS, m_showHotkeysOnButtons);
	notify(NOTIFY_SET_HOTKEYS_ENABLED, m_hotkeysEnabled);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setShowHotkeysOnButtons(bool show)
{
	m_showHotkeysOnButtons = show;
	writeConfig();
	notify(NOTIFY_SET_SHOW_HOTKEYS_ON_BUTTONS, show ? 1 : 0);
}

