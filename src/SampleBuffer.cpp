
#include "SampleBuffer.h"


typedef std::lock_guard<std::mutex> Lock;

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SampleBuffer::SampleBuffer( int channels ) :
	m_channels(channels),
	m_cbProd(NULL),
	m_cbCons(NULL)
{

}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleBuffer::produce( const short *samples, int count )
{
	Lock lock(m_mutex);
	m_buf.insert(m_buf.end(), samples, samples + (count * m_channels));
	if(m_cbProd)
		m_cbProd->onProduceSamples(samples, count, this);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int SampleBuffer::consume( short *samples, int maxCount, bool eraseConsumed )
{
	Lock lock(m_mutex);
	int count = std::min(avail(), maxCount);
	if(samples)
		memcpy(samples, m_buf.data(), count * m_channels * sizeof(short));
	if(eraseConsumed)
		m_buf.erase(m_buf.begin(), m_buf.begin() + count * m_channels);
	if(m_cbCons)
		m_cbCons->onConsumeSamples(samples, count, this);
	return count;
}

