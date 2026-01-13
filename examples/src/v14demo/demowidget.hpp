#ifndef __DEMO_WIDGET_HPP__
#define __DEMO_WIDGET_HPP__

#pragma once

#include "PopLib/widget/widget.hpp"
#include "PopLib/widget/buttonlistener.hpp"

namespace PopLib
{

class ButtonWidget;

//////////////////////////////////////////////////////////////////////////
//	Noting new in this header. Check the previous demos if this makes
//	no sense to you, as it's all covered there. CHECK OUT THE .CPP FILE
//	THOUGH, there's new stuff there.
//////////////////////////////////////////////////////////////////////////

class DemoWidget : public Widget, public ButtonListener
{

  private:
	ButtonWidget *mMoveButton;
	ButtonWidget *mCloseButton;

  public:
	DemoWidget();
	virtual ~DemoWidget();

	void Draw(Graphics *g);
	void ButtonDepress(int id);
};

} // namespace PopLib

#endif //__DEMO_WIDGET_HPP__
