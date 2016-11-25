// src/samples.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "common.h"

#include "inputfile.h"
#include "samples.h"
#include "SoundInfo.h"
#include "ts3log.h"
#include "HighResClock.h"

#include <queue>
#include <vector>
#include <cassert>


#define USE_SSE2
//#define MEASURE_PERFORMANCE

#ifdef USE_SSE2
#include <emmintrin.h>
#endif


using std::vector;
using std::queue;

static_assert(sizeof(short) == 2, "Short is weird size");

#if defined(_MSC_VER)
#define ALIGNED_(x) __declspec(align(x))
#elif defined(__GNUC__)
#define ALIGNED_(x) __attribute__ ((aligned(x)))
#else
#error Unknown compiler
#endif

#define ALIGNED_STACK_ARRAY(name, size, alignment) name[size] ALIGNED_(alignment) 

#define MAX_SAMPLEBUFFER_SIZE (48000 * 5)

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
Sampler::Sampler() :
	m_sbCapture(2, MAX_SAMPLEBUFFER_SIZE),
	m_sbPlayback(2, MAX_SAMPLEBUFFER_SIZE),
	m_sampleProducerThread(),
	m_inputFile(NULL),
	m_volumeDivider(1),
	m_state(eSILENT),
	m_localPlayback(true),
	m_globalDbSetting(-1.0),
	m_soundDbSetting(0.0)
{
	assert(m_state.is_lock_free());
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

static inline __m128i muly(const __m128i &a, const __m128i &b)
{
	__m128i tmp1 = _mm_mul_epu32(a, b); /* mul 2,0*/
	__m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4)); /* mul 3,1 */
	return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, _MM_SHUFFLE (0,0,2,0)), _mm_shuffle_epi32(tmp2, _MM_SHUFFLE (0,0,2,0))); /* shuffle results to [63..0] and pack */
}

#endif

#ifdef MEASURE_PERFORMANCE
size_t g_perfMeasureCount = 0;
double g_perfMeasurement = 0.0;
#endif

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int Sampler::fetchSamples(SampleBuffer &sb, short *samples, int count, int channels, bool eraseConsumed, int ciLeft, int ciRight, bool overLeft, bool overRight )
{
#ifdef MEASURE_PERFORMANCE
	std::chrono::time_point<HighResClock> start, end;
	start = HighResClock::now();
#endif

	if (m_state == ePAUSED)
		return 0;

	SampleBuffer::Lock sbl(sb.getMutex());

	if(sb.avail() == 0)
		return 0;

	if(overLeft)
	{
		if(channels == 1)
			memset(samples, 0, count * sizeof(short));
		else
			for(int i = 0; i < count; i++)
				samples[i*channels+ciLeft] = 0;
	}

	if(overRight && channels > 1)
		for(int i = 0; i < count; i++)
			samples[i*channels+ciRight] = 0;

	const int write = std::min(count, sb.avail());
	//std::vector<short> buffer(maxSamples * 2);
	//int write = sb.consume(buffer.data(), maxSamples, eraseConsumed);

	const short* const in = sb.getBufferData();
	short* const out = samples;

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
	if(channels == 1)
	{
		__m128i ones = _mm_set1_epi16(1);
		__m128i volumeDivider = _mm_set1_epi32(m_volumeDivider);
		for(int i = 0; i < write; i += 8)
		{
			__m128i a, b;
			a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(in + i*2));
			b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(in + i*2 + 8));

			// merge channels via mad
			// mad returns the following: (x0 * y0 + x1 * y1, x2 * y2 + x3 * y3, ... )
			// If we set y to only ones we just add every second member
			// madd also returns the result as 32 bit integers
			a = _mm_madd_epi16(a, ones); // = (a0 + a1, a2 + a3, a4 + a5, a6 + a7)
			b = _mm_madd_epi16(b, ones); // = (b0 + b1, b2 + b3, b4 + b5, b6 + b7)

			// scale, multiply with volume divider and divide by 8192
			// normally we would divide by 4096 but we haven't divided our samples
			// by two in the channel merge stage.
			// The resulting operation is essentially a = (a * volumeDivider) / 4096 / 2
			a = _mm_srai_epi32(muly(a, volumeDivider), 13);
			b = _mm_srai_epi32(muly(b, volumeDivider), 13);

			//Now convert 4*32 bits from a and 4*32 bits from b to 8*16 bits output
			a = _mm_packs_epi32(a, b);

			b = _mm_loadu_si128(reinterpret_cast<__m128i*>(out + i)); //b = out[i]
			b = _mm_adds_epi16(a, b); // b = sat(a + b)
			_mm_storeu_si128(reinterpret_cast<__m128i*>(out + i), b);
		}
	}
	else
	{
		__m128i volumeDivider = _mm_set1_epi32(m_volumeDivider);
		// Align outbuf to 16 bytes
		char outbufBuf[8*sizeof(short)+16];
		short *outbuf = (short*)(outbufBuf + (16 - (size_t)outbufBuf % 16));
		assert(reinterpret_cast<size_t>(outbuf) % 16 == 0);
		for(int i = 0; i < write; i += 4)
		{
			__m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(in + i*2));
			//widen to 32 bits
			__m128i a = _mm_srai_epi32(_mm_unpacklo_epi16(v, v), 16);
			__m128i b = _mm_srai_epi32(_mm_unpackhi_epi16(v, v), 16);

			// scale, multiply with volume divider and divide by 4096
			a = _mm_srai_epi32(muly(a, volumeDivider), 12);
			b = _mm_srai_epi32(muly(b, volumeDivider), 12);

			//store into aligned memory
			_mm_store_si128(reinterpret_cast<__m128i*>(outbuf), _mm_packs_epi32(a, b));

			for(int k = 0; k < 4; ++k)
			{
				int id = (i + k) * channels;
				out[id + ciLeft] += outbuf[k*2];
				out[id + ciRight] += outbuf[k*2+1];
			}
		}
	}
#endif

	sb.consume(NULL, write, true);

#ifdef MEASURE_PERFORMANCE
	end = HighResClock::now();
	std::chrono::duration<double> elapsed = end - start;
	g_perfMeasurement += elapsed.count();
	if(++g_perfMeasureCount >= 1000)
	{
		logInfo("Avg. time in fetchSamples: %f us", g_perfMeasurement / (double)g_perfMeasureCount * 1000000.0);
		g_perfMeasureCount = 0;
		g_perfMeasurement = 0.0;
	}
#endif
	return write;
}


int Sampler::findChannelId(unsigned int channel, const unsigned int *channelSpeakerArray, int count)
{
	for(int i = 0; i < count; i++)
		if(channelSpeakerArray[i] & channel)
			return i;
	return 0;
}


int Sampler::fetchInputSamples(short *samples, int count, int channels, bool *finished)
{
	std::lock_guard<std::mutex> Lock(m_mutex);

	int written = fetchSamples(m_sbCapture, samples, count, channels, true, 0, 1, m_muteMyself, m_muteMyself);
	
	SampleBuffer::Lock sbl(m_sbCapture.getMutex());
	if(m_state == ePLAYING && m_inputFile && m_inputFile->done() && m_sbCapture.avail() == 0)
	{
		m_state = eSILENT;
		if(finished)
			*finished = true;
		emit onStopPlaying();
	}

	return written;
}


int Sampler::fetchOutputSamples(short *samples, int count, int channels, const unsigned int *channelSpeakerArray, unsigned int *channelFillMask)
{
	std::lock_guard<std::mutex> Lock(m_mutex);

	const unsigned int bitMaskLeft = SPEAKER_FRONT_LEFT | SPEAKER_HEADPHONES_LEFT;
	const unsigned int bitMaskRight = SPEAKER_FRONT_RIGHT | SPEAKER_HEADPHONES_RIGHT;
	int ciLeft = findChannelId(bitMaskLeft, channelSpeakerArray, channels);
	int ciRight = findChannelId(bitMaskRight, channelSpeakerArray, channels);
	int written = fetchSamples(m_sbPlayback, samples, count, channels, true, ciLeft, ciRight,
		(*channelFillMask & bitMaskLeft) == 0,
		(*channelFillMask & bitMaskRight) == 0);
	
	if(written > 0)
		*channelFillMask |= (bitMaskLeft | bitMaskRight);

	if(m_state == ePLAYING_PREVIEW && m_inputFile && m_inputFile->done() && m_sbPlayback.avail() == 0)
	{
		m_state = eSILENT;
		emit onStopPlaying();
	}

	return written;
}


bool Sampler::playFile(const SoundInfo &sound)
{
	return playSoundInternal(sound, false);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool Sampler::playPreview(const SoundInfo &sound)
{
	return playSoundInternal(sound, true);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::stopPlayback()
{
	if(m_inputFile)
	{
		m_state = eSILENT;
		m_sampleProducerThread.setSource(NULL);
		m_inputFile->close();
		delete m_inputFile;
		m_inputFile = NULL;

		//Clear buffers
		SampleBuffer::Lock sblc(m_sbCapture.getMutex());
		SampleBuffer::Lock sblp(m_sbPlayback.getMutex());
		m_sbCapture.consume(NULL, m_sbCapture.avail());
		m_sbPlayback.consume(NULL, m_sbPlayback.avail());

		emit onStopPlaying();
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
	m_volumeDivider = (int)(factor * 4096.0 + 0.5);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool Sampler::playSoundInternal( const SoundInfo &sound, bool preview )
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

	SampleBuffer::Lock sblc(m_sbCapture.getMutex());
	SampleBuffer::Lock sblp(m_sbPlayback.getMutex());

	//Clear buffers
	m_sbCapture.consume(NULL, m_sbCapture.avail());
	m_sbPlayback.consume(NULL, m_sbPlayback.avail());

	if(preview)
	{
		m_state = ePLAYING_PREVIEW;
		m_sampleProducerThread.setBufferEnabled(&m_sbCapture, false);
		m_sampleProducerThread.setBufferEnabled(&m_sbPlayback, true);
	}
	else
	{
		m_state = ePLAYING;
		m_sampleProducerThread.setBufferEnabled(&m_sbCapture, true);
	}

	m_sampleProducerThread.setSource(m_inputFile);

	emit onStartPlaying(preview, sound.filename);

	return true;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::pausePlayback()
{
	std::lock_guard<std::mutex> Lock(m_mutex);
	if (m_state == ePLAYING)
	{
		m_state = ePAUSED;
		emit onPausePlaying();
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::unpausePlayback()
{
	std::lock_guard<std::mutex> Lock(m_mutex);
	if (m_state == ePAUSED)
	{
		m_state = ePLAYING;
		emit onUnpausePlaying();
	}
}

