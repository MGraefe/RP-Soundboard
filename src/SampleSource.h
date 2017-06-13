// src/SampleSource.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__SampleSource_H__
#define rpsbsrc__SampleSource_H__

#include "SampleProducer.h"

//Interface SampleSource
class SampleSource
{
public:
	virtual int readSamples(SampleProducer *sampleBuffer) = 0;
	virtual bool done() const = 0;
};

#endif // rpsbsrc__SampleSource_H__
