// src/SampleProducer.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__SampleProducer_H__
#define rpsbsrc__SampleProducer_H__

class SampleProducer
{
public:
	virtual void produce(const short *samples, int count) = 0;
};

#endif // rpsbsrc__SampleProducer_H__
