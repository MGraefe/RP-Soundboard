// src/PlaylistController.h
//----------------------------------
// RP Soundboard Playlist Feature
// Copyright (c) 2025 SaeedPakniat
//----------------------------------

#pragma once

#include <QObject>
#include <QVector>
#include "SoundInfo.h"

class Sampler;
class ConfigModel;

class PlaylistController : public QObject
{
    Q_OBJECT
public:
    enum class RepeatMode { None = 0, All = 1, One = 2 };

    PlaylistController(Sampler* sampler, ConfigModel* model, QObject* parent = nullptr);

    // Playlist data (stored in ConfigModel)
    void addFiles(const QStringList& paths);
    void setShuffle(bool enabled);
    void setRepeatMode(RepeatMode mode);

    bool shuffle() const { return m_shuffle; }
    RepeatMode repeatMode() const { return m_repeat; }

    // Playback controls
    void play(int startIndex = 0);
    void playSelected(int index);
    void stop();
    void next();
    void prev();

    bool isPlayingPlaylist() const { return m_playingPlaylist; }
    int currentIndex() const { return m_currentIndex; }
    void notifyPlaylistChanged() { emit playlistChanged(); }

signals:
    void playlistChanged();
    void nowPlayingChanged(int index);




private slots:
    void onSamplerStopped();

private:
    QVector<SoundInfo> getPlaylist() const;
    void setPlaylist(const QVector<SoundInfo>& items);

    void playIndex(int index);
    void rebuildOrder();
    int nextIndexAfterStop();

private:
    Sampler* m_sampler = nullptr;
    ConfigModel* m_model = nullptr;

    bool m_playingPlaylist = false;
    bool m_shuffle = false;
    bool m_ignoreNextStop = false;
    RepeatMode m_repeat = RepeatMode::None;

    int m_currentIndex = -1;

    QVector<int> m_order;
    int m_orderPos = -1;
};
