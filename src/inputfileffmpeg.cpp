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
#include <atomic>

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


int checkFFmpegErr(int code, const char *msg = NULL)
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
	bool openInternal(const char *filename, double startPosSeconds, double playTimeSeconds);
	int closeNoLock();
	void reset();
	int getAudioStreamNum() const;
	int handleDecoded(AVFrame *frame, SampleProducer *sb);
    int receiveSamples(SampleProducer *sampleBuffer, int& producedSamples);
	int seekNoLock(double seconds);

    typedef std::lock_guard<std::mutex> Lock;
private:
	const InputFileOptions m_inputFileOptions;
	const int m_outputChannels;
	const int m_outputSamplerate;
	AVChannelLayout m_outputChannelLayout;

	AVFormatContext *m_fmtCtx;
	AVCodecContext *m_codecCtx;
	AVFrame *m_frame = NULL;
	AVPacket *m_packet = NULL;
	SwrContext *m_swrCtx;
	int m_streamIndex;
	uint8_t *m_outBuf;
	bool m_opened;
	std::atomic<bool> m_done;
	std::mutex m_mutex;
	int64_t m_decodedSamples;
	int64_t m_convertedSamples;
	int64_t m_maxConvertedSamples;
	int64_t m_nextSeekTimestamp;
	int64_t m_skipSamples;
};



InputFileFFmpeg::InputFileFFmpeg(const InputFileOptions &options) :
	m_inputFileOptions(options),
	m_outputChannels(options.getNumChannels()),
	m_outputSamplerate(options.outputSampleRate)
{
	av_channel_layout_from_mask(&m_outputChannelLayout, 
		options.outputChannelLayout == InputFileOptions::MONO
		? AV_CH_LAYOUT_MONO
		: AV_CH_LAYOUT_STEREO
	);

	reset();
	av_samples_alloc(&m_outBuf, NULL, m_outputChannels, OUTPUT_BUFFER_COUNT, OUTPUT_FORMAT, 0);
}



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
	m_maxConvertedSamples = 0;
	m_nextSeekTimestamp = 0;
	m_skipSamples = 0;
}



InputFileFFmpeg::~InputFileFFmpeg()
{
	closeNoLock();
	av_freep(&m_outBuf);

	av_channel_layout_uninit(&m_outputChannelLayout);
}


bool InputFileFFmpeg::openInternal(const char *filename, double startPosSeconds, double playTimeSeconds)
{	
	if(checkFFmpegErr(avformat_open_input(&m_fmtCtx, filename, NULL, NULL), "Cannot open file") != 0)
		return false;

	if(checkFFmpegErr(avformat_find_stream_info(m_fmtCtx, NULL), "Cannot find stream info") < 0)
		return false;

	m_streamIndex = getAudioStreamNum();
	if(m_streamIndex < 0)
	{
		logError("Cannot find a suitable stream");
		return false;
	}

	AVCodecParameters *codecParams = m_fmtCtx->streams[m_streamIndex]->codecpar;

	// 2. Find the appropriate decoder
	const AVCodec *decoder = avcodec_find_decoder(codecParams->codec_id);
	if (!decoder)
	{
		logError("Cannot find suitable decoder");
		return false;
	}

	// 3. Allocate a new codec context
	m_codecCtx = avcodec_alloc_context3(decoder);
	if (!m_codecCtx)
	{
		logError("Unsupported codec");
		return false;
	}

	if (checkFFmpegErr(avcodec_parameters_to_context(m_codecCtx, codecParams), "Failed to copy codec parameters") < 0)
		return false;

	// Decoder needs to know the timebase to calculate correct timestamps during decoding,
	// but it's not always set by the demuxer, so set it manually from the stream info
	m_codecCtx->pkt_timebase = m_fmtCtx->streams[m_streamIndex]->time_base;

	if(checkFFmpegErr(avcodec_open2(m_codecCtx, decoder, NULL), "Cannot open codec") < 0)
		return false; //Cannot open codec

	//Open Resample context
	int result = swr_alloc_set_opts2(&m_swrCtx,
		&m_outputChannelLayout,		//Output layout (stereo)
		OUTPUT_FORMAT,				//Output format (signed 16bit int)
		m_outputSamplerate,			//Output Sample Rate
		&m_codecCtx->ch_layout, 	//Input layout
		m_codecCtx->sample_fmt,		//Input format
		m_codecCtx->sample_rate,	//Input Sample Rate
		0, NULL);

	if (result < 0)
	{
		logError("Failed to set resample options");
		return false;
	}

	if(!m_swrCtx)
	{
		logError("Failed to allocate resample context");
		return false;
	}

	if(checkFFmpegErr(swr_init(m_swrCtx), "Cannot initialize resample context") < 0)
		return false;

	logInfo("Opened file: %s; Codec: %s, Channels: %i, Rate: %i, Format: %s, Timebase: %i/%i, Sample-Estimation: %lld",
		filename, m_codecCtx->codec->long_name, m_codecCtx->ch_layout.nb_channels, m_codecCtx->sample_rate,
		av_get_sample_fmt_name(m_codecCtx->sample_fmt), m_codecCtx->time_base.num, m_codecCtx->time_base.den,
		outputSamplesEstimation());

	m_frame = av_frame_alloc();
	m_packet = av_packet_alloc();
	if (!m_frame || !m_packet)
	{
		logError("Failed to allocate frame or packet");
		return false;
	}

	m_opened = true;

	if(startPosSeconds > 0.0)
		seekNoLock(startPosSeconds);

	if(playTimeSeconds > 0.0)
		m_maxConvertedSamples = uint64_t(playTimeSeconds * (double)m_outputSamplerate + 0.5);
	
	return true;
}



int InputFileFFmpeg::open(const char *filename, double startPosSeconds /*= 0.0*/, double playTimeSeconds /*= -1.0*/)
{
	Lock lock(m_mutex);

	if(m_opened)
	{
		closeNoLock();
		reset();
	}

	if (!openInternal(filename, startPosSeconds, playTimeSeconds))
	{
		closeNoLock();
		return -1;
	}

	return 0;
}



int InputFileFFmpeg::seekNoLock( double seconds )
{
	AVRational time_base = m_fmtCtx->streams[m_streamIndex]->time_base;
	int64_t ts = (int64_t)(seconds / av_q2d(time_base));
	if(checkFFmpegErr(avformat_seek_file(m_fmtCtx, m_streamIndex, INT64_MIN, ts, ts, 0), "Seeking failed") < 0)
		return -1;
	avcodec_flush_buffers(m_codecCtx);
	m_nextSeekTimestamp = ts;
	return 0;
}


int InputFileFFmpeg::seek( double seconds )
{
	Lock lock(m_mutex);
	if(!m_opened)
		return -1;

	return seekNoLock(seconds);
}


int InputFileFFmpeg::close()
{
	Lock lock(m_mutex);
	return closeNoLock();
}


int InputFileFFmpeg::handleDecoded(AVFrame *frame, SampleProducer *sb)
{
	// Need to skip some samples? We currently can't do this while flushing, so do it here before the first conversion
	if (frame && m_nextSeekTimestamp > 0) 
	{
		int64_t curTs = frame->pts;
		if (curTs < 0)
			curTs = frame->pkt_dts; // Fall back to pkt_dts if pts is not set, should be close enough for skipping
		if (curTs < 0) // Still broken? Just give up on skipping, play the frame in full
			curTs = m_nextSeekTimestamp;

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

	int res;
	int generatedSamples = 0;
	do {
		res = swr_convert(m_swrCtx, &m_outBuf, OUTPUT_BUFFER_COUNT,
			frame ? (const uint8_t **)frame->extended_data : NULL,
			frame ? frame->nb_samples : 0);
		if(res < 0)
			return res;
		int64_t outSamples = std::max(int64_t(0), res - m_skipSamples);
		int64_t skippedSamples = res - outSamples;
		if(m_maxConvertedSamples > 0 && outSamples > (m_maxConvertedSamples - m_convertedSamples))
		{
			outSamples = m_maxConvertedSamples - m_convertedSamples;
			m_done = true;
		}
		if(outSamples > 0)
			sb->produce(((int16_t*)m_outBuf) + (skippedSamples * m_outputChannels), outSamples);

		m_skipSamples -= skippedSamples;
		m_convertedSamples += outSamples;
		generatedSamples += outSamples;
		frame = NULL; // Only use the frame for the first conversion, then pass NULL to flush the resampler
	} while (!m_done && res == OUTPUT_BUFFER_COUNT); // If we filled the whole output buffer, there might be more data to convert, so try again immediately

	return generatedSamples;
}


// Returns the number of generated samples, or a negative error code
int InputFileFFmpeg::receiveSamples(SampleProducer *sampleBuffer, int& producedSamples)
{
	int result = avcodec_receive_frame(m_codecCtx, m_frame);
	if (result < 0)
	{
		// Report error if it's not EAGAIN (need more packets) or EOF (flushed all frames)
		if (result != AVERROR(EAGAIN) && result != AVERROR_EOF)
			checkFFmpegErr(result, "Error while receiving frame");
	}
	else
	{
		m_decodedSamples += m_frame->nb_samples;

		// Resample
		auto decodeRes = handleDecoded(m_frame, sampleBuffer);
		if (checkFFmpegErr(decodeRes, "Unable to resample") < 0)
		{
			av_frame_unref(m_frame);
			return decodeRes;
		}
		producedSamples = decodeRes;
	}

	av_frame_unref(m_frame);
	return result;
}


// Purpose: Read a bunch of samples and push them into sampleBuffer.
// Returns the number of read samples, or a negative error code
int InputFileFFmpeg::readSamples(SampleProducer *sampleBuffer)
{
	Lock lock(m_mutex);

	if(!m_opened)
		return -1;

	int written = 0; //samples read
	while(!m_done && written == 0)
	{
		int readRet = av_read_frame(m_fmtCtx, m_packet);
		bool eofFlush = false;
		if (readRet < 0)
		{
			if (readRet != AVERROR_EOF) // not just EOF? Return real error.
			{
				checkFFmpegErr(readRet, "Error while reading frame");
				av_packet_unref(m_packet);
				return readRet;
			}
			// File is EOF, but we still have data in the decoder.
			eofFlush = true;
			m_done = true;
		}
		else if (m_packet->stream_index != m_streamIndex)
		{
			av_packet_unref(m_packet);
			continue;
		}

		// Patch missing PTS (can happen with some formats/codecs, e.g. MP3) by using DTS, which is usually set if PTS is not.
		if (m_packet->pts == AV_NOPTS_VALUE && m_packet->dts != AV_NOPTS_VALUE)
			m_packet->pts = m_packet->dts;

		int sendRet = avcodec_send_packet(m_codecCtx, eofFlush ? NULL : m_packet);
		av_packet_unref(m_packet); // Unref immediately after sending
		if (checkFFmpegErr(sendRet, "Error while sending packet to decoder") < 0)
		{
			return sendRet;
		}

		while (true)
		{
			int producedSamples = 0;
			int receiveRet = receiveSamples(sampleBuffer, producedSamples);
			if (receiveRet == AVERROR(EAGAIN) || receiveRet == AVERROR_EOF)
				break; // Break inner loop, try to read next packet
			else if (receiveRet < 0) // decoding error :(
				return receiveRet; // Fatal error, return it
			else
				written += producedSamples;
		}

		if (eofFlush)
		{
			int flushedSamples = handleDecoded(NULL, sampleBuffer);
			if (flushedSamples > 0)
				written += flushedSamples;

			break; // break outer loop, return what we got so far
		}
	}

	return written;
}



bool InputFileFFmpeg::done() const 
{
	return m_done;
}



int InputFileFFmpeg::getAudioStreamNum() const
{
	return av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
}



int InputFileFFmpeg::closeNoLock()
{
	if (m_frame)
		av_frame_free(&m_frame);

	if (m_packet)
		av_packet_free(&m_packet);

	if(m_swrCtx)
		swr_free(&m_swrCtx);

	if(m_codecCtx)
		avcodec_free_context(&m_codecCtx);

	if(m_fmtCtx)
		avformat_close_input(&m_fmtCtx);

	m_opened = false;

	return 0;
}



int64_t InputFileFFmpeg::outputSamplesEstimation() const
{
	AVStream *stream = m_fmtCtx->streams[m_streamIndex];
	if (stream->duration > 0)
		return stream->duration * (int64_t)stream->time_base.num *
			(int64_t)m_outputSamplerate / (int64_t)stream->time_base.den;
	else
		return m_fmtCtx->duration * m_outputSamplerate / AV_TIME_BASE;
}



InputFile *CreateInputFileFFmpeg(InputFileOptions options /*= InputFileOptions()*/)
{
	return new InputFileFFmpeg(options);
}
