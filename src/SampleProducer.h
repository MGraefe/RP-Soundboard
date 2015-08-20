#ifndef SampleProducer_H__
#define SampleProducer_H__

class SampleProducer
{
public:
	virtual void produce(const short *samples, int count) = 0;
};

#endif // SampleProducer_H__
