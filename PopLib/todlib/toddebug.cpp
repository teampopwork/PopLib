#include <time.h>
#include "toddebug.hpp"
#include "todcommon.hpp"
#include "debug/sehcatcher.hpp"
#include "popapp.hpp"
#include "debug/log.hpp"

using namespace PopLib;

static char gLogFileName[MAX_PATH];
static char gDebugDataFolder[MAX_PATH];

//0x514EA0
void TodErrorMessageBox(const char* theMessage, const char* theTitle)
{
	TodTraceAndLog("%s.%s", theMessage, theTitle);

    SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK" }
    };

    SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_ERROR,
        NULL,
        theTitle,
        theMessage,
        SDL_arraysize(buttons),
        buttons,
        NULL
    };

    int buttonid;
    SDL_ShowMessageBox(&messageboxdata, &buttonid);
}

void TodTraceMemory()
{
}

void* TodMalloc(int theSize)
{
	TOD_ASSERT(theSize > 0);
	return malloc(theSize);
}

void TodFree(void* theBlock)
{
	if (theBlock != nullptr)
	{
		free(theBlock);
	}
}

void TodAssertFailed(const char* theCondition, const char* theFile, int theLine, const char* theMsg, ...)
{
	TodTraceAndLog("TODO: TodAssertFailed\n");
}

void TodLog(const char* theFormat, ...)
{
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	TodLogString(aButter);
}

void TodLogString(const char* theMsg)
{
	TodTraceAndLog("TODO: TodLogString\n");
}

void TodTrace(const char* theFormat, ...)
{
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	LOG_INFO(aButter);
}

void TodHesitationTrace(...)
{
}

void TodTraceAndLog(const char* theFormat, ...)
{
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	LOG_INFO(aButter);
	TodLogString(aButter);
}

void TodTraceWithoutSpamming(const char* theFormat, ...)
{
	TodTraceAndLog("TODO: TodTraceWithoutSpamming\n");
}

void TodReportError(LPEXCEPTION_POINTERS exceptioninfo, const char* theMessage)
{
	PopLib::SEHCatcher::UnhandledExceptionFilter(exceptioninfo);
}

void TodAssertInitForApp()
{
	TodTraceAndLog("TODO: TodAssertInitForApp\n");
}
