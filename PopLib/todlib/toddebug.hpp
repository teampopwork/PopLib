#ifndef __TODDEBUG_HPP__
#define __TODDEBUG_HPP__

#include <cstdlib>
#include <csignal>
#include "debug/sehcatcher.hpp"

#ifndef _WIN32
using LPEXCEPTION_POINTERS = void *; // dummy.
#endif

class TodHesitationBracket
{
public:
	char			mMessage[256];
	int				mBracketStartTime;

public:
	TodHesitationBracket(const char* theFormat, ...) { ; }
	~TodHesitationBracket() { ; }

	inline void		EndBracket() { ; }
};

void				TodLog(const char* theFormat, ...);
void				TodLogString(const char* theMsg);
void				TodTrace(const char* theFormat, ...);
void				TodTraceMemory();
void				TodTraceAndLog(const char* theFormat, ...);
void				TodTraceWithoutSpamming(const char* theFormat, ...);
void				TodHesitationTrace(...);
void				TodReportError(LPEXCEPTION_POINTERS exceptioninfo, const char* theMessage);
void				TodAssertFailed(const char* theCondition, const char* theFile, int theLine, const char* theMsg = "", ...);
/*inline*/ void		TodErrorMessageBox(const char* theMessage, const char* theTitle);

/*inline*/ void*	TodMalloc(int theSize);
/*inline*/ void		TodFree(void* theBlock);
void				TodAssertInitForApp();

#ifdef _DEBUG
#define TOD_ASSERT(condition, ...) do { \
    if (!(condition)) { \
        TodAssertFailed(#condition, __FILE__, __LINE__, ##__VA_ARGS__); \
        TodTraceMemory(); \
    } \
} while(0)
#else
#define TOD_ASSERT(condition, ...)
#endif

#endif
