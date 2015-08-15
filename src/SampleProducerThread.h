

#ifndef SampleProducerThread_H__
#define SampleProducerThread_H__

#include <thread>
#include <vector>

#include "SampleProducer.h"

class SampleBuffer;
class SampleSource;


class SampleProducerThread : public SampleProducer
{
	struct buffer_t
	{
		SampleBuffer *buffer;
		bool enabled;
	};

public:
	SampleProducerThread();
	void addBuffer(SampleBuffer *buffer, bool enableBuffer = true);
	void remBuffer(SampleBuffer *buffer);
	void setBufferEnabled(SampleBuffer *buffer, bool enabled);
	void start();
	void stop(bool wait = true);
	bool isRunning();
	void setSource(SampleSource *source);
	
private:
	void run();
	void threadFunc();
	int singleBufferFill();
	void produce(const short *samples, int count) override;

	typedef std::lock_guard<std::mutex> Lock;

	std::thread m_thread;
	SampleSource * volatile m_source;
	std::vector<buffer_t> m_buffers;
	bool m_running;
	volatile bool m_stop;
	std::mutex m_mutex;	
};

#endif // SampleProducerThread_H__