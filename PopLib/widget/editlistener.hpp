#ifndef __EDITLISTENER_HPP__
#define __EDITLISTENER_HPP__

#pragma once

#include "common.hpp"
#include "misc/keycodes.hpp"

namespace PopLib
{

class EditListener
{
  public:
	virtual void EditWidgetText(int theId, const PopString &theString){};

	virtual bool AllowKey(int theId, KeyCode theKey)
	{
		return true;
	}
	virtual bool AllowChar(int theId, PopChar theChar)
	{
		return true;
	}
	virtual bool AllowText(int theId, const PopString &theText)
	{
		return true;
	}
};

} // namespace PopLib

#endif