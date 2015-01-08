
#ifndef SampleBuffer_H__
#define SampleBuffer_H__

#include <vector>
#include <mutex>

class SampleBuffer
{
public:
	class ProduceCallback
	{
	public:
		virtual void onProduceSamples(const short *samples, int count, SampleBuffer *caller) = 0;
	};

	class ConsumeCallback
	{
	public:
		virtual void onConsumeSamples(const short *samples, int count, SampleBuffer *caller) = 0;
	};

public:
	SampleBuffer(int channels);

	//Set the callback that is called when samples are placed into the buffer (produced)
	inline void setOnProduce(ProduceCallback *cb) { 
		m_cbProd = cb;
	}

	//Get the callback that is called when samples are placed into the buffer (produced)
	inline ConsumeCallback *getOnProduce() const {
		return m_cbCons;
	}

	//Set the callback that is called when samples are read from the buffer (consumed)
	inline void setOnConsume(ConsumeCallback *cb) {
		m_cbCons = cb;
	}

	//Get the callback that is called when samples are read from the buffer (consumed)
	inline ConsumeCallback *getOnConsume() const {
		return m_cbCons;
	}

	//Get the number of available samples
	//One sample is (2 * channels) bytes in size
	inline int avail() const { 
		return m_buf.size() / m_channels; 
	}

	//Return the number of channels this buffer was initialized with
	inline int channels() const {
		return m_channels;
	}

	//Place some samples into the buffer
	//samples: The sample buffer
	//count: Number of samples in buffer
	//One sample is (2 * channels) bytes in size
	void produce(const short *samples, int count);

	//Consume some samples from the buffer
	//samples: The sample buffer
	//count: Size of buffer measured in Samples
	//eraseConsumed: Erase the consumed samples from the buffer
	//One sample is (2 * channels) bytes in size
	int consume(short *samples, int maxCount, bool eraseConsumed = true);

	//Get size of a sample in bytes
	inline int sampleSize() const {
		return 2 * m_channels;
	}

private:
	const int m_channels;
	std::vector<short> m_buf;
	std::mutex m_mutex;
	ProduceCallback *m_cbProd;
	ConsumeCallback *m_cbCons;
};

#endif // SampleBuffer_H__