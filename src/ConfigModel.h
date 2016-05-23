// src/ConfigModel.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#pragma once
#ifndef rpsbsrc__ConfigModel_H__
#define rpsbsrc__ConfigModel_H__

#include <vector>
#include "SoundInfo.h"
#include <QtCore/QString>
#include "HotkeyInfo.h"
#include <memory>
#include <set>

class ConfigModel
{
public:
	enum notifications_e
	{
		NOTIFY_SET_SOUND,
		NOTIFY_SET_ROWS,
		NOTIFY_SET_COLS,
		NOTIFY_SET_VOLUME,
		NOTIFY_SET_PLAYBACK_LOCAL,
		NOTIFY_SET_MUTE_MYSELF_DURING_PB,
		NOTIFY_SET_WINDOW_SIZE,
		NOTIFY_SET_BUBBLE_BUTTONS_BUILD,
		NOTIFY_SET_BUBBLE_STOP_BUILD,
		NOTIFY_SET_BUBBLE_COLS_BUILD,
	};

	class Observer
	{
	public:
		virtual void notify(ConfigModel &model, notifications_e what, int data) = 0;
	};

	typedef HotkeyInfo* HotkeyPtr;
	typedef std::map<int, HotkeyPtr> HotkeyMap;

public:
	ConfigModel();

	void readConfig();

	void notifyAllEvents();

	void writeConfig();
	
	static QString GetConfigPath();
	static QString GetFullConfigPath();

	QString getFileName(int itemId) const;
	void setFileName(int itemId, const QString &fn);

	const SoundInfo *getSoundInfo(int itemId) const;
	void setSoundInfo(int itemId, const SoundInfo &info);

	inline int getRows() const { return m_rows; }
	void setRows(int n);

	inline int getCols() const { return m_cols; }
	void setCols(int n);

	inline int getVolume() const { return m_volume; }
	void setVolume(int val);

	inline bool getPlaybackLocal() const { return m_playbackLocal; }
	void setPlaybackLocal(bool val);
	
	inline bool getMuteMyselfDuringPb() const { return m_muteMyselfDuringPb; }
	void setMuteMyselfDuringPb(bool val);

	void getWindowSize(int *width, int *height) const;
	void setWindowSize(int width, int height);

	inline int getBubbleButtonsBuild() const { return m_bubbleButtonsBuild; }
	void setBubbleButtonsBuild(int build);

	inline int getBubbleStopBuild() const { return m_bubbleStopBuild; }
	void setBubbleStopBuild(int build);

	inline int getBubbleColsBuild() const { return m_bubbleColsBuild; }
	void setBubbleColsBuild(int build);

	HotkeyInfo *getHotkey(int itemId) const;
	const HotkeyMap &getHotkeys() const { return m_hotkeys; }
	void setHotkey(int itemId, HotkeyInfo hk);

	void addObserver(Observer *obs);
	void remObserver(Observer *obs);

private:
	void notify(notifications_e what, int data);
	void fillInitialSounds();

	std::vector<Observer*> m_obs;
	std::vector<SoundInfo> m_sounds;
	HotkeyMap m_hotkeys;

	int m_rows;
	int m_cols;
	int m_volume;
	bool m_playbackLocal;
	bool m_muteMyselfDuringPb;
	int m_windowWidth;
	int m_windowHeight;

	int m_bubbleButtonsBuild;
	int m_bubbleStopBuild;
	int m_bubbleColsBuild;
};

#endif // rpsbsrc__ConfigModel_H__