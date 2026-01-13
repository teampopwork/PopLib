#ifndef __FONT_HPP__
#define __FONT_HPP__

#pragma once

#include "common.hpp"
#include "math/rect.hpp"
#include "color.hpp"

namespace PopLib
{

class Graphics;

class Font
{
  public:
	int mAscent;
	int mAscentPadding; // How much space is above the avg uppercase char
	int mHeight;
	int mLineSpacingOffset; // This plus height should get added between lines

  public:
	Font();
	Font(const Font &theFont);
	virtual ~Font();

	virtual int GetAscent();
	virtual int GetAscentPadding();
	virtual int GetDescent();
	virtual int GetHeight();
	virtual int GetLineSpacingOffset();
	virtual int GetLineSpacing();
	virtual int StringWidth(const PopString &theString);
	virtual int CharWidth(PopChar theChar);
	virtual int CharWidthKern(PopChar theChar, PopChar thePrevChar);

	virtual void DrawString(Graphics *g, int theX, int theY, const PopString &theString, const Color &theColor,
							const Rect &theClipRect);

	virtual Font *Duplicate() {return new Font(*this);};
};

} // namespace PopLib

#endif