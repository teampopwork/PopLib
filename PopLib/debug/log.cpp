#include "log.hpp"

#include "misc/autocrit.hpp"
#include "misc/critsect.hpp"

#include <time.h>
#include <stdarg.h>
#include <SDL3/SDL.h>

#include "memmgr.hpp"

bool PopLib::gInAssert = false;

using namespace PopLib;

const char* GetTypeString(LogType type)
{
    switch (type) {
        case LogType::TYPE_INFO:    return "INFO";
        case LogType::TYPE_WARNING: return "WARN";
        case LogType::TYPE_ERROR:   return "ERROR";
        case LogType::TYPE_DEBUG:   return "DEBUG";
    }
    return "UNKNOWN";
}


void PopLib::Log(LogType type, const char* file, int line, const char* fmt, ...)
{
    const char* logType = GetTypeString(type);
    SDL_Log("[%s][%s:%d]", logType, file, line); //Log the type, file and line.
    va_list args;
    va_start(args, fmt);

    int sdlCategory = SDL_LOG_CATEGORY_APPLICATION;
    SDL_LogPriority priority;

    switch (type) {
        case TYPE_INFO:    priority = SDL_LOG_PRIORITY_INFO; break;
        case TYPE_WARNING: priority = SDL_LOG_PRIORITY_WARN; break;
        case TYPE_ERROR:   priority = SDL_LOG_PRIORITY_ERROR; break;
        case TYPE_DEBUG:   priority = SDL_LOG_PRIORITY_DEBUG; break;
        default:           priority = SDL_LOG_PRIORITY_VERBOSE; break;
    }
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, priority, fmt, args);
    va_end(args);
    SDL_Log("\n");
}

void PopLib::Assert(const char* expr, const char* file, int line, const char* msg)
{
    LOG_ERROR("Assertion failed: (%s), [%s:%d]", expr, file, line);
    if (msg)
        LOG_INFO("Assertion message: %s", msg);

	gInAssert = true;
    #if defined(_MSC_VER)
        __debugbreak();
    #elif defined(__GNUC__) || defined(__clang__)
        __builtin_trap();
    #else
        std::abort();
    #endif
	gInAssert = false;
}
