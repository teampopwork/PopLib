#include "color.hpp"
#include <algorithm>

#define CLIP(X) std::clamp((X), 0, 255)//for YUV conversion

using namespace PopLib;

Color Color::Black(0, 0, 0);
Color Color::White(255, 255, 255);

Color::Color() : mRed(0), mGreen(0), mBlue(0), mAlpha(255)
{
}

Color::Color(int theColor)
	: mRed((theColor >> 24) & 0xFF), mGreen((theColor >> 16) & 0xFF), mBlue((theColor >> 8) & 0xFF),
	  mAlpha((theColor) & 0xFF)
{
	if (mAlpha == 0)
		mAlpha = 0xff;
}

Color::Color(int theColor, int theAlpha)
	: mRed((theColor >> 16) & 0xFF), mGreen((theColor >> 8) & 0xFF), mBlue((theColor) & 0xFF), mAlpha(theAlpha)
{
}

Color::Color(int theRed, int theGreen, int theBlue) : mRed(theRed), mGreen(theGreen), mBlue(theBlue), mAlpha(0xFF)
{
}

Color::Color(int theRed, int theGreen, int theBlue, int theAlpha)
	: mRed(theRed), mGreen(theGreen), mBlue(theBlue), mAlpha(theAlpha)
{
}

Color::Color(const RGBA &theColor) : mRed(theColor.r), mGreen(theColor.g), mBlue(theColor.b), mAlpha(theColor.a)
{
}

Color::Color(const uchar *theElements)
	: mRed(theElements[0]), mGreen(theElements[1]), mBlue(theElements[2]), mAlpha(0xFF)
{
}

Color::Color(const int *theElements) : mRed(theElements[0]), mGreen(theElements[1]), mBlue(theElements[2]), mAlpha(0xFF)
{
}


int Color::GetRed() const
{
	return mRed;
}

int Color::GetGreen() const
{
	return mGreen;
}

int Color::GetBlue() const
{
	return mBlue;
}

int Color::GetAlpha() const
{
	return mAlpha;
}

int &Color::operator[](int theIdx)
{
	static int aJunk = 0;

	switch (theIdx)
	{
	case 0:
		return mRed;
	case 1:
		return mGreen;
	case 2:
		return mBlue;
	case 3:
		return mAlpha;
	default:
		return aJunk;
	}
}

int Color::operator[](int theIdx) const
{
	switch (theIdx)
	{
	case 0:
		return mRed;
	case 1:
		return mGreen;
	case 2:
		return mBlue;
	case 3:
		return mAlpha;
	default:
		return 0;
	}
}

ulong Color::ToInt() const
{
	return (mRed << 24) | (mGreen << 16) | (mBlue << 8) | (mAlpha);
}

RGBA Color::ToBGRA() const
{

	//Swap the values when your doing BGRA, NOT RGBA!  This function converts it from RGBA to BGRA.
	RGBA TheBGRAValue;
	TheBGRAValue.b = mRed;
	TheBGRAValue.g = mBlue;
	TheBGRAValue.r = mGreen;
	TheBGRAValue.a = mAlpha;

	return TheBGRAValue;
}
YUV Color::ToYUV() const
{
	YUV yuv;
	//YUV conversion here, uses std::clamp to convert hue sat and lum in the RGBA space.  
	yuv.y = CLIP(((66 * mRed + 129 * mGreen + 25 * mBlue + 128) >> 8) + 16);
	yuv.u = CLIP(((-38 * mRed - 74 * mGreen + 112 * mBlue + 128) >> 8) + 128);
	yuv.v = CLIP(((112 * mRed - 94 * mGreen - 18 * mBlue + 128) >> 8) + 128);
	return yuv;



}

bool PopLib::operator==(const Color &theColor1, const Color &theColor2)
{
	return (theColor1.mRed == theColor2.mRed) && (theColor1.mGreen == theColor2.mGreen) &&
		   (theColor1.mBlue == theColor2.mBlue) && (theColor1.mAlpha == theColor2.mAlpha);
}

bool PopLib::operator!=(const Color &theColor1, const Color &theColor2)
{
	return (theColor1.mRed != theColor2.mRed) || (theColor1.mGreen != theColor2.mGreen) ||
		   (theColor1.mBlue != theColor2.mBlue) || (theColor1.mAlpha != theColor2.mAlpha);
}
