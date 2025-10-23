#ifndef __TODSTRINGFILE_HPP__
#define __TODSTRINGFILE_HPP__

#include "graphics/graphics.hpp"
using namespace PopLib;

enum DrawStringJustification;
enum TodStringFormatFlag
{
    TOD_FORMAT_IGNORE_NEWLINES,
    TOD_FORMAT_HIDE_UNTIL_MAGNETSHROOM
};

class TodStringListFormat
{
public:
    const char*     mFormatName;
    Font**          mNewFont;
    Color           mNewColor;
    int             mLineSpacingOffset;
    uint32_t    mFormatFlags;

public:
    TodStringListFormat();
    TodStringListFormat(const char* theFormatName, Font** theFont, const Color& theColor, int theLineSpacingOffset, uint32_t theFormatFlags);
};
extern int gTodStringFormatCount;               //[0x69DE4C]
extern TodStringListFormat* gTodStringFormats;  //[0x69DA34]

extern int gLawnStringFormatCount;
extern TodStringListFormat gLawnStringFormats[14];  //0x6A5010

void                TodStringListSetColors(TodStringListFormat* theFormats, int theCount);
void                TodWriteStringSetFormat(const char* theFormat, TodStringListFormat& theCurrentFormat);
bool                TodStringListReadName(const char*& thePtr, std::string& theName);
bool                TodStringListReadValue(const char*& thePtr, std::string& theValue);
bool                TodStringListReadItems(const char* theFileText);
bool                TodStringListReadFile(const char* theFileName);
void                TodStringListLoad(const char* theFileName);
PopString          TodStringListFind(const PopString& theName);
PopString			TodStringTranslate(const PopString& theString);
PopString			TodStringTranslate(const PopChar* theString);
bool                TodStringListExists(const PopString& theString);
void                TodStringRemoveReturnChars(std::string& theString);
bool                CharIsSpaceInFormat(char theChar, const TodStringListFormat& theCurrentFormat);
int                 TodWriteString(Graphics* g, const PopString& theString, int theX, int theY, TodStringListFormat& theCurrentFormat, int theWidth, DrawStringJustification theJustification, bool drawString, int theOffset, int theLength);
/*inline*/ int      TodWriteWordWrappedHelper(Graphics* g, const PopString& theString, int theX, int theY, TodStringListFormat& theCurrentFormat, int theWidth, DrawStringJustification theJustification, bool drawString, int theOffset, int theLength, int theMaxChars);
int                 TodDrawStringWrappedHelper(Graphics* g, const PopString& theText, const Rect& theRect, Font* theFont, const Color& theColor, DrawStringJustification theJustification, bool drawString);
/*inline*/ void		TodDrawStringWrapped(Graphics* g, const PopString& theText, const Rect& theRect, Font* theFont, const Color& theColor, DrawStringJustification theJustification);

#endif  //__TODSTRINGFILE_HPP__