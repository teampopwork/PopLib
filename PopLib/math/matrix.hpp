#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

#pragma once

#include "vector.hpp"

namespace PopLib
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Matrix3
{
  public:
	union {
		float m[3][3];
		struct
		{
			float m00, m01, m02;
			float m10, m11, m12;
			float m20, m21, m22;
		};
	};

  public:
	Matrix3();
	void ZeroMatrix();
	void LoadIdentity();

	Vector2 operator*(const Vector2 &theVec) const;
	Vector3 operator*(const Vector3 &theVec) const;
	Matrix3 operator*(const Matrix3 &theMat) const;
	const Matrix3 &operator*=(const Matrix3 &theMat);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Transform2D : public Matrix3
{
	// only useful internally
  protected:
	double determinantOfMinor(int theRowHeightY, int theColumnWidthX) const;

  public:
	Transform2D();
	Transform2D(bool loadIdentity);
	Transform2D(const Matrix3 &theMatrix);

	const Transform2D &operator=(const Matrix3 &theMat);

	double GetDeterminant() const;
	Transform2D Inverse() const;

	void Translate(float tx, float ty);

	// Rotate has been replaced by RotateRad.
	// NOTE:  If you had Rotate(angle) you should now use RotateRad(-angle).
	// This is to make positive rotations go counter-clockwise when using screen coordinates.
	void RotateRad(float rot);
	void RotateDeg(float rot);
	void Scale(float sx, float sy);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Transform
{
  protected:
	mutable Transform2D mMatrix;
	mutable bool mNeedCalcMatrix;
	void MakeComplex();
	void CalcMatrix() const;

  public:
	bool mComplex, mHaveRot, mHaveScale;
	float mTransX1, mTransY1, mTransX2, mTransY2;
	float mScaleX, mScaleY;
	float mRot;

  public:
	Transform();

	void Reset();

	void Translate(float tx, float ty);
	void RotateRad(float rot);
	void RotateDeg(float rot);
	void Scale(float sx, float sy);

	const Transform2D &GetMatrix() const;
};

} // namespace PopLib

#endif