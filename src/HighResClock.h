#ifndef HighResClock_H__
#define HighResClock_H__

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

#endif // HighResClock_H__
