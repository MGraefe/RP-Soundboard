// src/samples.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__samples_H__
#define rpsbsrc__samples_H__

#include <QObject>

#include "SampleBuffer.h"
#include "SampleProducerThread.h"
#include "peakmeter.h"

#include <mutex>
#include <atomic>

class InputFile;
class SoundInfo;


class Sampler : public QObject
{
	Q_OBJECT

public:
	enum state_e
	{
		eSILENT = 0,
		ePLAYING,
		ePAUSED,
		ePLAYING_PREVIEW,
	};

public:
	Sampler();
	~Sampler();
	void init();
	void shutdown();
	int fetchInputSamples(short *samples, int count, int channels, bool *finished);
	int fetchOutputSamples(short *samples, int count, int channels, const unsigned int *channelSpeakerArray, unsigned int *channelFillMask);
	bool playFile(const SoundInfo &sound);
	bool playPreview(const SoundInfo &sound);
	void stopPlayback();
	void setVolumeLocal(int vol);
	void setVolumeRemote(int vol);
	void setLocalPlayback(bool enabled);
	void setMuteMyself(bool enabled);
	void pausePlayback();
	void unpausePlayback();
	inline state_e getState() const { return m_state; }

signals:
	void onStartPlaying(bool preview, QString filename);
	void onStopPlaying();
	void onPausePlaying();
	void onUnpausePlaying();

private:
	void stopSoundInternal();
	bool playSoundInternal(const SoundInfo &sound, bool preview);
	void setVolumeDb(double decibel);
	int fetchSamples(SampleBuffer &sb, PeakMeter &pm, short *samples, int count, int channels, bool eraseConsumed, int ciLeft, int ciRight, bool overLeft, bool overRight);
	int findChannelId(unsigned int channel, const unsigned int *channelSpeakerArray, int count);
	inline short scale(int val) const {
		return (short)((val * m_volumeDivider) >> volumeScaleExp);
	}
	
private:
	SampleBuffer m_sbCapture;
	SampleBuffer m_sbPlayback;
	SampleProducerThread m_sampleProducerThread;
	InputFile *m_inputFile;
	PeakMeter m_peakMeterCapture;
	PeakMeter m_peakMeterPlayback;
	int m_volumeDivider;
	float m_volumeFactor;
	static const int volumeScaleExp = 12;
	double m_globalDbSettingLocal;
	double m_globalDbSettingRemote;
	double m_soundDbSetting;
	std::mutex m_mutex;
	std::atomic<state_e> m_state;
	bool m_localPlayback;
	bool m_muteMyself;
};


#endif // rpsbsrc__samples_H__
