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
#include "device.h"



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
ConfigModel::ConfigModel()
{
	m_rows = 4;
	m_cols = 6;
	m_volume = 50;
	m_playbackLocal = true;
	m_muteMyselfDuringPb = false;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::readConfig()
{
	QSettings settings(GetFullConfigPath(), QSettings::IniFormat);

	int size = settings.beginReadArray("files");
	m_sounds.resize(size);
	for(int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		m_sounds[i].readFromConfig(settings);
	}
	settings.endArray();

	m_rows = settings.value("num_rows", 4).toInt();
	m_cols = settings.value("num_cols", 6).toInt();
	m_volume = settings.value("volume", 50).toInt();
	m_playbackLocal = settings.value("playback_local", true).toBool();
	m_muteMyselfDuringPb = settings.value("mute_myself_during_pb", false).toBool();
	m_windowWidth = settings.value("window_width", 650).toInt();
	m_windowHeight = settings.value("window_height", 450).toInt();

	//Notify all changes
	for(int i = 0; i < m_sounds.size(); i++)
		notify(NOTIFY_SET_SOUND, i);
	notify(NOTIFY_SET_COLS, m_cols);
	notify(NOTIFY_SET_ROWS, m_rows);
	notify(NOTIFY_SET_VOLUME, m_volume);
	notify(NOTIFY_SET_PLAYBACK_LOCAL, m_playbackLocal);
	notify(NOTIFY_SET_MUTE_MYSELF_DURING_PB, m_muteMyselfDuringPb);
	notify(NOTIFY_SET_WINDOW_SIZE, 0);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::writeConfig()
{
	QSettings settings(GetFullConfigPath(), QSettings::IniFormat);

	settings.beginWriteArray("files");
	for(int i = 0; i < m_sounds.size(); i++)
	{
		settings.setArrayIndex(i);
		m_sounds[i].saveToConfig(settings);
	}
	settings.endArray();

	settings.setValue("num_rows", m_rows);
	settings.setValue("num_cols", m_cols);
	settings.setValue("volume", m_volume);
	settings.setValue("playback_local", m_playbackLocal);
	settings.setValue("mute_myself_during_pb", m_muteMyselfDuringPb);
	settings.setValue("window_width", m_windowWidth);
	settings.setValue("window_height", m_windowHeight);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString ConfigModel::getFileName( int itemId ) const
{
	if(itemId >= 0 && itemId < m_sounds.size())
		return m_sounds[itemId].filename;
	return QString();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setFileName( int itemId, const QString &fn )
{
	if(itemId >= 0)
	{
		if(itemId < 1000 && itemId >= m_sounds.size())
			m_sounds.resize(itemId + 1);
		m_sounds[itemId].filename = fn;
		writeConfig();
		notify(NOTIFY_SET_SOUND, itemId);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
const SoundInfo *ConfigModel::getSoundInfo(int itemId) const
{
	if(itemId >= 0 && itemId < m_sounds.size())
		return &m_sounds[itemId];
	return NULL;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setSoundInfo( int itemId, const SoundInfo &info )
{
	if(itemId < 1000 && itemId >= m_sounds.size())
		m_sounds.resize(itemId + 1);
	m_sounds[itemId] = info;
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
	return configPath;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
QString ConfigModel::GetFullConfigPath()
{
	QString fullPath = GetConfigPath();
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

