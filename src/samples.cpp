
#include "common.h"

#include "inputfile.h"
#include "samples.h"

#include <queue>
#include <vector>

using std::vector;
using std::queue;

static_assert(sizeof(short) == 2, "Short is weird size");


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
Sampler::Sampler() :
	m_sbCapture(2),
	m_sbPlayback(2),
	m_sampleProducerThread(&m_sbCapture),
	m_onBufferProduceCB(*this),
	m_inputFile(NULL),
	m_volumeDivider(1),
	m_playing(false),
	m_localPlayback(true)
{
	m_sbCapture.setOnProduce(&m_onBufferProduceCB);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
Sampler::~Sampler()
{
	
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::init()
{
	m_sampleProducerThread.start();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::shutdown()
{
	std::lock_guard<std::mutex> Lock(m_mutex);

	if(m_inputFile)
	{
		m_inputFile->close();
		delete m_inputFile;
		m_inputFile = NULL;
	}

	m_sampleProducerThread.stop();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int Sampler::fetchSamples(SampleBuffer &sb, short *samples, int count, int channels, bool eraseConsumed, int ciLeft, int ciRight, bool overLeft, bool overRight )
{
	if(sb.avail() == 0)
		return 0;

	if(overLeft)
		for(int i = 0; i < count; i++)
			samples[i*channels+ciLeft] = 0;

	if(overRight && channels > 1)
		for(int i = 0; i < count; i++)
			samples[i*channels+ciRight] = 0;

	int maxSamples = std::min(count, sb.avail());
	std::vector<short> buffer(maxSamples * 2);
	int write = sb.consume(buffer.data(), maxSamples, eraseConsumed);

	short *in = buffer.data();
	short *out = samples;

	if(channels == 1)
	{
		for(int i = 0; i < write; i++)
			out[i] += scale(in[i*2] / 2 + in[i*2+1] / 2);
	}
	else
	{
		for(int i = 0; i < write; i++)
		{
			out[i * channels + ciLeft] += scale(in[i*2]);
			out[i * channels + ciRight] += scale(in[i*2+1]);
		}
	}

	return write;
}


int Sampler::findChannelId(int channel, const unsigned int *channelSpeakerArray, int count)
{
	for(int i = 0; i < count; i++)
		if(channelSpeakerArray[i] == channel)
			return i;
	return 0;
}


int Sampler::fetchInputSamples(short *samples, int count, int channels, bool *finished)
{
	std::lock_guard<std::mutex> Lock(m_mutex);

	int written = fetchSamples(m_sbCapture, samples, count, channels, true, 0, 1, m_muteMyself, m_muteMyself);
	if(m_playing && m_inputFile->done() && m_sbCapture.avail() == 0)
	{
		m_playing = false;
		if(finished)
			*finished = true;
	}

	return written;
}


int Sampler::fetchOutputSamples(short *samples, int count, int channels, const unsigned int *channelSpeakerArray, unsigned int *channelFillMask)
{
	std::lock_guard<std::mutex> Lock(m_mutex);

	int ciLeft = findChannelId(SPEAKER_FRONT_LEFT, channelSpeakerArray, channels);
	int ciRight = findChannelId(SPEAKER_FRONT_RIGHT, channelSpeakerArray, channels);
	int written = fetchSamples(m_sbPlayback, samples, count, channels, true, ciLeft, ciRight,
		(*channelFillMask & SPEAKER_FRONT_LEFT) == 0,
		(*channelFillMask & SPEAKER_FRONT_RIGHT) == 0);
	
	if(written > 0)
		*channelFillMask |= (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT);
	return written;
}


bool Sampler::playFile( const char *filename )
{
	std::lock_guard<std::mutex> Lock(m_mutex);

	stopPlayback();

	m_inputFile = CreateInputFileFFmpeg();
	if(m_inputFile->open(filename) != 0)
	{
		delete m_inputFile;
		m_inputFile = NULL;
		return false;
	}

	//Clear buffers
	m_sbCapture.consume(NULL, m_sbCapture.avail());
	m_sbPlayback.consume(NULL, m_sbPlayback.avail());

	m_playing = true;
	m_sampleProducerThread.setSource(m_inputFile);

	return true;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::stopPlayback()
{
	if(m_inputFile)
	{
		m_playing = false;
		m_sampleProducerThread.setSource(NULL);
		m_inputFile->close();
		delete m_inputFile;
		m_inputFile = NULL;
	}
}

#define VOLUMESCALER_EXPONENT 1.0
#define VOLUMESCALER_DB_MIN -28.0
//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::setVolume( int vol )
{
	double v = (double)vol / 100.0;
	double db = pow(1.0 - v, VOLUMESCALER_EXPONENT) * VOLUMESCALER_DB_MIN;
	double factor = pow(10.0, db/10.0);
	m_volumeDivider = (int)(std::min(std::max(65536.0 * (1.0 / factor), 1.0), (double)INT_MAX));
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::setLocalPlayback( bool enabled )
{
	m_localPlayback = enabled;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::setMuteMyself(bool enabled)
{
	m_muteMyself = enabled;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::OnBufferProduceCB::onProduceSamples( const short *samples, int count, SampleBuffer *caller )
{
	if(parent.m_localPlayback)
		parent.m_sbPlayback.produce(samples, count);
}
