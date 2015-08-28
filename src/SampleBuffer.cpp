// src/SampleBuffer.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "SampleBuffer.h"


typedef std::lock_guard<std::mutex> Lock;

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SampleBuffer::SampleBuffer( int channels, size_t maxSize /*= 0*/ ) :
	m_channels(channels),
	m_maxSize(maxSize),
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
	if(m_maxSize == 0 || avail() < m_maxSize)
	{
		m_buf.insert(m_buf.end(), samples, samples + (count * m_channels));
		if(m_cbProd)
			m_cbProd->onProduceSamples(samples, count, this);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int SampleBuffer::consume( short *samples, int maxCount, bool eraseConsumed )
{
	Lock lock(m_mutex);
	int count = std::min(avail(), maxCount);
	size_t shorts = count * m_channels;
	if(samples)
		memcpy(samples, m_buf.data(), shorts * 2);
	if(eraseConsumed)
		m_buf.erase(m_buf.begin(), m_buf.begin() + count * m_channels);
	if(m_cbCons)
		m_cbCons->onConsumeSamples(samples, count, this);
	return count;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleBuffer::copyToOther( SampleBuffer *other, int count, int start /*= 0*/ ) const
{
	Lock lock(m_mutex);
	if(m_channels != other->m_channels)
		throw std::logic_error("Copying to buffers with different channel count is not supported.");
	start = std::min(start, avail());
	count = std::min(count, avail() - start);
	if(count > 0)
		other->produce(&m_buf[start * m_channels], count);
}

