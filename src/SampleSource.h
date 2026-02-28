// src/SampleSource.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include "SampleProducer.h"

//Interface SampleSource
class SampleSource
{
public:
	virtual int readSamples(SampleProducer *sampleBuffer) = 0;
};

