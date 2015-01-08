

#ifndef SampleProducerThread_H__
#define SampleProducerThread_H__

#include <thread>

class SampleBuffer;
class SampleSource;


class SampleProducerThread
{
public:
	SampleProducerThread(SampleBuffer *sampleBuffer);
	void start();
	void stop(bool wait = true);
	bool isRunning();
	void setSource(SampleSource *source);
	
private:
	void run();
	void threadFunc();

	std::thread m_thread;
	SampleSource *m_source;
	SampleBuffer * const m_buffer;
	bool m_running;
	volatile bool m_stop;
	
};

#endif // SampleProducerThread_H__