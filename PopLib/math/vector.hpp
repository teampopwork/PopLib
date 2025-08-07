#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

#pragma once

#include "math.hpp"

namespace PopLib
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Vector2
{
  public:
	float x, y;

  public:
	Vector2() : x(0), y(0)
	{
	}
	Vector2(float theX, float theY) : x(theX), y(theY)
	{
	}

	float Dot(const Vector2 &v) const
	{
		return x * v.x + y * v.y;
	}
	Vector2 operator+(const Vector2 &v) const
	{
		return Vector2(x + v.x, y + v.y);
	}
	Vector2 operator-(const Vector2 &v) const
	{
		return Vector2(x - v.x, y - v.y);
	}
	Vector2 operator-() const
	{
		return Vector2(-x, -y);
	}
	Vector2 operator*(float t) const
	{
		return Vector2(t * x, t * y);
	}
	Vector2 operator/(float t) const
	{
		return Vector2(x / t, y / t);
	}
	void operator+=(const Vector2 &v)
	{
		x += v.x;
		y += v.y;
	}
	void operator-=(const Vector2 &v)
	{
		x -= v.x;
		y -= v.y;
	}
	void operator*=(float t)
	{
		x *= t;
		y *= t;
	}
	void operator/=(float t)
	{
		x /= t;
		y /= t;
	}

	bool operator==(const Vector2 &v)
	{
		return x == v.x && y == v.y;
	}
	bool operator!=(const Vector2 &v)
	{
		return x != v.x || y != v.y;
	}

	float Magnitude() const
	{
		return std::sqrt(x * x + y * y);
	}
	float MagnitudeSquared() const
	{
		return x * x + y * y;
	}

	Vector2 Normalize() const
	{
		float aMag = Magnitude();
		return aMag != 0 ? (*this) / aMag : *this;
	}

	Vector2 Perp() const
	{
		return Vector2(-y, x);
	}

	float Distance(const Vector2& a, const Vector2& b)
	{
		return std::sqrt(DistanceSquared(a,b));
	}
	float DistanceSquared(const Vector2& a, const Vector2& b)
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		return dx * dx + dy * dy;
	}

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Vector3
{
  public:
	float x, y, z;

  public:
	Vector3() : x(0), y(0), z(0)
	{
	}
	Vector3(float theX, float theY, float theZ) : x(theX), y(theY), z(theZ)
	{
	}

	float Dot(const Vector3 &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}
	Vector3 Cross(const Vector3 &v) const
	{
		return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
	Vector3 operator+(const Vector3 &v) const
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}
	Vector3 operator-(const Vector3 &v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}
	Vector3 operator*(float t) const
	{
		return Vector3(t * x, t * y, t * z);
	}
	Vector3 operator/(float t) const
	{
		return Vector3(x / t, y / t, z / t);
	}
	float Magnitude() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Vector3 Normalize() const
	{
		float aMag = Magnitude();
		return aMag != 0 ? (*this) / aMag : *this;
	}

	float Distance(const Vector3& a, const Vector3& b)
	{
		return std::sqrt(DistanceSquared(a,b));
	}
	float DistanceSquared(const Vector3& a, const Vector3& b)
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		float dz = a.z - b.z;
		return dx * dx + dy * dy + dz * dz;
	}
};

}; // namespace PopLib

#endif