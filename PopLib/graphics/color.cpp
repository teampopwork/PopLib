#include "color.hpp"
#include <algorithm>

#define CLIP(X) std::clamp((X), 0, 255)//for YUV conversion

using namespace PopLib;

Color Color::Black(0, 0, 0);
Color Color::White(255, 255, 255);
Color Color::Red (255, 0, 0);
Color Color::Lime (0, 255, 0);
Color Color::Blue (0, 0, 255);
Color Color::Cyan (0, 255, 255);
Color Color::Magenta(255, 0, 255);
Color Color::Yellow (255, 255, 0);
Color Color::SkyBlue (0, 160, 255);
Color Color::Brown (128, 64, 0);
Color Color::Orange(255, 128, 0);
Color Color::Green (0, 128, 0);
Color Color::SpringGreen(112, 224, 0);
Color Color::Lavender(224, 200, 255);
Color Color::Gold (255, 200, 0);
Color Color::Purple (128, 0, 128);
Color Color::Navy (0, 0, 128);
Color Color::Gray (128, 128, 128);
Color Color::Silver (224, 224, 224);
Color Color::Pink (255, 192, 224);
Color Color::Scarlet(255, 64, 0);

Color::Color() : mRed(0), mGreen(0), mBlue(0), mAlpha(255)
{
}

Color::Color(int theColor)
	: mAlpha((theColor >> 24) & 0xFF), mRed((theColor >> 16) & 0xFF), mGreen((theColor >> 8) & 0xFF),
	Blue((theColor) & 0xFF)
{
	if (mBlue == 0)
		mBlue = 0xff;
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

Color::Color(const ARGB &theColor) : mAlpha(theColor.a), mRed(theColor.r), mGreen(theColor.g), mBlue(theColor.b)
{
}

Color::Color(const uchar *theElements)
	: mAlpha(theElements[0]), mRed(theElements[1]), mGreen(theElements[2]), mBlue(0xFF)
{
}

Color::Color(const int *theElements) : mAlpha(theElements[0]), mRed(theElements[1]), mGreen(theElements[2]), mBlue(0xFF)
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
		return mAlpha;
	case 1:
		return mRed;
	case 2:
		return mGreen;
	case 3:
		return mBlue;
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
	return (mAlpha << 24) | (mRed << 16) | (mGreen << 8) | (mBlue);
}

ARGB Color::ToBGRA() const
{

	//Swap the values when your doing BGRA, NOT ARGB!  This function converts it from ARGB to BGRA.
	ARGB TheBGRAValue;
	TheBGRAValue.b = mAlpha;
	TheBGRAValue.g = mRed;
	TheBGRAValue.r = mGreen;
	TheBGRAValue.a = mBlue;

	return TheBGRAValue;
}
YUV Color::ToYUV() const
{
	YUV yuv;
	//YUV conversion here, uses std::clamp to convert hue sat and lum in the RGB space.  
	yuv.y = CLIP(((66 * mRed + 129 * mGreen + 25 * mBlue + 128) >> 8) + 16);
	yuv.u = CLIP(((-38 * mRed - 74 * mGreen + 112 * mBlue + 128) >> 8) + 128);
	yuv.v = CLIP(((112 * mRed - 94 * mGreen - 18 * mBlue + 128) >> 8) + 128);
	return yuv;



}

bool PopLib::operator==(const Color &theColor1, const Color &theColor2)
{
	return (theColor1.mAlpha == theColor2.mAlpha) && (theColor1.mRed == theColor2.mRed) &&
		   (theColor1.mGreen == theColor2.mGreen) && (theColor1.mBlue == theColor2.mBlue);
}

bool PopLib::operator!=(const Color &theColor1, const Color &theColor2)
{
	return (theColor1.mAlpha != theColor2.mAlpha) || (theColor1.mRed != theColor2.mRed) ||
		   (theColor1.mGreen != theColor2.mGreen) || (theColor1.mBlue != theColor2.mBlue);
}
