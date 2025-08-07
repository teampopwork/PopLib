#ifndef __POINT_HPP__
#define __POINT_HPP__

#pragma once

#include "common.hpp"

namespace PopLib
{

template <class _T> class TPoint
{
  public:
	_T mX;
	_T mY;

  public:
	TPoint(_T theX, _T theY) : mX(theX), mY(theY)
	{
	}

	TPoint(const TPoint<_T> &theTPoint) : mX(theTPoint.mX), mY(theTPoint.mY)
	{
	}

	TPoint() : mX(0), mY(0)
	{
	}

	inline bool operator==(const TPoint &p)
	{
		return ((p.mX == mX) && (p.mY == mY));
	}

	inline bool operator!=(const TPoint &p)
	{
		return ((p.mX != mX) || (p.mY != mY));
	}

	TPoint operator+(const TPoint &p) const
	{
		return TPoint(mX + p.mX, mY + p.mY);
	}
	TPoint operator-(const TPoint &p) const
	{
		return TPoint(mX - p.mX, mY - p.mY);
	}
	TPoint operator*(const TPoint &p) const
	{
		return TPoint(mX * p.mX, mY * p.mY);
	}
	TPoint operator/(const TPoint &p) const
	{
		return TPoint(mX / p.mX, mY / p.mY);
	}
	TPoint &operator+=(const TPoint &p)
	{
		mX += p.mX;
		mY += p.mY;
		return *this;
	}
	TPoint &operator-=(const TPoint &p)
	{
		mX -= p.mX;
		mY -= p.mY;
		return *this;
	}
	TPoint &operator*=(const TPoint &p)
	{
		mX *= p.mX;
		mY *= p.mY;
		return *this;
	}
	TPoint &operator/=(const TPoint &p)
	{
		mX /= p.mX;
		mY /= p.mY;
		return *this;
	}
	TPoint operator*(_T s) const
	{
		return TPoint(mX * s, mY * s);
	}
	TPoint operator/(_T s) const
	{
		return TPoint(mX / s, mY / s);
	}
};

typedef TPoint<int> Point;
typedef TPoint<double> FPoint;

}; // namespace PopLib

#endif //__POINT_HPP__