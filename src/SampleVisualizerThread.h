// src/SampleVisualizerThread.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <string>

#include "SampleBuffer.h"


class InputFile;
class SampleVisualizerThread
{
  public:
	class SampleBufferSynced : public SampleBuffer
	{
	  public:
		SampleBufferSynced(int channels, size_t maxSize = 0);
		virtual void produce(const short* samples, int count) override;
	};

  public:
	SampleVisualizerThread();
	~SampleVisualizerThread();
	void startAnalysis(const char* filename, size_t numBins);
	void stop(bool wait = true);
	bool isRunning() const;
	size_t getBinsProcessed() const;

	// Array with numBins * 2 values (min and max value)
	volatile const int* getBins() const;

	inline int64_t getTotalSamplesEst() const
	{
		return m_numSamplesTotalEst;
	}

	// Get file length in seconds, might be an estimation when processing isn't finished yet
	double fileLength() const;

	static SampleVisualizerThread& GetInstance();

  private:
	void run();
	void threadFunc();
	void openNewFile();
	void processSamples(size_t newSamples);
	static void getMinMax(const short* data, size_t count, int& min, int& max);

	typedef std::lock_guard<std::mutex> Lock;

  private:
	SampleBufferSynced m_buffer;
	size_t m_numBins;
	size_t m_numBinsProcessed;
	int64_t m_numSamplesProcessed;
	int64_t m_numSamplesTotalEst;
	size_t m_numSamplesProcessedThisBin;
	int m_min;
	int m_max;
	mutable std::mutex m_mutex;
	std::vector<int> m_bins;
	InputFile* m_file;
	std::thread m_thread;
	std::string m_filename;
	volatile bool m_running;
	volatile bool m_newFile;
	volatile bool m_stop;
};
