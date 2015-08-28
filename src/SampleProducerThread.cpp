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



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SampleProducerThread::SampleProducerThread() :
	m_source(NULL),
	m_running(false),
	m_stop(false)
{
	
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::start()
{
	if(m_running)
		return;

	m_running = true;
	m_stop = false;
	std::thread t(&SampleProducerThread::threadFunc, this);
	m_thread = std::move(t);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::stop(bool wait)
{
	m_stop = true;
	if(wait && m_thread.joinable())
		m_thread.join();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool SampleProducerThread::isRunning()
{
	return m_running;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::setSource( SampleSource *source )
{
	m_mutex.lock();
	m_source = source;
	m_mutex.unlock();
}

#define MIN_BUFFER_SAMPLES (48000 / 2)
//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::run()
{
	while(!m_stop)
	{
		m_mutex.lock();
		if(m_source)
		{
			while(singleBufferFill() > 0) {}

		//	// Loop over all buffers and see if any of them is going to be empty soon.
		//	// If so fill the one buffer with data from the source, and copy that data
		//	// to all other attached playback buffers
		//	int readTotal = 0;
		//	size_t availBuffer = -1;
		//	for(size_t i = 0; i < m_buffers.size(); ++i)
		//	{
		//		if(!m_buffers[i].enabled)
		//			continue;
		//		if(m_buffers[i].buffer->avail() < 48000 / 2)
		//		{
		//			availBuffer = i;
		//			while(m_buffers[i].buffer->avail() < 48000 / 2)
		//			{
		//				int read = m_source->readSamples(m_buffers[i].buffer);
		//				if(read <= 0)
		//					break;
		//				readTotal += read;
		//			}
		//			break;
		//		}
		//	}

		//	// Now we have 'read' bytes available in buffer nr. 'availBuffer',
		//	// copy those to the other buffers
		//	if(readTotal > 0 && availBuffer != -1)
		//	{
		//		SampleBuffer *sourceBuffer = m_buffers[availBuffer].buffer;
		//		int start = sourceBuffer->avail() - readTotal;
		//		assert(start > 0 && "Start is negative?!");
		//		for(size_t i = 0; i < m_buffers.size(); ++i)
		//		{
		//			//Do not copy to ourselves or to disabled buffers
		//			if(i == availBuffer || !m_buffers[i].enabled)
		//				continue;
		//			sourceBuffer->copyToOther(m_buffers[i].buffer, readTotal, start);
		//		}
		//	}
		}
		m_mutex.unlock();

		// We now have half a second of samples available and have done
		// so much work that we deserve a little rest
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::threadFunc()
{
	run();
	m_running = false;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::addBuffer( SampleBuffer *buffer, bool enableBuffer /*= true*/ )
{
	Lock lock(m_mutex);
	if(std::find_if(m_buffers.begin(), m_buffers.end(), [buffer](const buffer_t &b) {return b.buffer == buffer;}) == m_buffers.end())
	{
		buffer_t b = {buffer, enableBuffer};
		m_buffers.push_back(b);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::remBuffer( SampleBuffer *buffer )
{
	Lock lock(m_mutex);
	auto it = std::find_if(m_buffers.begin(), m_buffers.end(), [buffer](const buffer_t &b) {return b.buffer == buffer;});
	if(it != m_buffers.end())
		m_buffers.erase(it);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::setBufferEnabled( SampleBuffer *buffer, bool enabled )
{
	Lock lock(m_mutex);
	auto it = std::find_if(m_buffers.begin(), m_buffers.end(), [buffer](const buffer_t &b) {return b.buffer == buffer;});
	if(it != m_buffers.end())
		it->enabled = enabled;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::produce( const short *samples, int count )
{
	for(const buffer_t &buffer : m_buffers)
		if(buffer.enabled)
			buffer.buffer->produce(samples, count);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int SampleProducerThread::singleBufferFill()
{
	for(const buffer_t &buffer : m_buffers)
	{
		if(buffer.enabled && buffer.buffer->avail() < MIN_BUFFER_SAMPLES)
		{
			assert(buffer.buffer->maxSize() > MIN_BUFFER_SAMPLES && "Buffer too small");
			return m_source->readSamples(this);
		}
	}
	return 0;
}
