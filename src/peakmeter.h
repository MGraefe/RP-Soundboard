// src/peakmeter.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2017 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#include <math.h>

class PeakMeter
{
private:
	static constexpr float minOutput = 0.001f;

public:
	PeakMeter(float alpha, float beta, int hold) :
		alpha(alpha),
		beta(beta),
		hold(hold),
		output(minOutput),
		timer(0)
	{}

	inline float process(float sample)
	{
		float absSample = fabs(sample);
		if (absSample > output) // attack
		{
			output = alpha * absSample + (1.0f - alpha) * output;
			timer = hold;
		}
		else if (timer == 0) // release
			output = (1.0f - beta) * output;
		else // hold
			timer--;
		if (output < minOutput)
			output = minOutput;
		return output;
	}

	inline short limit(float sample, float threshold) const
	{
		if (output > threshold)
			sample *= threshold / output;
		return (short)(sample + 0.5f);
	}

	inline float getOutput() const { return output; }

	inline void reset()
	{
		output = minOutput;
		timer = 0;
	}

private:
	const float alpha;
	const float beta;
	const int hold;
	float output;
	int timer;
};

