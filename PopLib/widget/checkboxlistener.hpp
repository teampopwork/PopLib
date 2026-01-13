#ifndef __CHECKBOXLISTENER_HPP__
#define __CHECKBOXLISTENER_HPP__

#pragma once

namespace PopLib
{

class CheckboxListener
{
  public:
	virtual void CheckboxChecked(int theId, bool checked)
	{
	}
};

} // namespace PopLib

#endif