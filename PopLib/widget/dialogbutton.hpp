#ifndef __DIALOGBUTTON_HPP__
#define __DIALOGBUTTON_HPP__

#pragma once

#include "buttonwidget.hpp"

namespace PopLib
{

class DialogButton : public ButtonWidget
{
  public:
	Image *mComponentImage;
	int mTranslateX, mTranslateY;
	int mTextOffsetX, mTextOffsetY;

  public:
	DialogButton(Image *theComponentImage, int theId, ButtonListener *theListener);

	virtual void Draw(Graphics *g);
};

} // namespace PopLib

#endif