#ifndef __WORKERTHREAD_HPP__
#define __WORKERTHREAD_HPP__

#pragma once

#include "common.hpp"
#include <SDL3/SDL.h>

namespace PopLib
{
class WorkerThread
{
  public:
	WorkerThread(const std::string &name);
	virtual ~WorkerThread();
	void DoTask(void (*)(void *), void *);
	void WaitForTask();
	bool IsProcessingTask();

	std::string mName;

  protected:
	static int StaticThreadProc(void *);

	SDL_Thread *mThread;
	SDL_Mutex *mMutex;
	SDL_Condition *mCond;

	void *mSignalEvent;
	void *mDoneEvent;
	void (*mTask)(void *);
	void *mTaskArg;
	bool mStopped;
	bool mTaskPending;

	void ThreadProc();
};
} // namespace PopLib

#endif