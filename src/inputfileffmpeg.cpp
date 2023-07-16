// src/inputfileffmpeg.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#include "common.h"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <vector>
#include <algorithm>

#include "ts3log.h"
#include "inputfile.h"
#include "SampleBuffer.h"
#include "SampleSource.h"
#include "main.h"
#include <mutex>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}


#define OUTPUT_BUFFER_COUNT 32768
#define OUTPUT_FORMAT AV_SAMPLE_FMT_S16
//#define OUTPUT_CHANNELS 2
//#define OUTPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO
//#define OUTPUT_SAMPLERATE 48000

//#ifdef _WIN32
//void LoadWildcardDll(const TCHAR *path, const TCHAR *name)
//{
//	tstring str = path;
//	str += TEXT("\\");
//	str += name;
//	WIN32_FIND_DATA data;
//	if(FindFirstFile(str.c_str(), &data) != INVALID_HANDLE_VALUE)
//	{
//		tstring file = path;
//		file += TEXT("\\");
//		file += data.cFileName;
//		LoadLibrary(file.c_str());
//	}
//}
//
//BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
//{
//	const TCHAR *path = TEXT("plugins\\rp_soundboard");
//	LoadWildcardDll(path, TEXT("avcodec*.dll"));
//	LoadWildcardDll(path, TEXT("avformat*.dll"));
//	LoadWildcardDll(path, TEXT("avutil*.dll"));
//	LoadWildcardDll(path, TEXT("swresample*.dll"));
//}
//#endif

void InitFFmpegLibrary()
{
}


int LogFFmpegError(int code, const char *msg = NULL)
{
	if(code < 0)
	{
		char buf[256];
		if(av_strerror(code, buf, sizeof buf) < 0)
			strcpy(buf, "Unknown Error");
		if(msg)
			logError("%s. FFmpeg Error: %s", msg, buf);
		else
			logError("FFmpeg Error: %s", buf);
	}

	return code;
}


class InputFileFFmpeg : public InputFile
{
public:
	InputFileFFmpeg(const InputFileOptions &options);
	~InputFileFFmpeg();
	int open(const char *filename, double startPosSeconds = 0.0, double playTimeSeconds = -1.0) override;
	int close() override;

	int readSamples(SampleProducer *sampleBuffer) override;
	bool done() const override;
	int seek(double seconds) override;
	int64_t outputSamplesEstimation() const override;

private:
	int _close();
	void reset();
	int getAudioStreamNum() const;
	int64_t handleDecoded(AVFrame *frame, SampleProducer *sb);
	int64_t getTargetSamples(int64_t sourceSamples, int64_t sourceSampleRate, int64_t targetSampleRate);

	typedef std::lock_guard<std::mutex> Lock;
private:
	const InputFileOptions m_inputFileOptions;
	const int m_outputChannels;
	const int m_outputSamplerate;
	const int64_t m_outputChannelLayout;

	AVFormatContext *m_fmtCtx;
	AVCodecContext *m_codecCtx;
	SwrContext *m_swrCtx;
	int m_streamIndex;
	uint8_t *m_outBuf;
	bool m_opened;
	bool m_done;
	std::mutex m_mutex;
	int64_t m_decodedSamples;
	int64_t m_convertedSamples;
	int64_t m_decodedSamplesTargetSR;
	int64_t m_maxConvertedSamples;
	int64_t m_nextSeekTimestamp;
	int64_t m_skipSamples;
};

//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
inline int64_t getChannelLayoutFromOptions(const InputFileOptions &options)
{
	switch(options.outputChannelLayout)
	{
	case InputFileOptions::MONO:
		return AV_CH_LAYOUT_MONO;
	case InputFileOptions::STEREO:
		return AV_CH_LAYOUT_STEREO;
	default:
		return AV_CH_LAYOUT_STEREO;
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
InputFileFFmpeg::InputFileFFmpeg(const InputFileOptions &options) :
	m_inputFileOptions(options),
	m_outputChannels(options.getNumChannels()),
	m_outputSamplerate(options.outputSampleRate),
	m_outputChannelLayout(getChannelLayoutFromOptions(options))
{
	reset();
	av_samples_alloc(&m_outBuf, NULL, m_outputChannels, OUTPUT_BUFFER_COUNT, OUTPUT_FORMAT, 0);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void InputFileFFmpeg::reset()
{
	m_fmtCtx = NULL;
	m_codecCtx = NULL;
	m_swrCtx = NULL;
	m_streamIndex = 0;
	m_opened = false;
	m_done = false;
	m_decodedSamples = 0;
	m_convertedSamples = 0;
	m_decodedSamplesTargetSR = 0;
	m_maxConvertedSamples = 0;
	m_nextSeekTimestamp = 0;
	m_skipSamples = 0;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
InputFileFFmpeg::~InputFileFFmpeg()
{
	_close();
	av_freep(&m_outBuf);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int InputFileFFmpeg::open(const char *filename, double startPosSeconds /*= 0.0*/, double playTimeSeconds /*= -1.0*/)
{
	Lock lock(m_mutex);

	if(m_opened)
	{
		_close();
		reset();
	}


	if(LogFFmpegError(avformat_open_input(&m_fmtCtx, filename, NULL, NULL), "Cannot open file") != 0)
	{
		return -1;
	}

	if(LogFFmpegError(avformat_find_stream_info(m_fmtCtx, NULL), "Cannot find stream info") < 0)
	{	
		_close();
		return -1;
	}

	m_streamIndex = getAudioStreamNum();
	if(m_streamIndex < 0)
	{
		logError("Cannot find a suitable stream");
		_close();
		return -1;
	}

	
	m_codecCtx = m_fmtCtx->streams[m_streamIndex]->codec;
	AVCodec *codec = avcodec_find_decoder(m_codecCtx->codec_id);
	if(!codec)
	{
		logError("Unsupported codec");
		_close();
		return -1;
	}

	m_codecCtx->codec = codec;
	m_codecCtx->channel_layout = av_get_default_channel_layout(m_codecCtx->channels);
	if(LogFFmpegError(avcodec_open2(m_codecCtx, m_codecCtx->codec, NULL), "Cannot open codec") < 0)
	{
		_close();
		return -1; //Cannot open codec
	}


	//Open Resample context
	m_swrCtx = swr_alloc_set_opts(NULL,
		m_outputChannelLayout,		//Output layout (stereo)
		OUTPUT_FORMAT,				//Output format (signed 16bit int)
		m_outputSamplerate,			//Output Sample Rate
		m_codecCtx->channel_layout, //Input layout
		m_codecCtx->sample_fmt,		//Input format
		m_codecCtx->sample_rate,	//Input Sample Rate
		0, NULL);

	if(!m_swrCtx)
	{
		logError("Failed to allocate resample context");
		_close();
		return -1;
	}

	if(LogFFmpegError(swr_init(m_swrCtx), "Cannot initialize resample context") < 0)
	{
		_close();
		return -1;
	}

	logInfo("Opened file: %s; Codec: %s, Channels: %i, Rate: %i, Format: %s, Timebase: %i/%i, Sample-Estimation: %ll",
		filename, codec->long_name, m_codecCtx->channels, m_codecCtx->sample_rate,
		av_get_sample_fmt_name(m_codecCtx->sample_fmt), m_codecCtx->time_base.num, m_codecCtx->time_base.den,
		outputSamplesEstimation());

	m_opened = true;

	if(startPosSeconds > 0.0)
		seek(startPosSeconds);

	if(playTimeSeconds > 0.0)
		m_maxConvertedSamples = uint64_t(playTimeSeconds * (double)m_outputSamplerate + 0.5);

	return 0;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int InputFileFFmpeg::seek( double seconds )
{
	AVRational time_base = m_fmtCtx->streams[m_streamIndex]->time_base;
	int64_t ts = (int64_t)(seconds / time_base.num * time_base.den);
	if(LogFFmpegError(avformat_seek_file(m_fmtCtx, m_streamIndex, INT64_MIN, ts, ts, 0), "Seeking failed") < 0)
		return -1;
	avcodec_flush_buffers(m_codecCtx);
	m_nextSeekTimestamp = ts;
	return 0;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int InputFileFFmpeg::close()
{
	Lock lock(m_mutex);
	return _close();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int64_t InputFileFFmpeg::handleDecoded(AVFrame *frame, SampleProducer *sb)
{
	if (m_nextSeekTimestamp > 0) // Need to skip some samples?
	{
		int64_t curTs = av_frame_get_best_effort_timestamp(frame);
		int64_t tsToSkip = m_nextSeekTimestamp - curTs;
		if (tsToSkip > 0)
		{
			double timeBase = av_q2d(m_fmtCtx->streams[m_streamIndex]->time_base);
			double secondsToSkip = tsToSkip * timeBase;
			m_skipSamples = (int)(secondsToSkip * m_outputSamplerate);
			logInfo("Current timestep is %f but desired is %f, skipping %lli samples",
				curTs * timeBase, m_nextSeekTimestamp * timeBase, m_skipSamples);
		}
		m_nextSeekTimestamp = 0;
	}

	int64_t res = swr_convert(m_swrCtx, &m_outBuf, OUTPUT_BUFFER_COUNT,
		frame ? (const uint8_t **)frame->extended_data : NULL,
		frame ? frame->nb_samples : 0);
	if(res <= 0)
		return res;
	int64_t outSamples = std::max(int64_t(0), res - m_skipSamples);
	int64_t skippedSamples = res - outSamples;
	if(m_maxConvertedSamples > 0 && outSamples > (m_maxConvertedSamples - m_convertedSamples))
	{
		outSamples = m_maxConvertedSamples - m_convertedSamples;
		m_done = true;
	}
	if(outSamples > 0)
		sb->produce(((short*)m_outBuf) + (skippedSamples * m_outputChannels), outSamples);

	m_skipSamples -= skippedSamples;
	return outSamples;
}



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int InputFileFFmpeg::readSamples(SampleProducer *sampleBuffer)
{
	Lock lock(m_mutex);

	if(!m_opened)
		return -1;

	AVFrame *frame = av_frame_alloc();
	AVPacket packet;
	av_init_packet(&packet);
	int written = 0; //samples read

	int properFrames = 0;
	while(properFrames == 0 && av_read_frame(m_fmtCtx, &packet) == 0)
	{
		if(packet.stream_index == m_streamIndex)
		{
			AVPacket decodePacket = packet;
			while(decodePacket.size > 0)
			{
				// Try to decode the packet into a frame
				// Some frames rely on multiple packets, so we have to make sure the frame is finished before
				// we can use it
				int gotFrame = 0;
				int consumed = avcodec_decode_audio4(m_codecCtx, frame, &gotFrame, &decodePacket);
				if(consumed >= 0)
				{
					decodePacket.size -= consumed;
					decodePacket.data += consumed;
					if (gotFrame)
					{
						m_decodedSamples += frame->nb_samples;
						m_decodedSamplesTargetSR += getTargetSamples(frame->nb_samples, m_outputSamplerate, m_codecCtx->sample_rate);

						//Resample
						int res = handleDecoded(frame, sampleBuffer);
						if (LogFFmpegError(res, "Unable to resample") < 0)
						{
							av_free_packet(&packet);
							av_frame_free(&frame);
							return -1;
						}

						m_convertedSamples += res;
						written += res;
						if (res > 0)
							properFrames++;
					}
				}
				else
				{
					decodePacket.size = 0;
					decodePacket.data = NULL;
				}
			}
		}

		// You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
		av_free_packet(&packet);
	}

	if(properFrames == 0)
	{
		if(m_codecCtx->codec->capabilities & CODEC_CAP_DELAY && m_convertedSamples < m_decodedSamplesTargetSR)
		{
			// Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
			// is set, there can be buffered up frames that need to be flushed, so we'll do that

			//av_init_packet(&packet);
			packet.data = NULL;
			packet.size = 0;

			// Decode all the remaining frames in the buffer, until the end is reached
			int gotFrame = 0;
			while (avcodec_decode_audio4(m_codecCtx, frame, &gotFrame, &packet) >= 0 && gotFrame)
			{
				// We now have a fully decoded audio frame
				properFrames++;
				int res = handleDecoded(frame, sampleBuffer);
				if(LogFFmpegError(res, "Unable to resample") < 0)
				{
					av_free_packet(&packet);
					av_frame_free(&frame);
					return -1;
				}
				written += res;
			}
		}

		//Flush resampling
		int res;
		while(m_convertedSamples < m_decodedSamplesTargetSR && (res = handleDecoded(NULL, sampleBuffer)) > 0)
		{
			written += res;
		}
		m_done = true;
	}

	av_free_packet(&packet);
	av_frame_free(&frame);

	return written;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool InputFileFFmpeg::done() const 
{
	return m_done;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int InputFileFFmpeg::getAudioStreamNum() const
{
	return av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int64_t InputFileFFmpeg::getTargetSamples(int64_t sourceSamples, int64_t sourceSampleRate, int64_t targetSampleRate)
{
	return av_rescale_rnd(sourceSamples, sourceSampleRate, targetSampleRate, AV_ROUND_DOWN);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int InputFileFFmpeg::_close()
{
	if(m_swrCtx)
		swr_free(&m_swrCtx);

	if(m_codecCtx)
		avcodec_close(m_codecCtx);

	if(m_fmtCtx)
	{
		avformat_close_input(&m_fmtCtx);
		m_codecCtx = NULL;
	}

	m_opened = false;

	return 0;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int64_t InputFileFFmpeg::outputSamplesEstimation() const
{
	AVStream *stream = m_fmtCtx->streams[m_streamIndex];
	if (stream->duration > 0)
		return stream->duration * (int64_t)stream->time_base.num *
			(int64_t)m_outputSamplerate / (int64_t)stream->time_base.den;
	else
		return m_fmtCtx->duration * m_outputSamplerate / AV_TIME_BASE;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
InputFile *CreateInputFileFFmpeg(InputFileOptions options /*= InputFileOptions()*/)
{
	return new InputFileFFmpeg(options);
}
