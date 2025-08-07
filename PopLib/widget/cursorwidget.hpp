#ifndef __CURSORWIDGET_HPP__
#define __CURSORWIDGET_HPP__

#pragma once

#include "widget.hpp"
#include "math/point.hpp"

namespace PopLib
{

class Image;

class CursorWidget : public Widget
{
  public:
	Image *mImage;

  public:
	CursorWidget();

	virtual void Draw(Graphics *g);
	void SetImage(Image *theImage);
	Point GetHotspot();
};

} // namespace PopLib

#endif // __CURSORWIDGET_HPP__
