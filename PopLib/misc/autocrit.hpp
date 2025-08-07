#ifndef __AUTOCRIT_HPP__
#define __AUTOCRIT_HPP__

#pragma once

#include "common.hpp"
#include "critsect.hpp"

namespace PopLib
{

class AutoCrit
{
	std::unique_lock<std::recursive_mutex> mLock;

  public:
	AutoCrit(std::recursive_mutex theCritSec) : mLock(theCritSec)
	{
	}

	AutoCrit(CritSect &theCritSect) : mLock(theCritSect.mCriticalSection)
	{
	}

	~AutoCrit()
	{
	}
};

} // namespace PopLib

#endif