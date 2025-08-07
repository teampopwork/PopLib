#ifndef __CHECKBOX_HPP__
#define __CHECKBOX_HPP__

#pragma once

#include "widget.hpp"

namespace PopLib
{

class CheckboxListener;
class Image;

class Checkbox : public Widget
{
  protected:
	CheckboxListener *mListener;

  public:
	int mId;

	bool mChecked;

	Image *mUncheckedImage;
	Image *mCheckedImage;

	Rect mCheckedRect;
	Rect mUncheckedRect;

	Color mOutlineColor; // These are only used if no image is specified
	Color mBkgColor;
	Color mCheckColor;

  public:
	virtual void SetChecked(bool checked, bool tellListener = true);
	virtual bool IsChecked();

	virtual void MouseDown(int x, int y, int theClickCount)
	{
		Widget::MouseDown(x, y, theClickCount);
	}
	virtual void MouseDown(int x, int y, int theBtnNum, int theClickCount);
	virtual void Draw(Graphics *g);

  public:
	Checkbox(Image *theUncheckedImage, Image *theCheckedImage, int theId, CheckboxListener *theCheckboxListener);
};

} // namespace PopLib

#endif