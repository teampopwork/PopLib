#ifndef __COLOR_HPP__
#define __COLOR_HPP__

#pragma once

#include "common.hpp"

namespace PopLib
{

#pragma pack(push, 1)
struct ARGB // change this stupid fucking definition a 3rd TIME to the CORRECT one this time.
{
	unsigned char a, r, g, b;
};
struct YUV //add YUV color components based off of ITU standard reccomendations BT.601 & BT.709.  This will most likely be a conversion set.  
{
	unsigned char y, u, v;


};
#pragma pack(pop)

class Color
{
  public:
	int mRed;
	int mGreen;
	int mBlue;
	int mAlpha;
	
	static Color Black;
	static Color White;

  public:
	Color();
	Color(int theColor);
	Color(int theColor, int theAlpha);
	Color(int theRed, int theGreen, int theBlue);
	Color(int theRed, int theGreen, int theBlue, int theAlpha);
	Color(const ARGB &theColor);
	Color(const uchar *theElements);
	Color(const int *theElements);

	int GetRed() const;
	int GetGreen() const;
	int GetBlue() const;
	int GetAlpha() const;
	ulong ToInt() const;
	ARGB ToRGBA() const;//extra pixel formats in case if certain images load weird with weird pixel formats.
	ARGB ToBGRA() const;
	YUV ToYUV() const;
	int &operator[](int theIdx);
	int operator[](int theIdx) const;
};

bool operator==(const Color &theColor1, const Color &theColor2);
bool operator!=(const Color &theColor1, const Color &theColor2);

} // namespace PopLib

#endif

