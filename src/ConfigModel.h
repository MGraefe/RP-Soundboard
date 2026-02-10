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
#include <array>
#include "SoundInfo.h"
#include <QString>
#include <memory>
#include <set>
#include "common.h"
#include <QVector>


class ConfigModel
{
public:
	enum notifications_e
	{
		NOTIFY_SET_SOUND,
		NOTIFY_SET_ROWS,
		NOTIFY_SET_COLS,
		NOTIFY_SET_VOLUME_LOCAL,
		NOTIFY_SET_VOLUME_REMOTE,
		NOTIFY_SET_PLAYBACK_LOCAL,
		NOTIFY_SET_MUTE_MYSELF_DURING_PB,
		NOTIFY_SET_WINDOW_SIZE,
		NOTIFY_SET_BUBBLE_BUTTONS_BUILD,
		NOTIFY_SET_BUBBLE_STOP_BUILD,
		NOTIFY_SET_BUBBLE_COLS_BUILD,
		NOTIFY_SET_SHOW_HOTKEYS_ON_BUTTONS,
		NOTIFY_SET_HOTKEYS_ENABLED,
		NOTIFY_SET_NEXT_UPDATE_CHECK,
	};

	class Observer
	{
	public:
        virtual ~Observer() {}
		virtual void notify(ConfigModel &model, notifications_e what, int data) = 0;
	};

public:
	ConfigModel();

	// Playlist (independent from button grid)
	QVector<SoundInfo> getPlaylist() const;
	void setPlaylist(const QVector<SoundInfo>& items);

	bool getPlaylistShuffle() const { return m_playlistShuffle; }
	void setPlaylistShuffle(bool v);

	int getPlaylistRepeatMode() const { return m_playlistRepeatMode; } // 0=None,1=All,2=One
	void setPlaylistRepeatMode(int v);

	void readConfig(const QString &file = QString());
    void writeConfig(const QString &file = QString());

	void notifyAllEvents();
	
	static QString GetConfigPath();
	static QString GetFullConfigPath();

	QString getFileName(int itemId) const;
	void setFileName(int itemId, const QString &fn);

	const SoundInfo *getSoundInfo(int itemId) const;
	void setSoundInfo(int itemId, const SoundInfo &info);

	inline int getRows() const { return m_rows[m_activeConfig]; }
	void setRows(int n);

	inline int getCols() const { return m_cols[m_activeConfig]; }
	void setCols(int n);

	inline int getVolumeLocal() const { return m_volumeLocal; }
	void setVolumeLocal(int val);

	inline int getVolumeRemote() const { return m_volumeRemote; }
	void setVolumeRemote(int val);

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

	inline bool getHotkeysEnabled() const { return m_hotkeysEnabled; }
	void setHotkeysEnabled(bool enabled);

	void addObserver(Observer *obs);
	void remObserver(Observer *obs);

    void setConfiguration(int config);
	int getConfiguration();

	const std::vector<SoundInfo> &sounds() const { return m_sounds[m_activeConfig]; }
    int numSounds() const { return (int)sounds().size(); }

	uint getNextUpdateCheck() const { return m_nextUpdateCheck; }
	void setNextUpdateCheck(uint time);

private:
	std::vector<SoundInfo> &sounds() { return m_sounds[m_activeConfig]; }
	void notify(notifications_e what, int data);
	std::vector<SoundInfo> getInitialSounds();
	std::vector<SoundInfo> readConfiguration(QSettings & settings, const QString &name);
    void writeConfiguration(QSettings & settings, const QString &name, const std::vector<SoundInfo> &sounds);

	std::vector<Observer*> m_obs;
	std::array<std::vector<SoundInfo>, NUM_CONFIGS> m_sounds;
	int m_activeConfig;

    std::array<int, NUM_CONFIGS> m_rows;
	std::array<int, NUM_CONFIGS> m_cols;
	int m_volumeLocal;
	int m_volumeRemote;
	bool m_playbackLocal;
	bool m_muteMyselfDuringPb;
	int m_windowWidth;
	int m_windowHeight;

	int m_bubbleButtonsBuild;
	int m_bubbleStopBuild;
	int m_bubbleColsBuild;

	bool m_showHotkeysOnButtons;
	bool m_hotkeysEnabled;

	uint m_nextUpdateCheck;
	QVector<SoundInfo> m_playlist;
	bool m_playlistShuffle = false;
	int m_playlistRepeatMode = 0;


};



#endif // rpsbsrc__ConfigModel_H__
