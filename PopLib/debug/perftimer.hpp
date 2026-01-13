#ifndef __PERFTIMER_HPP__
#define __PERFTIMER_HPP__

#pragma once

#include "common.hpp"

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

namespace PopLib
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class PerfTimer
{
  protected:
	int64_t mStart;
	double mDuration;
	bool mRunning;

	void CalcDuration();

  public:
	PerfTimer();
	void Start();
	void Stop();

	double GetDuration();

	static int64_t GetCPUSpeed(); // in Hz
	static int GetCPUSpeedMHz();
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Perf
{
  public:
	static void BeginPerf(bool measurePerfOverhead = false);
	static void EndPerf();
	static bool IsPerfOn();

	static void StartTiming(const char *theName);
	static void StopTiming(const char *theName);

	static std::string GetResults();
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AutoPerf
{
  public:
	const char *mName;
	bool mIsStarted;

	AutoPerf(const char *theName) : mName(theName), mIsStarted(true)
	{
		Perf::StartTiming(theName);
	}
	AutoPerf(const char *theName, bool doStart) : mIsStarted(doStart), mName(theName)
	{
		if (doStart)
			Perf::StartTiming(theName);
	}

	~AutoPerf()
	{
		Stop();
	}

	void Start()
	{
		if (!mIsStarted)
		{
			mIsStarted = true;
			Perf::StartTiming(mName);
		}
	}

	void Stop()
	{
		if (mIsStarted)
		{
			Perf::StopTiming(mName);
			mIsStarted = false;
		}
	}
};

} // namespace PopLib

#pragma warning(push)
#pragma warning(disable : 4005)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// #define PERF_ENABLED
#if defined(PERF_ENABLED) && !defined(RELEASEFINAL)

#define PERF_BEGIN(theName) Perf::StartTiming(theName)
#define PERF_END(theName) Perf::StopTiming(theName)
#define AUTO_PERF_MULTI(theName, theSuffix) AutoPerf anAutoPerf##theSuffix(theName)
#define AUTO_PERF_2(theName, theSuffix) AUTO_PERF_MULTI(theName, theSuffix)
#define AUTO_PERFL(theName)                                                                                    \
	AUTO_PERF_2(theName, __LINE__) // __LINE__ doesn't work correctly if Edit-and-Continue (/ZI) is enabled
#define AUTO_PERF(theName) AUTO_PERF_2(theName, UNIQUE)

#define PERF_BEGIN_COND(theName, theCond)                                                                      \
	if (theCond)                                                                                                       \
	Perf::StartTiming(theName)
#define PERF_END_COND(theName, theCond)                                                                        \
	if (theCond)                                                                                                       \
	Perf::StopTiming(theName)
#define AUTO_PERF_MULTI_COND(theName, theSuffix, theCond)                                                      \
	AutoPerf anAutoPerf##theSuffix(theName, theCond);
#define AUTO_PERF_COND_2(theName, theSuffix, theCond) AUTO_PERF_MULTI_COND(theName, theSuffix, theCond);
#define AUTO_PERF_CONDL(theName) AUTO_PERF_COND_2(theName, __LINE__, theCond)
#define AUTO_PERF_COND(theName) AUTO_PERF_COND_2(theName, UNIQUE, theCond)

#else

#define PERF_BEGIN(theName)
#define PERF_END(theName)
#define AUTO_PERF_MULTI(theName, theSuffix)
#define AUTO_PERF(theName)

#define PERF_BEGIN_COND(theName, theCond)
#define PERF_END_COND(theName, theCond)
#define AUTO_PERF_MULTI_COND(theName, theSuffix, theCond)
#define AUTO_PERF_COND(theName)

#endif

#pragma warning(pop)

#endif