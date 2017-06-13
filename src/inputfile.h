// src/inputfile.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__inputfile_H__
#define rpsbsrc__inputfile_H__

#include <stdint.h>
#include "SampleSource.h"

class SampleBuffer;

struct InputFileOptions
{
	enum channel_layout_e
	{
		MONO = 0,
		STEREO,
	};

	channel_layout_e outputChannelLayout;
	int outputSampleRate;

	InputFileOptions() :
		outputChannelLayout(STEREO),
		outputSampleRate(48000)
	{}

	inline int getNumChannels() const
	{
		switch(outputChannelLayout)
		{
		case MONO:   return 1;
		case STEREO: return 2;
		default: return 0;
		}
	}
};


class InputFile : public SampleSource
{
public:
	virtual ~InputFile() {};
	virtual int open(const char *filename, double startPosSeconds = 0.0, double playTimeSeconds = -1.0) = 0;
	virtual int close() = 0;
	virtual int seek(double seconds) = 0;
	virtual int64_t outputSamplesEstimation() const = 0;
};

extern InputFile *CreateInputFileFFmpeg(InputFileOptions options = InputFileOptions());
extern void InitFFmpegLibrary();


#endif // rpsbsrc__inputfile_H__
