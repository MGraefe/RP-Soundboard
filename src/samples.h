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

#include <mutex>

class InputFile;
class SoundInfo;


class Sampler : public QObject
{
	Q_OBJECT

public:
	enum state_e
	{
		SILENT = 0,
		PLAYING,
		PLAYING_PREVIEW,
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
	void setVolume(int vol);
	void setLocalPlayback(bool enabled);
	void setMuteMyself(bool enabled);
	inline state_e getState() const { return m_state; }

signals:
	void onStartPlaying(bool preview, QString filename);
	void onStopPlaying();

private:
	bool playSoundInternal(const SoundInfo &sound, bool preview);
	void setVolumeDb(double decibel);
	int fetchSamples(SampleBuffer &sb, short *samples, int count, int channels, bool eraseConsumed, int ciLeft, int ciRight, bool overLeft, bool overRight);
	int findChannelId(unsigned int channel, const unsigned int *channelSpeakerArray, int count);
	inline short scale(int val) const {
		//return (short)((val << 16) / m_volumeDivider);
		return (short)((val * m_volumeDivider) >> 12);
	}
	
private:
	SampleBuffer m_sbCapture;
	SampleBuffer m_sbPlayback;
	SampleProducerThread m_sampleProducerThread;
	InputFile *m_inputFile;
	int m_volumeDivider;
	double m_globalDbSetting;
	double m_soundDbSetting;
	std::mutex m_mutex;
	state_e m_state;
	bool m_localPlayback;
	bool m_muteMyself;
};


#endif // rpsbsrc__samples_H__
