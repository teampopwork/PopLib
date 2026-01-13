#ifndef __HYPERLINKWIDGET_HPP__
#define __HYPERLINKWIDGET_HPP__

#pragma once

#include "buttonwidget.hpp"

namespace PopLib
{

class HyperlinkWidget : public ButtonWidget
{
  public:
	Color mColor;
	Color mOverColor;
	int mUnderlineSize;
	int mUnderlineOffset;

  public:
	HyperlinkWidget(int theId, ButtonListener *theButtonListener);
	virtual ~HyperlinkWidget(){};

	void Draw(Graphics *g);
	void MouseEnter();
	void MouseLeave();
};

} // namespace PopLib

#endif