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
#include <math.h>

//#define MEASURE_PERFORMANCE

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
#define AMP_THRESH (SHRT_MAX / 2)



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
Sampler::Sampler() :
	m_sbCapture(2, MAX_SAMPLEBUFFER_SIZE),
	m_sbPlayback(2, MAX_SAMPLEBUFFER_SIZE),
	m_sampleProducerThread(),
	m_inputFile(NULL),
	m_peakMeterCapture(0.01f, 0.00005f, 24000),
	m_peakMeterPlayback(0.01f, 0.00005f, 24000),
	m_volumeDivider(1),
	m_globalDbSettingLocal(-1.0),
	m_globalDbSettingRemote(-1.0),
    m_soundDbSetting(0.0),
    m_state(eSILENT),
    m_localPlayback(true)
{
    /* Ensure resources are loaded */
    Q_INIT_RESOURCE(qtres);

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


#ifdef MEASURE_PERFORMANCE
size_t g_perfMeasureCount = 0;
double g_perfMeasurement = 0.0;
#endif

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int Sampler::fetchSamples(SampleBuffer &sb, PeakMeter &pm, short *samples, int count, int channels, bool eraseConsumed, int ciLeft, int ciRight, bool overLeft, bool overRight )
{
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

	const short* const in = sb.getBufferData();
	short* const out = samples;

#ifdef MEASURE_PERFORMANCE
	std::chrono::time_point<HighResClock> start, end;
	start = HighResClock::now();
#endif

	if(channels == 1)
	{
		for (int i = 0; i < write; i++)
		{
			float sample = out[i] + m_volumeFactor * (float(in[i * 2]) + float(in[i * 2 + 1])) * 0.5f;
			pm.process(sample);
			out[i] = pm.limit(sample, AMP_THRESH);
		}
	}
	else
	{
		for(int i = 0; i < write; i++)
		{
			float sample0 = out[i * channels + ciLeft] + m_volumeFactor * float(in[i * 2]);
			float sample1 = out[i * channels + ciRight] + m_volumeFactor * float(in[i * 2 + 1]);
			pm.process(fabs(sample0) > fabs(sample1) ? sample0 : sample1);
			out[i * channels + ciLeft] = pm.limit(sample0, AMP_THRESH);
			out[i * channels + ciRight] = pm.limit(sample1, AMP_THRESH);
		}
	}

	sb.consume(NULL, write, true);

#ifdef MEASURE_PERFORMANCE
	end = HighResClock::now();
	std::chrono::duration<double> elapsed = end - start;
	g_perfMeasurement += elapsed.count();
	if(++g_perfMeasureCount >= 1000)
	{
		logInfo("Avg. time in fetchSamples: %f us, volume: %f, limiter: %f", g_perfMeasurement / (double)g_perfMeasureCount * 1000000.0,
			m_volumeFactor, std::min(AMP_THRESH / m_peakMeterPlayback.getOutput(), 1.0f));
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

	setVolumeDb(m_globalDbSettingRemote + m_soundDbSetting);
	int written = fetchSamples(m_sbCapture, m_peakMeterCapture, samples, count, channels, true, 0, 1, m_muteMyself, m_muteMyself);
	
    if(m_state == ePLAYING && m_inputFile && m_inputFile->done())
	{
        SampleBuffer::Lock sbl(m_sbCapture.getMutex());
        if (m_sbCapture.avail() == 0)
        {
            m_state = eSILENT;
            if(finished)
                *finished = true;
            emit onStopPlaying();
        }
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
	setVolumeDb(m_globalDbSettingLocal + m_soundDbSetting);
	int written = fetchSamples(m_sbPlayback, m_peakMeterPlayback, samples, count, channels, true, ciLeft, ciRight,
		(*channelFillMask & bitMaskLeft) == 0,
		(*channelFillMask & bitMaskRight) == 0);
	
	if(written > 0)
		*channelFillMask |= (bitMaskLeft | bitMaskRight);

    if(m_state == ePLAYING_PREVIEW && m_inputFile && m_inputFile->done())
	{
        SampleBuffer::Lock sbl(m_sbPlayback.getMutex());
        if (m_sbPlayback.avail() == 0)
        {
            m_state = eSILENT;
            emit onStopPlaying();
        }
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
	std::lock_guard<std::mutex> Lock(m_mutex);
	stopSoundInternal();
}

#define VOLUMESCALER_EXPONENT 1.0
#define VOLUMESCALER_DB_MIN -28.0
//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::setVolumeRemote( int vol )
{
	double v = (double)vol / 100.0;
	double db = pow(1.0 - v, VOLUMESCALER_EXPONENT) * VOLUMESCALER_DB_MIN;
	m_globalDbSettingRemote = db;
	setVolumeDb(m_globalDbSettingRemote + m_soundDbSetting);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::setVolumeLocal( int vol )
{
	double v = (double)vol / 100.0;
	double db = pow(1.0 - v, VOLUMESCALER_EXPONENT) * VOLUMESCALER_DB_MIN;
	m_globalDbSettingLocal = db;
	setVolumeDb(m_globalDbSettingLocal + m_soundDbSetting);
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
	m_volumeFactor = (float)factor;
	m_volumeDivider = (int)(factor * (1 << volumeScaleExp) + 0.5);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void Sampler::stopSoundInternal()
{
	if (m_inputFile)
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


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool Sampler::playSoundInternal( const SoundInfo &sound, bool preview )
{
	std::lock_guard<std::mutex> Lock(m_mutex);

	stopSoundInternal();

	m_inputFile = CreateInputFileFFmpeg();

	if(m_inputFile->open(sound.filename.toUtf8(), sound.getStartTime(), sound.getPlayTime()) != 0)
	{
		delete m_inputFile;
		m_inputFile = NULL;
		return false;
	}

	m_soundDbSetting = (double)sound.volume;
	setVolumeDb(m_globalDbSettingLocal + m_soundDbSetting);

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

