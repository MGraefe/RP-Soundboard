// src/SampleProducerThread.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include <thread>
#include <algorithm>
#include <cassert>

#include "SampleBuffer.h"
#include "SampleSource.h"
#include "SampleProducerThread.h"


SampleProducerThread::SampleProducerThread() :
	m_source(nullptr),
	m_running(false),
	m_stop(false)
{
}


void SampleProducerThread::start()
{
	if (m_running)
		return;

	m_running = true;
	m_stop = false;
	std::thread t(&SampleProducerThread::threadFunc, this);
	m_thread = std::move(t);
}


void SampleProducerThread::stop(bool wait)
{
	m_stop = true;
	if (wait && m_thread.joinable())
		m_thread.join();
}


bool SampleProducerThread::isRunning()
{
	return m_running;
}


void SampleProducerThread::setSource(SampleSource* source)
{
	m_mutex.lock();
	m_source = source;
	m_mutex.unlock();
}

#define MIN_BUFFER_SAMPLES (48000 / 2)
void SampleProducerThread::run()
{
	while (!m_stop)
	{
		m_mutex.lock();
		if (m_source)
			singleBufferFill();
		m_mutex.unlock();

		// We now have half a second of samples available and have done
		// so much work that we deserve a little rest
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}


void SampleProducerThread::threadFunc()
{
	run();
	m_running = false;
}


void SampleProducerThread::addBuffer(SampleBuffer* buffer, bool enableBuffer /*= true*/)
{
	Lock lock(m_mutex);
	if (std::find_if(m_buffers.begin(), m_buffers.end(), [buffer](const buffer_t& b) { return b.buffer == buffer; }) ==
		m_buffers.end())
	{
		buffer_t b = {buffer, enableBuffer};
		m_buffers.push_back(b);
	}
}


void SampleProducerThread::remBuffer(SampleBuffer* buffer)
{
	Lock lock(m_mutex);
	auto it =
		std::find_if(m_buffers.begin(), m_buffers.end(), [buffer](const buffer_t& b) { return b.buffer == buffer; });
	if (it != m_buffers.end())
		m_buffers.erase(it);
}


void SampleProducerThread::setBufferEnabled(SampleBuffer* buffer, bool enabled)
{
	Lock lock(m_mutex);
	auto it =
		std::find_if(m_buffers.begin(), m_buffers.end(), [buffer](const buffer_t& b) { return b.buffer == buffer; });
	if (it != m_buffers.end())
		it->enabled = enabled;
}


void SampleProducerThread::produce(const short* samples, int count)
{
	for (const buffer_t& buffer : m_buffers)
	{
		if (buffer.enabled)
		{
			SampleBuffer::Lock lock(buffer.buffer->getMutex());
			buffer.buffer->produce(samples, count);
		}
	}
}


bool SampleProducerThread::singleBufferFill()
{
	for (const buffer_t& buffer : m_buffers)
	{
		if (buffer.enabled)
		{
			std::unique_lock<SampleBuffer::Mutex> sbl(buffer.buffer->getMutex());
			while (buffer.buffer->avail() < MIN_BUFFER_SAMPLES)
			{
				assert(buffer.buffer->maxSize() > MIN_BUFFER_SAMPLES && "Buffer too small");
				sbl.unlock();
				int samples = m_source->readSamples(this);
				if (samples < 0) // error
					return false;
				if (samples == 0) // file is done
					return true;
				sbl.lock();
			}
		}
	}
	return true;
}
