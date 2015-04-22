#ifndef inputfile_H__
#define inputfile_H__

#define NOMINMAX
#include <Windows.h>
#include "SampleSource.h"

class SampleBuffer;

class InputFile : public SampleSource
{
public:
	virtual ~InputFile() {};
	virtual int open(const char *filename, double startPosSeconds = 0.0, double playTimeSeconds = -1.0) = 0;
	virtual int close() = 0;
	virtual bool done() const {DebugBreak(); return 0;}
	virtual int seek(double seconds) = 0;
};

extern InputFile *CreateInputFileFFmpeg();
extern void InitFFmpegLibrary();


#endif // inputfile_H__
