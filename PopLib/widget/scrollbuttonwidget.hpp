#ifndef __SCROLLBUTTONWIDGET_HPP__
#define __SCROLLBUTTONWIDGET_HPP__

#pragma once

#include "buttonwidget.hpp"

namespace PopLib
{

class ButtonListener;

class ScrollbuttonWidget : public ButtonWidget
{
  public:
	bool mHorizontal;

	// mType is a new way of doing things (1 = up, 2 = down, 3 = left, 4 = right).
	// This overrides mHorizontal and mId.
	int mType;

  public:
	ScrollbuttonWidget(int theId, ButtonListener *theButtonListener, int theType = 0);
	virtual ~ScrollbuttonWidget();

	void Draw(Graphics *g);
};

} // namespace PopLib

#endif // __SCROLLBUTTONWIDGET_HPP__