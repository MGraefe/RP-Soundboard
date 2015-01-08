
#include <thread>

#include "SampleBuffer.h"
#include "SampleSource.h"
#include "SampleProducerThread.h"



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SampleProducerThread::SampleProducerThread( SampleBuffer *sampleBuffer ) :
	m_source(NULL),
	m_buffer(sampleBuffer),
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
	m_source = source;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleProducerThread::run()
{
	while(!m_stop)
	{
		if(m_source)
		{
			while(m_buffer->avail() < 48000 / 2)
			{
				int read = m_source->readSamples(m_buffer);
				if(read <= 0)
					break;
			}
		}

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
