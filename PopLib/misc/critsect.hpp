#ifndef __CRITSECT_HPP__
#define __CRITSECT_HPP__

#pragma once

#include "common.hpp"
#include <mutex>

// We are using std::recursive_mutex because the old Win32 CRITICAL_SECTION was recursive by default.

class CritSync;
namespace PopLib
{

class CritSect
{
  private:
	std::recursive_mutex mCriticalSection;
	friend class AutoCrit;

  public:
	CritSect(void){};
	~CritSect(void){};
};

} // namespace PopLib

#endif