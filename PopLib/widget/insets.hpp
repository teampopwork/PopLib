#ifndef __INSETS_HPP__
#define __INSETS_HPP__

#pragma once

namespace PopLib
{

class Insets
{
  public:
	int mLeft;
	int mTop;
	int mRight;
	int mBottom;

  public:
	Insets();
	Insets(int theLeft, int theTop, int theRight, int theBottom);
	Insets(const Insets &theInsets);
};

} // namespace PopLib

#endif