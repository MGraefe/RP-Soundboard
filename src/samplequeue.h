#ifndef samplequeue_H__
#define samplequeue_H__

#include <queue>
#include <vector>
#include <stdint.h>

class SampleQueue
{
public:
	bool empty() { return m_queue.empty(); }
	bool addSamples(uint16_t *samples, int count);
private:
	std::queue<std::vector<uint16_t>> m_queue;
};

#endif // samplequeue_H__
