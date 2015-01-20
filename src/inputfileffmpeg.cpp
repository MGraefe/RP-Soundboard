#include "common.h"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <vector>

#include "ts3log.h"
#include "inputfile.h"
#include "SampleBuffer.h"
#include "SampleSource.h"
#include "device.h"
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
#define OUTPUT_CHANNELS 2
#define OUTPUT_CHANNEL_LAYOUT AV_CH_LAYOUT_STEREO
#define OUTPUT_SAMPLERATE 48000

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
	av_register_all();
}


int LogFFmpegError(int code, char *msg = NULL)
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
	InputFileFFmpeg();
	~InputFileFFmpeg();
	int open(const char *filename) override;
	int close() override;

	int readSamples(SampleBuffer *sampleBuffer) override;
	bool done() const override;

private:
	int _close();
	void reset();
	int getAudioStreamNum() const;
	int handleDecoded(AVFrame *frame, SampleBuffer *sb);
	int64_t getTargetSamples(int64_t sourceSamples, int64_t sourceSampleRate, int64_t targetSampleRate);
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
	typedef std::lock_guard<std::mutex> Lock;
};


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
InputFileFFmpeg::InputFileFFmpeg()
{
	reset();
	av_samples_alloc(&m_outBuf, NULL, OUTPUT_CHANNELS, OUTPUT_BUFFER_COUNT, OUTPUT_FORMAT, 0);
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
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
InputFileFFmpeg::~InputFileFFmpeg()
{
	close();
	av_freep(&m_outBuf);
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int InputFileFFmpeg::open(const char *filename )
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
		m_mutex.unlock();
		close();
		return -1;
	}

	m_streamIndex = getAudioStreamNum();
	if(m_streamIndex < 0)
	{
		logError("Cannot find a suitable stream");
		m_mutex.unlock();
		close();
		return -1;
	}

	m_codecCtx = m_fmtCtx->streams[m_streamIndex]->codec;
	AVCodec *codec = avcodec_find_decoder(m_codecCtx->codec_id);
	if(!codec)
	{
		logError("Unsupported codec");
		m_mutex.unlock();
		close();
		return -1;
	}

	m_codecCtx->codec = codec;
	m_codecCtx->channel_layout = av_get_default_channel_layout(m_codecCtx->channels);

	if(LogFFmpegError(avcodec_open2(m_codecCtx, m_codecCtx->codec, NULL), "Cannot open codec") < 0)
	{
		m_mutex.unlock();
		close();
		return -1; //Cannot open codec
	}


	//Open Resample context
	m_swrCtx = swr_alloc_set_opts(NULL,
		OUTPUT_CHANNEL_LAYOUT,		//Output layout (stereo)
		OUTPUT_FORMAT,				//Output format (signed 16bit int)
		OUTPUT_SAMPLERATE,			//Output Sample Rate
		m_codecCtx->channel_layout, //Input layout
		m_codecCtx->sample_fmt,		//Input format
		m_codecCtx->sample_rate,	//Input Sample Rate
		0, NULL);

	if(!m_swrCtx)
	{
		logError("Failed to allocate resample context");
		m_mutex.unlock();
		close();
		return -1;
	}

	if(LogFFmpegError(swr_init(m_swrCtx), "Cannot initialize resample context") < 0)
	{
		m_mutex.unlock();
		close();
		return -1;
	}

	m_opened = true;

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
int InputFileFFmpeg::handleDecoded(AVFrame *frame, SampleBuffer *sb)
{
	int res = swr_convert(m_swrCtx, &m_outBuf, OUTPUT_BUFFER_COUNT,
		frame ? (const uint8_t **)frame->extended_data : NULL,
		frame ? frame->nb_samples : 0);
	if(res > 0)
		sb->produce((short*)m_outBuf, res);
	return res;
	//const int sr_in = m_codecCtx->sample_rate;
	//const int sr_out = 48000;
	//const int sm_in = frame->nb_samples;
	//const int sm_in_del = swr_get_delay(m_swrCtx, sr_out) + sm_in;
	//const int sm_out = std::min((int)av_rescale_rnd(sm_in_del, sr_in, sr_out, AV_ROUND_UP), outCount / 2);

	//uint8_t *output;
	//int outputSize = av_samples_alloc(&output, NULL, 2, sm_out, AV_SAMPLE_FMT_S16, 0);
	//
	//uint8_t *out = (uint8_t*)outBuf;
	//int res = swr_convert(m_swrCtx, &output, sm_out, (const uint8_t **)frame->extended_data, frame->nb_samples);
	//if(res < 0)
	//	return res;

	//memcpy(outBuf, output, res * sizeof(short) * 2);
	//av_freep(&output);
	//return res * 2;
}



//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
int InputFileFFmpeg::readSamples(SampleBuffer *sampleBuffer)
{
	Lock lock(m_mutex);

	if(!m_opened)
		return -1;

	AVFrame *frame = av_frame_alloc();
	AVPacket packet;
	av_init_packet(&packet);
	int sampleSize = av_samples_get_buffer_size(NULL, m_codecCtx->channels, 1, m_codecCtx->sample_fmt, 1);
	int written = 0; //samples read

	int properFrames = 0;
	while(properFrames == 0 && av_read_frame(m_fmtCtx, &packet) == 0)
	{
		if(packet.stream_index == m_streamIndex)
		{
			properFrames++;
			AVPacket decodePacket = packet;
			while(decodePacket.size > 0)
			{
				// Try to decode the packet into a frame
				// Some frames rely on multiple packets, so we have to make sure the frame is finished before
				// we can use it
				int gotFrame = 0;
				int consumed = avcodec_decode_audio4(m_codecCtx, frame, &gotFrame, &decodePacket);
				if(consumed >= 0 && gotFrame)
				{
					decodePacket.size -= consumed;
					decodePacket.data += consumed;
					m_decodedSamples += frame->nb_samples;
					m_decodedSamplesTargetSR += getTargetSamples(frame->nb_samples, OUTPUT_SAMPLERATE, m_codecCtx->sample_rate);

					//Resample
					int res = handleDecoded(frame, sampleBuffer);
					if(LogFFmpegError(res, "Unable to resample") < 0)
					{
						av_free_packet(&packet);
						av_frame_free(&frame);
						return -1;
					}

					m_convertedSamples += res;
					written += res;
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

			av_init_packet(&packet);
			// Decode all the remaining frames in the buffer, until the end is reached
			int gotFrame = 0;
			while (avcodec_decode_audio4(m_codecCtx, frame, &gotFrame, &packet) >= 0 && gotFrame)
			{
				// We now have a fully decoded audio frame
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
		else
		{
			//Flush resampling
			int res;
			while(m_convertedSamples < m_decodedSamplesTargetSR && (res = handleDecoded(NULL, sampleBuffer)) > 0)
			{
				written += res;
			}
			m_done = true;
		}
	}

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
	//for(int i = 0; i < m_ctx->nb_streams; i++)
	//	if(m_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
	//		return i;
	//return -1;
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
InputFile *CreateInputFileFFmpeg()
{
	return new InputFileFFmpeg();
}
