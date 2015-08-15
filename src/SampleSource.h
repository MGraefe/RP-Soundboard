#ifndef SampleSource_H__
#define SampleSource_H__

#include "SampleProducer.h"

//Interface SampleSource
class SampleSource
{
public:
	virtual int readSamples(SampleProducer *sampleBuffer) = 0;
};

#endif // SampleSource_H__
