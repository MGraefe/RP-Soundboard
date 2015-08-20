

#include "inputfile.h"
#include "SampleVisualizerThread.h"
#include "SampleBuffer.h"

#define MIN_SAMPLES_PER_ITERATION (1024 * 32)


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SampleVisualizerThread & SampleVisualizerThread::GetInstance()
{
	static SampleVisualizerThread t;
	return t;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SampleVisualizerThread::SampleVisualizerThread() :
	m_buffer(1),
	m_numBins(0),
	m_numBinsProcessed(0),
	m_numSamplesProcessed(0),
	m_numSamplesTotalEst(0),
	m_numSamplesProcessedThisBin(0),
	m_file(NULL),
	m_running(false),
	m_newFile(false),
	m_stop(false)
{

}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
SampleVisualizerThread::~SampleVisualizerThread()
{
	stop();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleVisualizerThread::startAnalysis( const char *filename, size_t numBins )
{
	Lock lock(m_mutex);

	m_filename = filename;
	m_numBins = numBins;
	m_numBinsProcessed = 0;
	m_numSamplesProcessed = 0;
	m_numSamplesTotalEst = 0;
	m_numSamplesProcessedThisBin = 0;
	m_bins.clear();
	m_newFile = true;

	if(!m_running)
	{
		m_running = true;
		m_stop = false;
		std::thread t(&SampleVisualizerThread::threadFunc, this);
		m_thread = std::move(t);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleVisualizerThread::stop( bool wait /*= true*/ )
{
	m_stop = true;
	if(wait && m_thread.joinable())
		m_thread.join();
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
bool SampleVisualizerThread::isRunning() const
{
	return m_running;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
size_t SampleVisualizerThread::getBinsProcessed() const
{
	return m_numBinsProcessed;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleVisualizerThread::run()
{
	while(!m_stop)
	{
		m_mutex.lock();
		if(m_newFile)
			openNewFile();

		int readSamplesThisIt = 0;
		while(m_file && readSamplesThisIt < MIN_SAMPLES_PER_ITERATION)
		{
			int samples = m_file->readSamples(&m_buffer);
			if(samples <= 0 || m_file->done())
			{
				m_file->close();
				delete m_file;
				m_file = NULL;
			}
			if(samples > 0)
			{
				readSamplesThisIt += samples;
				processSamples(samples);
			}
		}
		
		int sleepTime = m_file ? 1 : 100;
		m_mutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleVisualizerThread::threadFunc()
{
	run();
	m_running = false;
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleVisualizerThread::openNewFile()
{
	m_newFile = false;
	if(m_file)
		delete m_file;
	InputFileOptions options;
	options.outputChannelLayout = InputFileOptions::MONO;
	options.outputSampleRate = 44100;
	m_file = CreateInputFileFFmpeg(options);
	if(m_file->open(m_filename.c_str()) == 0)
		m_numSamplesTotalEst = m_file->outputSamplesEstimation();
	else
	{
		delete m_file;
		m_file = NULL;
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleVisualizerThread::processSamples(size_t newSamples)
{
	while(true)
	{
		size_t samplesPerBin = (size_t)(m_numSamplesTotalEst / (int64_t)m_numBins);
		if(m_numSamplesProcessedThisBin == 0)
		{
			m_min = std::numeric_limits<short>::max();
			m_max = std::numeric_limits<short>::min();
		}
		size_t numSamplesThisIt = std::min((size_t)m_buffer.avail(), samplesPerBin - m_numSamplesProcessedThisBin);
		if(numSamplesThisIt == 0)
			break;
		getMinMax(m_buffer.getBufferData(), numSamplesThisIt, m_min, m_max);
		m_buffer.consume(NULL, numSamplesThisIt);
		m_numSamplesProcessedThisBin += numSamplesThisIt;
		if(m_numSamplesProcessedThisBin >= samplesPerBin)
		{
			m_bins.push_back(m_min);
			m_bins.push_back(m_max);
			m_numSamplesProcessedThisBin = 0;
			m_numBinsProcessed++;
		}
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
void SampleVisualizerThread::getMinMax( const short *data, size_t count, int &min, int &max )
{

	for(size_t i = 0; i < count; ++i)
	{
		min = std::min(min, (int)data[i]);
		max = std::max(max, (int)data[i]);
	}
}


//---------------------------------------------------------------
// Purpose: 
//---------------------------------------------------------------
volatile const int * SampleVisualizerThread::getBins() const
{
	return m_bins.data();
}





