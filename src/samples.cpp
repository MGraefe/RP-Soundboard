
#include "common.h"

#include "inputfile.h"
#include "samples.h"
#include "SoundInfo.h"

#include <queue>
#include <vector>

#ifdef USE_SSE2
#include <emmintrin.h>
#endif


using std::vector;
using std::queue;

static_assert(sizeof(short) == 2, "Short is weird size");


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
Sampler::Sampler() :
	m_sbCapture(2),
	m_sbPlayback(2),
	m_sampleProducerThread(),
	m_inputFile(NULL),
	m_volumeDivider(1),
	m_state(SILENT),
	m_localPlayback(true),
	m_globalDbSetting(-1.0),
	m_soundDbSetting(0.0)
{

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
	m_sampleProducerThread.addBuffer(&m_sbCapture);
	m_sampleProducerThread.addBuffer(&m_sbPlayback, m_localPlayback);
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

#ifdef USE_SSE2
inline void loadStridedChannels(short *v, __m128i &l, __m128i &r)
{
	__m128i a, b;
	a = _mm_loadu_si128(reinterpret_cast<__m128i*>(v));
	b = _mm_loadu_si128(reinterpret_cast<__m128i*>(v + 8));
	l = _mm_unpacklo_epi16(a, b);
	r = _mm_unpackhi_epi16(a, b);
	a = _mm_unpacklo_epi16(l, r);
	b = _mm_unpackhi_epi16(l, r);
	l = _mm_unpacklo_epi16(a, b);
	r = _mm_unpackhi_epi16(a, b);
}


inline void scaleSSE(__m128i &v, int factor)
{
	//widen values to 32 bit
	__m128i v0 = _mm_srai_epi32(_mm_unpacklo_epi16(v, v), 16);
	__m128i v1 = _mm_srai_epi32(_mm_unpackhi_epi16(v, v), 16);

	//TODO: Finish
	
}
#endif

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

#ifndef USE_SSE2
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
#else
	for(int i = 0; i < write; i += 8)
	{
		//Load channel data to in_l, in_r
		__m128i in_l, in_r;
		loadStridedChannels(in + i*2, in_l, in_r);

		if(channels == 1)
		{
			// av = (in_l + in_r) / 2
			__m128i a = _mm_add_epi16(a, _mm_set1_epi16(32768));
			__m128i b = _mm_add_epi16(b, _mm_set1_epi16(32768));
			__m128i av = _mm_avg_epu16(a, b);

			__m128i outv = _mm_loadu_si128(reinterpret_cast<__m128i*>(out + i));
			outv = _mm_add_epi16(outv, av);
		}
	}
	//TODO: finish
#endif

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
	if(m_state == PLAYING && m_inputFile->done() && m_sbCapture.avail() == 0)
	{
		m_state = SILENT;
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


bool Sampler::playFile(const SoundInfo &sound)
{
	std::lock_guard<std::mutex> Lock(m_mutex);

	stopPlayback();

	m_inputFile = CreateInputFileFFmpeg();

	if(m_inputFile->open(sound.filename.toUtf8(), sound.getStartTime(), sound.getPlayTime()) != 0)
	{
		delete m_inputFile;
		m_inputFile = NULL;
		return false;
	}

	m_soundDbSetting = (double)sound.volume;
	setVolumeDb(m_globalDbSetting + m_soundDbSetting);

	//Clear buffers
	m_sbCapture.consume(NULL, m_sbCapture.avail());
	m_sbPlayback.consume(NULL, m_sbPlayback.avail());

	m_state = PLAYING;
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
		m_state = SILENT;
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
	m_globalDbSetting = db;
	setVolumeDb(m_globalDbSetting + m_soundDbSetting);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::setLocalPlayback( bool enabled )
{
	m_localPlayback = enabled;
	m_sampleProducerThread.setBufferEnabled(&m_sbPlayback, enabled);
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
void Sampler::setVolumeDb( double decibel )
{
	double factor = pow(10.0, decibel/10.0);
	//m_volumeDivider = (int)(std::min(std::max(65536.0 * (1.0 / factor), 1.0), (double)INT_MAX));
	m_volumeDivider = (int)(factor * 4096.0 + 0.5);
}

