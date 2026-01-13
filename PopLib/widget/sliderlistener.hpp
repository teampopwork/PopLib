#ifndef __SLIDERLISTENER_HPP__
#define __SLIDERLISTENER_HPP__

#pragma once

namespace PopLib
{

class SliderListener
{
  public:
	virtual void SliderVal(int theId, double theVal){};
};

} // namespace PopLib

#endif
