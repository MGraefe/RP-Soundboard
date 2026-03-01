// src/HighResClock.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <chrono>

#ifdef _MSC_VER
struct HighResClock
{
	typedef long long rep;
	typedef std::nano period;
	typedef std::chrono::duration<rep, period> duration;
	typedef std::chrono::time_point<HighResClock> time_point;
	static const bool is_steady = true;

	static time_point now();
};
#else
typedef std::chrono::high_resolution_clock HighResClock;
#endif
