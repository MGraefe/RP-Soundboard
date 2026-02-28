// src/SampleBuffer.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#pragma once

#include <vector>
#include <mutex>
#include <cassert>


#include "SampleProducer.h"

class SampleBuffer : public SampleProducer
{
public:
	typedef std::mutex Mutex;
	typedef std::lock_guard<Mutex> Lock;

	class ProduceCallback
	{
	public:
		virtual void onProduceSamples(const short *samples, int count, SampleBuffer *caller) = 0;
	};

	class ConsumeCallback
	{
	public:
		virtual void onConsumeSamples(const short *samples, int count, SampleBuffer *caller) = 0;
	};

public:
	SampleBuffer(int channels, size_t maxSize = 0);

	//Set the callback that is called when samples are placed into the buffer (produced)
	inline void setOnProduce(ProduceCallback *cb) { 
		assert(!m_mutex.try_lock() && "Mutex not locked");
		m_cbProd = cb;
	}

	//Get the callback that is called when samples are placed into the buffer (produced)
	inline ConsumeCallback *getOnProduce() const {
		assert(!m_mutex.try_lock() && "Mutex not locked");
		return m_cbCons;
	}

	//Set the callback that is called when samples are read from the buffer (consumed)
	inline void setOnConsume(ConsumeCallback *cb) {
		assert(!m_mutex.try_lock() && "Mutex not locked");
		m_cbCons = cb;
	}

	//Get the callback that is called when samples are read from the buffer (consumed)
	inline ConsumeCallback *getOnConsume() const {
		assert(!m_mutex.try_lock() && "Mutex not locked");
		return m_cbCons;
	}

	//Get the number of available samples
	//One sample is (2 * channels) bytes in size
	inline int avail() const { 
		assert(!m_mutex.try_lock() && "Mutex not locked");
		return m_buf.size() / m_channels; 
	}

	//Return the number of channels this buffer was initialized with
	inline int channels() const {
		assert(!m_mutex.try_lock() && "Mutex not locked");
		return m_channels;
	}

	//Return max size as set
	inline size_t maxSize() const {
		assert(!m_mutex.try_lock() && "Mutex not locked");
		return m_maxSize;
	}

	//Place some samples into the buffer
	//samples: The sample buffer
	//count: Number of samples in buffer
	//One sample is (2 * channels) bytes in size
	virtual void produce(const short *samples, int count) override;

	//Consume some samples from the buffer
	//samples: The sample buffer
	//count: Size of buffer measured in Samples
	//eraseConsumed: Erase the consumed samples from the buffer
	//One sample is (2 * channels) bytes in size
	int consume(short *samples, int maxCount, bool eraseConsumed = true);

	//Get size of a sample in bytes
	inline int sampleSize() const {
		assert(!m_mutex.try_lock() && "Mutex not locked");
		return 2 * m_channels;
	}

	//Directly return bare memory adress
	//Be careful!
	inline short *getBufferData() {
		assert(!m_mutex.try_lock() && "Mutex not locked");
		return m_buf.data();
	}

	inline const std::mutex &getMutex() const {
		return m_mutex;
	}

	inline std::mutex &getMutex() {
		return m_mutex;
	}

private:
	const int m_channels;
	const size_t m_maxSize;
	std::vector<short> m_buf;
	mutable std::mutex m_mutex;
	ProduceCallback *m_cbProd;
	ConsumeCallback *m_cbCons;
};

