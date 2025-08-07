#ifndef __DIALOGLISTENER_HPP__
#define __DIALOGLISTENER_HPP__

#pragma once

namespace PopLib
{

class DialogListener
{
  public:
	virtual void DialogButtonPress(int theDialogId, int theButtonId)
	{
	}
	virtual void DialogButtonDepress(int theDialogId, int theButtonId)
	{
	}
};

} // namespace PopLib

#endif