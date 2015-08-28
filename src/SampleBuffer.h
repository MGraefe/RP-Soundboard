// src/SampleBuffer.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#pragma once
#ifndef rpsbsrc__SampleBuffer_H__
#define rpsbsrc__SampleBuffer_H__

#include <vector>
#include <mutex>

#include "SampleProducer.h"

class SampleBuffer : public SampleProducer
{
public:
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
		m_cbProd = cb;
	}

	//Get the callback that is called when samples are placed into the buffer (produced)
	inline ConsumeCallback *getOnProduce() const {
		return m_cbCons;
	}

	//Set the callback that is called when samples are read from the buffer (consumed)
	inline void setOnConsume(ConsumeCallback *cb) {
		m_cbCons = cb;
	}

	//Get the callback that is called when samples are read from the buffer (consumed)
	inline ConsumeCallback *getOnConsume() const {
		return m_cbCons;
	}

	//Get the number of available samples
	//One sample is (2 * channels) bytes in size
	inline int avail() const { 
		return m_buf.size() / m_channels; 
	}

	//Return the number of channels this buffer was initialized with
	inline int channels() const {
		return m_channels;
	}

	//Return max size as set
	inline size_t maxSize() const {
		return m_maxSize;
	}

	//Place some samples into the buffer
	//samples: The sample buffer
	//count: Number of samples in buffer
	//One sample is (2 * channels) bytes in size
	void produce(const short *samples, int count) override;

	//Consume some samples from the buffer
	//samples: The sample buffer
	//count: Size of buffer measured in Samples
	//eraseConsumed: Erase the consumed samples from the buffer
	//One sample is (2 * channels) bytes in size
	int consume(short *samples, int maxCount, bool eraseConsumed = true);

	//Get size of a sample in bytes
	inline int sampleSize() const {
		return 2 * m_channels;
	}

	//Copy a number of samples to another buffer
	void copyToOther(SampleBuffer *other, int count, int start = 0) const;

	//Directly return bare memory adress
	//Be careful!
	inline short *getBufferData() {
		return m_buf.data();
	}
private:
	const int m_channels;
	const size_t m_maxSize;
	std::vector<short> m_buf;
	mutable std::mutex m_mutex;
	ProduceCallback *m_cbProd;
	ConsumeCallback *m_cbCons;
};

#endif // rpsbsrc__SampleBuffer_H__