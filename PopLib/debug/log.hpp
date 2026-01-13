#ifndef __LOG_HPP__
#define __LOG_HPP__

#pragma once

#include "common.hpp"
#include <cassert>
namespace PopLib
{
	extern bool gInAssert;

	enum LogType
	{
		TYPE_INFO,
		TYPE_WARNING,
		TYPE_ERROR,
		TYPE_DEBUG,
	};

	
    void Log(LogType type, const char* file, int line, const char* fmt, ...);;
    void Assert(const char* expr, const char* file, int line, const char* msg = nullptr);

	#ifdef LOGGING_ENABLED
    #define LOG_INFO(fmt, ...) Log(LogType::TYPE_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_WARN(fmt, ...) Log(LogType::TYPE_WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_ERROR(fmt, ...) Log(LogType::TYPE_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define LOG_DEBUG(fmt, ...) Log(LogType::TYPE_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
	#else
	#define LOG_INFO(fmt, ...)
    #define LOG_WARN(fmt, ...)
    #define LOG_ERROR(fmt, ...)
    #define LOG_DEBUG(fmt, ...)
	#endif

	#ifdef ASSERT_ENABLED
    #define ASSERT(expr) \
        ((expr) ? (void)0 : Assert(#expr, __FILE__, __LINE__))

    #define ASSERT_MSG(expr, msg) \
        ((expr) ? (void)0 : Assert(#expr, __FILE__, __LINE__, msg))
	#else

	#define ASSERT(expr) 
	#define ASSERT_MSG(expr, msg) 

	#endif
}
#endif