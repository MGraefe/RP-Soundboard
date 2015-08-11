
#include <thread>
#include <algorithm>

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
			// Loop over all buffers and see if any of them is going to be empty soon.
			// If so fill the one buffer with data from the source, and copy that data
			// to all other attached playback buffers
			int readTotal = -1;
			size_t availBuffer = -1;
			for(size_t i = 0; i < m_buffers.size(); ++i)
			{
				if(!m_buffers[i].enabled)
					continue;
				while(m_buffers[i].buffer->avail() < 48000 / 2)
				{
					availBuffer = i;
					int read = m_source->readSamples(m_buffers[i].buffer);
					if(read <= 0)
						break;
					readTotal += read;
				}
				if (availBuffer != -1)
					break;
			}

			// Now we have 'read' bytes available in buffer nr. 'availBuffer',
			// copy those to the other buffers
			if(readTotal > 0 && availBuffer != -1)
			{
				SampleBuffer *sourceBuffer = m_buffers[availBuffer].buffer;
				int start = sourceBuffer->avail() - readTotal;
				for(size_t i = 0; i < m_buffers.size(); ++i)
				{
					//Do not copy to ourselves or to disabled buffers
					if(i == availBuffer || !m_buffers[i].enabled)
						continue;
					sourceBuffer->copyToOther(m_buffers[i].buffer, readTotal, start);
				}
			}
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
	auto it = std::find_if(m_buffers.begin(), m_buffers.end(), [buffer](const buffer_t &b) {return b.buffer == buffer;});
	if(it != m_buffers.end())
		m_buffers.erase(it);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::setBufferEnabled( SampleBuffer *buffer, bool enabled )
{
	auto it = std::find_if(m_buffers.begin(), m_buffers.end(), [buffer](const buffer_t &b) {return b.buffer == buffer;});
	if(it != m_buffers.end())
		it->enabled = enabled;
}
