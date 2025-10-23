#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#pragma once

#pragma warning(disable : 4786)
#pragma warning(disable : 4503)

#undef _WIN32_WINNT
#undef WIN32_LEAN_AND_MEAN

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#undef _UNICODE
#undef UNICODE

#ifndef NOMINMAX
#define NOMINMAX
#endif

//vorbis workaounds since for some fucking reason, vorbis force defines min and max.
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

#include <string>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <mmsystem.h>
#else
#include <wctype.h>
#include <string.h>
#include <stdint.h>

#define _stricmp strcasecmp
#define stricmp strcasecmp

#define _cdecl
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
typedef uint32_t UINT;
typedef int64_t __int64;

typedef std::map<std::string, std::string> DefinesMap;
#endif

#include "readwrite/modval.hpp"

#ifdef _WIN32
#include <malloc.h>
#define STACK_ALLOC(size) _alloca(size)
#else
#include <alloca.h>
#define STACK_ALLOC(size) alloca(size)
#endif

typedef std::string PopString;

#define LONG_BIGE_TO_NATIVE(l)                                                                                         \
	(((l >> 24) & 0xFF) | ((l >> 8) & 0xFF00) | ((l << 8) & 0xFF0000) | ((l << 24) & 0xFF000000))
#define WORD_BIGE_TO_NATIVE(w) (((w >> 8) & 0xFF) | ((w << 8) & 0xFF00))
#define LONG_LITTLEE_TO_NATIVE(l) (l)
#define WORD_LITTLEE_TO_NATIVE(w) (w)

#define LENGTH(anyarray) (sizeof(anyarray) / sizeof(anyarray[0]))

#ifndef PI
#define PI              (3.1415926536f)
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
#ifdef ulong
#undef ulong
#endif
#define ulong uint32_t
// int64 has been removed due to Steam API. I hate this job.

typedef std::map<std::string, std::string> DefinesMap;
typedef std::map<std::wstring, std::wstring> WStringWStringMap;
typedef PopString::value_type PopChar;
#define HAS_PopChar

// @ThePixelMoon: stupid cmake refuses to set these,
// TODO: change these every new version

#ifndef POPLIB_VERSION_MAJOR
#define POPLIB_VERSION_MAJOR 2
#endif

#ifndef POPLIB_VERSION_MINOR
#define POPLIB_VERSION_MINOR 1
#endif

#ifndef POPLIB_VERSION_PATCH
#define POPLIB_VERSION_PATCH 0
#endif

#ifndef POPLIB_VERSION_STAGE
#define POPLIB_VERSION_STAGE "alpha"
#endif

#define STR_HELPER(x) #x
#define STR_MACRO(x) STR_HELPER(x)
#define StringToPopString(x) x

#define POPLIB_VERSION STR_MACRO(POPLIB_VERSION_MAJOR) "." STR_MACRO(POPLIB_VERSION_MINOR) "." STR_MACRO(POPLIB_VERSION_PATCH) "-" STR_MACRO(POPLIB_VERSION_STAGE)

#ifdef _DEBUG
#define LOGGING_ENABLED
#define ASSERT_ENABLED
#endif

// Holy shit, more compatibility!
#define _S(x)                	 x
#define PopStringToString(x) 	 x
#define PopStringToStringFast(x) x

enum DrawStringJustification
{
    DS_ALIGN_LEFT = 0,
    DS_ALIGN_RIGHT = 1,
    DS_ALIGN_CENTER = 2,
    DS_ALIGN_LEFT_VERTICAL_MIDDLE = 3,
    DS_ALIGN_RIGHT_VERTICAL_MIDDLE = 4,
    DS_ALIGN_CENTER_VERTICAL_MIDDLE = 5
};

enum EmitterType
{
    EMITTER_CIRCLE = 0,
    EMITTER_BOX = 1,
    EMITTER_BOX_PATH = 2,
    EMITTER_CIRCLE_PATH = 3,
    EMITTER_CIRCLE_EVEN_SPACING = 4
};

enum EffectType
{
    EFFECT_PARTICLE = 0,
    EFFECT_TRAIL = 1,
    EFFECT_REANIM = 2,
    EFFECT_ATTACHMENT = 3,
    EFFECT_OTHER = 4
};

enum TodCurves
{
    CURVE_CONSTANT,             // 常函数曲线
    CURVE_LINEAR,               // 线性曲线
    CURVE_EASE_IN,              // 二次曲线（缓入）
    CURVE_EASE_OUT,             // 二次曲线（缓出）
    CURVE_EASE_IN_OUT,          // 缓入缓出曲线
    CURVE_EASE_IN_OUT_WEAK,     // 缓入缓出曲线（效果减弱）
    CURVE_FAST_IN_OUT,          // 快入快出曲线
    CURVE_FAST_IN_OUT_WEAK,     // 快入快出曲线（效果减弱）
    CURVE_WEAK_FAST_IN_OUT,     // 【废弃】弱快入快出曲线
    CURVE_BOUNCE,               // 弹跳效果曲线
    CURVE_BOUNCE_FAST_MIDDLE,   // 弹跳效果曲线（尖形）
    CURVE_BOUNCE_SLOW_MIDDLE,   // 弹跳效果曲线（罩形）
    CURVE_SIN_WAVE,             // 正弦曲线
    CURVE_EASE_SIN_WAVE         // 缓入缓出的正弦曲线
};

enum ReanimLoopType
{
    REANIM_LOOP = 0,
    REANIM_LOOP_FULL_LAST_FRAME = 1,
    REANIM_PLAY_ONCE = 2,
    REANIM_PLAY_ONCE_AND_HOLD = 3,
    REANIM_PLAY_ONCE_FULL_LAST_FRAME = 4,
    REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD = 5
};

#ifndef COMPILING_PVZ
enum ParticleEffect
{
    PARTICLE_NONE = -1,
	PARTICLE_EXAMPLE,
    NUM_PARTICLES
};
#endif

//////////////////////////////
enum AttachmentID { ATTACHMENTID_NULL };
enum CoinID { COINID_NULL };
enum ParticleID { PARTICLEID_NULL };
enum ParticleEmitterID { PARTICLEEMITTERID_NULL };
enum ParticleSystemID { PARTICLESYSTEMID_NULL };
enum PlantID { PLANTID_NULL };
enum ReanimationID { REANIMATIONID_NULL };
enum ZombieID { ZOMBIEID_NULL };

enum TrialType
{
    TRIALTYPE_NONE,
    TRIALTYPE_STAGELOCKED
};

/**
 * @namespace PopLib
 * @brief root namespace for all PopLib classes
 */
namespace PopLib
{

/// @brief maximum random value
const ulong POPLIB_RAND_MAX = 0x7FFFFFFF;

extern bool gDebug;

/// @brief random
/// @return random int
int Rand();
/// @brief random range
/// @param range 
/// @return random int
int Rand(int range);
/// @brief float random range
/// @param range 
/// @return random float
float Rand(float range);
/// @brief ulong random
/// @param theSeed 
void SRand(ulong theSeed);
/// @brief TBA
/// @param fmt 
/// @param argPtr 
/// @return string
extern std::string vformat(const char *fmt, va_list argPtr);
/// @brief TBA
/// @param fmt 
/// @param argPtr 
/// @return widestring
extern std::wstring vformat(const wchar_t *fmt, va_list argPtr);
/// @brief TBA
/// @param ... 
/// @return string
extern std::string StrFormat(const char *fmt...);
/// @brief TBA
/// @param ... 
/// @return widestring
extern std::wstring StrFormat(const wchar_t *fmt...);
/// @brief obsolete
/// @return false
bool CheckFor98Mill();
/// @brief obsolete
/// @return false
bool CheckForVista();
/// @brief gets the appdata folder
/// @return .config on linux, appdata/local on windows
std::string GetAppDataFolder();
/// @brief obsolete
/// @param thePath 
void SetAppDataFolder(const std::string &thePath);
/// @brief encodes a url
/// @param theString 
/// @return encoded url string
std::string URLEncode(const std::string &theString);
/// @brief converts a string to uppercase
/// @param theString 
/// @return string
std::string StringToUpper(const std::string &theString);
/// @brief converts a string to uppercase
/// @param theString 
/// @return wide string
std::wstring StringToUpper(const std::wstring &theString);
/// @brief converts a string to lowercase
/// @param theString 
/// @return string
std::string StringToLower(const std::string &theString);
/// @brief converts a string to lowercase
/// @param theString 
/// @return wide string
std::wstring StringToLower(const std::wstring &theString);
/// @brief converts a string to wide string
/// @param theString 
/// @return widestring
std::wstring StringToWString(const std::string &theString);
/// @brief converts a wide string to string
/// @param theString 
/// @return string
std::string WStringToString(const std::wstring &theString);
/// @brief TBA
/// @param theData 
/// @return string
std::string Upper(const std::string &theData);
/// @brief TBA
/// @param theData 
/// @return wide string
std::wstring Upper(const std::wstring &theData);
/// @brief TBA
/// @param theData 
/// @return string
std::string Lower(const std::string &theData);
/// @brief TBA
/// @param theData 
/// @return wide string
std::wstring Lower(const std::wstring &theData);
/// @brief TBA
/// @param theString 
/// @return string
std::string Trim(const std::string &theString);
/// @brief TBA
/// @param theString 
/// @return wide string
std::wstring Trim(const std::wstring &theString);
/// @brief TBA
/// @param theString 
/// @param theIntVal 
/// @return true if success
bool StringToInt(const std::string theString, int *theIntVal);
/// @brief TBA
/// @param theString 
/// @param theDoubleVal 
/// @return true if success
bool StringToDouble(const std::string theString, double *theDoubleVal);
/// @brief TBA
/// @param theString 
/// @param theIntVal 
/// @return true if success
bool StringToInt(const std::wstring theString, int *theIntVal);
/// @brief TBA
/// @param theString 
/// @param theDoubleVal 
/// @return true if success
bool StringToDouble(const std::wstring theString, double *theDoubleVal);
/// @brief TBA
/// @param theStr 
/// @param theFind 
/// @return integer
int StrFindNoCase(const char *theStr, const char *theFind);
bool StrPrefixNoCase(const char *theStr, const char *thePrefix, int maxLength = 10000000);
PopString CommaSeperate(int theValue);
std::string Evaluate(const std::string &theString, const DefinesMap &theDefinesMap);
std::string XMLDecodeString(const std::string &theString);
std::string XMLEncodeString(const std::string &theString);
std::wstring XMLDecodeString(const std::wstring &theString);
std::wstring XMLEncodeString(const std::wstring &theString);

bool Deltree(const std::string &thePath);
bool FileExists(const std::string &theFileName);
void MkDir(const std::string &theDir);
std::string GetFileName(const std::string &thePath, bool noExtension = false);
std::string GetFileDir(const std::string &thePath, bool withSlash = false);
std::string RemoveTrailingSlash(const std::string &theDirectory);
std::string AddTrailingSlash(const std::string &theDirectory, bool backSlash = false);
time_t GetFileDate(const std::string &theFileName);
std::string GetCurDir();
std::string GetFullPath(const std::string &theRelPath);
std::string GetPathFrom(const std::string &theRelPath, const std::string &theDir);
bool AllowAllAccess(const std::string &theFileName);

inline void inlineUpper(std::string &theData)
{
	// std::transform(theData.begin(), theData.end(), theData.begin(), toupper);

	int aStrLen = (int)theData.length();
	for (int i = 0; i < aStrLen; i++)
	{
		theData[i] = toupper(theData[i]);
	}
}

inline void inlineUpper(std::wstring &theData)
{
	// std::transform(theData.begin(), theData.end(), theData.begin(), toupper);

	int aStrLen = (int)theData.length();
	for (int i = 0; i < aStrLen; i++)
	{
		theData[i] = towupper(theData[i]);
	}
}

inline void inlineLower(std::string &theData)
{
	std::transform(theData.begin(), theData.end(), theData.begin(), tolower);
}

inline void inlineLower(std::wstring &theData)
{
	std::transform(theData.begin(), theData.end(), theData.begin(), tolower);
}

inline void inlineLTrim(std::string &theData, const std::string &theChars = " \t\n")
{
	theData.erase(0, theData.find_first_not_of(theChars));
}

inline void inlineLTrim(std::wstring &theData, const std::wstring &theChars = L" \t\n")
{
	theData.erase(0, theData.find_first_not_of(theChars));
}

inline void inlineRTrim(std::string &theData, const std::string &theChars = " \t\n")
{
	theData.resize(theData.find_last_not_of(theChars) + 1);
}

inline void inlineTrim(std::string &theData, const std::string &theChars = " \t\n")
{
	inlineRTrim(theData, theChars);
	inlineLTrim(theData, theChars);
}

struct StringLessNoCase
{
	bool operator()(const std::string &s1, const std::string &s2) const
	{
		return _stricmp(s1.c_str(), s2.c_str()) < 0;
	}
};

} // namespace PopLib

// @ThePixelMoon: compatibility.
namespace Sexy = PopLib;

#ifndef COMPILING_PVZ
#define BOARD_WIDTH gAppBase->mWidth
#define BOARD_HEIGHT gAppBase->mHeight
#define gGetCurrentLevelName() "None"
#define gAppHasUsedCheatKeys() false
#endif

#endif