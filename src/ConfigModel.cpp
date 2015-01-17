
#include "common.h"

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
	m_playbackLocal = 1;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::readConfig()
{
	QSettings settings(GetFullConfigPath(), QSettings::IniFormat);

	int size = settings.beginReadArray("files");
	m_fns.resize(size);
	for(int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		QByteArray ba = settings.value("path").toString().toLocal8Bit();
		m_fns[i] = ba.data();
	}
	settings.endArray();

	m_rows = settings.value("num_rows", 4).toInt();
	m_cols = settings.value("num_cols", 6).toInt();
	m_volume = settings.value("volume", 50).toInt();
	m_playbackLocal = settings.value("playback_local", true).toBool();

	//Notify all changes
	for(int i = 0; i < m_fns.size(); i++)
		notify(NOTIFY_SET_FILENAME, i);
	notify(NOTIFY_SET_COLS, m_cols);
	notify(NOTIFY_SET_ROWS, m_rows);
	notify(NOTIFY_SET_VOLUME, m_volume);
	notify(NOTIFY_SET_PLAYBACK_LOCAL, m_playbackLocal);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::writeConfig()
{
	QSettings settings(GetFullConfigPath(), QSettings::IniFormat);

	settings.beginWriteArray("files");
	for(int i = 0; i < m_fns.size(); i++)
	{
		settings.setArrayIndex(i);
		QString value = m_fns[i].c_str();
		settings.setValue("path", value);
	}
	settings.endArray();

	settings.setValue("num_rows", m_rows);
	settings.setValue("num_cols", m_cols);
	settings.setValue("volume", m_volume);
	settings.setValue("playback_local", m_playbackLocal);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
const char * ConfigModel::getFileName( int itemId )
{
	if(itemId >= 0 && itemId < m_fns.size())
		return m_fns[itemId].c_str();
	return NULL;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void ConfigModel::setFileName( int itemId, const char *fn )
{
	if(itemId >= 0)
	{
		if(itemId < 1000 && itemId >= m_fns.size())
			m_fns.resize(itemId + 1);
		m_fns[itemId] = fn;
		sb_saveConfig();
		notify(NOTIFY_SET_FILENAME, itemId);
	}
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
	writeConfig();
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