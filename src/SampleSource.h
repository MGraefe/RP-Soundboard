#ifndef SampleSource_H__
#define SampleSource_H__

class SampleBuffer;

//Interface SampleSource
class SampleSource
{
public:
	virtual int readSamples(SampleBuffer *sampleBuffer) = 0;
};

#endif // SampleSource_H__
