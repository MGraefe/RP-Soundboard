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
		NOTIFY_SET_SHOW_HOTKEYS_ON_BUTTONS,
	};

	class Observer
	{
	public:
		virtual void notify(ConfigModel &model, notifications_e what, int data) = 0;
	};

public:
	ConfigModel();

	void readConfig(const QString &file = QString());
    void writeConfig(const QString &file = QString());

	void notifyAllEvents();
	
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

	inline bool getShowHotkeysOnButtons() const { return m_showHotkeysOnButtons; }
	void setShowHotkeysOnButtons(bool show);

	void addObserver(Observer *obs);
	void remObserver(Observer *obs);

    void setConfiguration(int config);

private:
	void notify(notifications_e what, int data);
	void fillInitialSounds(std::vector<SoundInfo> *sounds);
    void readConfiguration(QSettings & settings, const QString &name, std::vector<SoundInfo> *sounds);
    void writeConfiguration(QSettings & settings, const QString &name, std::vector<SoundInfo> *sounds);

	std::vector<Observer*> m_obs;
	std::vector<SoundInfo> * m_sounds;

    std::vector<SoundInfo> m_sounds1;
    std::vector<SoundInfo> m_sounds2;
    std::vector<SoundInfo> m_sounds3;
    std::vector<SoundInfo> m_sounds4;

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

	bool m_showHotkeysOnButtons;
};

#endif // rpsbsrc__ConfigModel_H__