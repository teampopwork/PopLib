#include "memoryimage.hpp"

#include "appbase.hpp"
#include "graphics.hpp"
#include "nativedisplay.hpp"
#include "sdlinterface.hpp"
#include "debug/debug.hpp"
#include "quantize.hpp"
#include "debug/perftimer.hpp"
#include "SWTri/SWTri.hpp"

#include <math.h>

using namespace PopLib;

#ifdef OPTIMIZE_SOFTWARE_DRAWING
bool gOptimizeSoftwareDrawing = false;
#endif

// Disable macro redefinition warning
#pragma warning(disable : 4005)

MemoryImage::MemoryImage()
{
	mApp = gAppBase;

	Init();
}

MemoryImage::MemoryImage(AppBase *theApp)
{
	mApp = theApp;
	Init();
}

MemoryImage::MemoryImage(const MemoryImage &theMemoryImage)
	: Image(theMemoryImage), mApp(theMemoryImage.mApp), mHasAlpha(theMemoryImage.mHasAlpha),
	  mHasTrans(theMemoryImage.mHasTrans), mBitsChanged(theMemoryImage.mBitsChanged),
	  mIsVolatile(theMemoryImage.mIsVolatile), mPurgeBits(theMemoryImage.mPurgeBits), mWantPal(theMemoryImage.mWantPal),
	  mImageFlags(theMemoryImage.mImageFlags), mBitsChangedCount(theMemoryImage.mBitsChangedCount), mD3DData(nullptr)
{
	bool deleteBits = false;

	MemoryImage *aNonConstMemoryImage = (MemoryImage *)&theMemoryImage;

	if ((theMemoryImage.mBits == nullptr) && (theMemoryImage.mColorTable == nullptr))
	{
		// Must be a SDLImage with only a DDSurface
		aNonConstMemoryImage->GetBits();
		deleteBits = true;
	}

	if (theMemoryImage.mBits != nullptr)
	{
		mBits = new ulong[mWidth * mHeight + 1];
		mBits[mWidth * mHeight] = MEMORYCHECK_ID;
		memcpy(mBits, theMemoryImage.mBits, (mWidth * mHeight + 1) * sizeof(ulong));
	}
	else
		mBits = nullptr;

	if (deleteBits)
	{
		// Remove the temporary source bits
		delete[] aNonConstMemoryImage->mBits;
		aNonConstMemoryImage->mBits = nullptr;
	}

	if (theMemoryImage.mColorTable != nullptr)
	{
		mColorTable = new ulong[256];
		memcpy(mColorTable, theMemoryImage.mColorTable, 256 * sizeof(ulong));
	}
	else
		mColorTable = nullptr;

	if (theMemoryImage.mColorIndices != nullptr)
	{
		mColorIndices = new uchar[mWidth * mHeight];
		memcpy(mColorIndices, theMemoryImage.mColorIndices, mWidth * mHeight * sizeof(uchar));
	}
	else
		mColorIndices = nullptr;

	if (theMemoryImage.mNativeAlphaData != nullptr)
	{
		if (theMemoryImage.mColorTable == nullptr)
		{
			mNativeAlphaData = new ulong[mWidth * mHeight];
			memcpy(mNativeAlphaData, theMemoryImage.mNativeAlphaData, mWidth * mHeight * sizeof(ulong));
		}
		else
		{
			mNativeAlphaData = new ulong[256];
			memcpy(mNativeAlphaData, theMemoryImage.mNativeAlphaData, 256 * sizeof(ulong));
		}
	}
	else
		mNativeAlphaData = nullptr;

	if (theMemoryImage.mRLAlphaData != nullptr)
	{
		mRLAlphaData = new uchar[mWidth * mHeight];
		memcpy(mRLAlphaData, theMemoryImage.mRLAlphaData, mWidth * mHeight);
	}
	else
		mRLAlphaData = nullptr;

	if (theMemoryImage.mRLAdditiveData != nullptr)
	{
		mRLAdditiveData = new uchar[mWidth * mHeight];
		memcpy(mRLAdditiveData, theMemoryImage.mRLAdditiveData, mWidth * mHeight);
	}
	else
		mRLAdditiveData = nullptr;

	mApp->AddMemoryImage(this);
}

MemoryImage::~MemoryImage()
{
	mApp->RemoveMemoryImage(this);

	delete[] mBits;
	delete[] mNativeAlphaData;
	delete[] mRLAlphaData;
	delete[] mRLAdditiveData;
	delete[] mColorIndices;
	delete[] mColorTable;
}

void MemoryImage::Init()
{
	mBits = nullptr;
	mColorTable = nullptr;
	mColorIndices = nullptr;

	mNativeAlphaData = nullptr;
	mRLAlphaData = nullptr;
	mRLAdditiveData = nullptr;
	mHasTrans = false;
	mHasAlpha = false;
	mBitsChanged = false;
	mForcedMode = false;
	mIsVolatile = false;

	mD3DData = nullptr;
	mImageFlags = 0;
	mBitsChangedCount = 0;

	mPurgeBits = false;
	mWantPal = false;

	mApp->AddMemoryImage(this);
}

void MemoryImage::BitsChanged()
{
	mBitsChanged = true;
	mBitsChangedCount++;

	delete[] mNativeAlphaData;
	mNativeAlphaData = nullptr;

	delete[] mRLAlphaData;
	mRLAlphaData = nullptr;

	delete[] mRLAdditiveData;
	mRLAdditiveData = nullptr;
}

void MemoryImage::NormalDrawLine(double theStartX, double theStartY, double theEndX, double theEndY,
								 const Color &theColor)
{
	double aMinX = std::min(theStartX, theEndX);
	double aMinY = std::min(theStartY, theEndY);
	double aMaxX = std::max(theStartX, theEndX);
	double aMaxY = std::max(theStartY, theEndY);

	ulong aRMask = 0xFF0000;
	ulong aGMask = 0x00FF00;
	ulong aBMask = 0x0000FF;
	ulong aRRoundAdd = aRMask >> 1;
	ulong aGRoundAdd = aGMask >> 1;
	ulong aBRoundAdd = aBMask >> 1;

	ulong *aSurface = GetBits();

	if (true) //(mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		if (theColor.mAlpha == 255)
		{
			ulong aColor = 0xFF000000 | ((((theColor.mRed * aRMask) + aRRoundAdd) >> 8) & aRMask) |
						   ((((theColor.mGreen * aGMask) + aGRoundAdd) >> 8) & aGMask) |
						   ((((theColor.mBlue * aBMask) + aBRoundAdd) >> 8) & aBMask);

			double dv = theEndY - theStartY;
			double dh = theEndX - theStartX;
			int minG, maxG, G, DeltaG1, DeltaG2;
			double swap;
			int inc = 1;
			int aCurX;
			int aCurY;
			int aRowWidth = mWidth;
			int aRowAdd = aRowWidth;
			;

			if (abs(dv) < abs(dh))
			{
				// Mostly horizontal
				if (dh < 0)
				{
					dh = -dh;
					dv = -dv;
					swap = theEndY;
					theEndY = theStartY;
					theStartY = swap;
					swap = theEndX;
					theEndX = theStartX;
					theStartX = swap;
				}
				if (dv < 0)
				{
					dv = -dv;
					inc = -1;
					aRowAdd = -aRowAdd;
				}

				ulong *aDestPixels = ((ulong *)aSurface) + ((int)theStartY * aRowWidth) + (int)theStartX;
				*aDestPixels = aColor;
				aDestPixels++;

				aCurY = theStartY;
				aCurX = theStartX + 1;

				G = 2 * dv - dh;
				DeltaG1 = 2 * (dv - dh);
				DeltaG2 = 2 * dv;

				G += DeltaG2 * (theStartY - (int)theStartY);

				while (aCurX <= theEndX)
				{
					if (G > 0)
					{
						G += DeltaG1;
						aCurY += inc;
						aDestPixels += aRowAdd;

						if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
							break;
					}
					else
						G += DeltaG2;

					*aDestPixels = aColor;

					aCurX++;
					aDestPixels++;
				}
			}
			else
			{
				// Mostly vertical
				if (dv < 0)
				{
					dh = -dh;
					dv = -dv;
					swap = theEndY;
					theEndY = theStartY;
					theStartY = swap;
					swap = theEndX;
					theEndX = theStartX;
					theStartX = swap;
				}

				if (dh < 0)
				{
					dh = -dh;
					inc = -1;
				}

				ulong *aDestPixels = ((ulong *)aSurface) + ((int)theStartY * aRowWidth) + (int)theStartX;
				*aDestPixels = aColor;
				aDestPixels += aRowAdd;

				aCurX = theStartX;
				aCurY = theStartY + 1;

				G = 2 * dh - dv;
				minG = maxG = G;
				DeltaG1 = 2 * (dh - dv);
				DeltaG2 = 2 * dh;

				G += DeltaG2 * (theStartX - (int)theStartX);

				while (aCurY <= theEndY)
				{
					if (G > 0)
					{
						G += DeltaG1;
						aCurX += inc;
						aDestPixels += inc;

						if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
							break;
					}
					else
						G += DeltaG2;

					*aDestPixels = aColor;

					aCurY++;
					aDestPixels += aRowAdd;
				}
			}
		}
		else
		{
			ulong src = 0xFF000000 |
						((((((theColor.mRed * theColor.mAlpha + 0x80) >> 8) * aRMask) + aRRoundAdd) >> 8) & aRMask) |
						((((((theColor.mGreen * theColor.mAlpha + 0x80) >> 8) * aGMask) + aGRoundAdd) >> 8) & aGMask) |
						((((((theColor.mBlue * theColor.mAlpha + 0x80) >> 8) * aBMask) + aBRoundAdd) >> 8) & aBMask);
			int oma = 256 - theColor.mAlpha;

			double dv = theEndY - theStartY;
			double dh = theEndX - theStartX;
			int minG, maxG, G, DeltaG1, DeltaG2;
			double swap;
			int inc = 1;
			int aCurX;
			int aCurY;
			int aRowWidth = mWidth;
			int aRowAdd = aRowWidth;

			if (abs(dv) < abs(dh))
			{
				// Mostly horizontal
				if (dh < 0)
				{
					dh = -dh;
					dv = -dv;
					swap = theEndY;
					theEndY = theStartY;
					theStartY = swap;
					swap = theEndX;
					theEndX = theStartX;
					theStartX = swap;
				}
				if (dv < 0)
				{
					dv = -dv;
					inc = -1;
					aRowAdd = -aRowAdd;
				}

				ulong *aDestPixels = ((ulong *)aSurface) + ((int)theStartY * aRowWidth) + (int)theStartX;
				ulong dest = *aDestPixels;
				*(aDestPixels++) = src + (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
								   (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
								   (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

				aCurY = theStartY;
				aCurX = theStartX + 1;

				G = 2 * dv - dh;
				DeltaG1 = 2 * (dv - dh);
				DeltaG2 = 2 * dv;

				G += DeltaG2 * (theStartX - (int)theStartX);

				while (aCurX <= theEndX)
				{
					if (G > 0)
					{
						G += DeltaG1;
						aCurY += inc;
						aDestPixels += aRowAdd;

						if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
							break;
					}
					else
						G += DeltaG2;

					dest = *aDestPixels;
					*(aDestPixels++) = src + (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
									   (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
									   (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

					aCurX++;
				}
			}
			else
			{
				// Mostly vertical
				if (dv < 0)
				{
					dh = -dh;
					dv = -dv;
					swap = theEndY;
					theEndY = theStartY;
					theStartY = swap;
					swap = theEndX;
					theEndX = theStartX;
					theStartX = swap;
				}

				if (dh < 0)
				{
					dh = -dh;
					inc = -1;
				}

				ulong *aDestPixels = ((ulong *)aSurface) + ((int)theStartY * aRowWidth) + (int)theStartX;
				ulong dest = *aDestPixels;
				*aDestPixels = src + (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
							   (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
							   (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);
				aDestPixels += aRowAdd;

				aCurX = theStartX;
				aCurY = theStartY + 1;

				G = 2 * dh - dv;
				minG = maxG = G;
				DeltaG1 = 2 * (dh - dv);
				DeltaG2 = 2 * dh;

				G += DeltaG2 * (theStartX - (int)theStartX);

				while (aCurY <= theEndY)
				{
					if (G > 0)
					{
						G += DeltaG1;
						aCurX += inc;
						aDestPixels += inc;

						if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
							break;
					}
					else
						G += DeltaG2;

					dest = *aDestPixels;
					*aDestPixels = src + (((((dest & aRMask) * oma) + aRRoundAdd) >> 8) & aRMask) +
								   (((((dest & aGMask) * oma) + aGRoundAdd) >> 8) & aGMask) +
								   (((((dest & aBMask) * oma) + aBRoundAdd) >> 8) & aBMask);

					aCurY++;
					aDestPixels += aRowAdd;
				}
			}
		}
	}
}

void MemoryImage::AdditiveDrawLine(double theStartX, double theStartY, double theEndX, double theEndY,
								   const Color &theColor)
{
	double aMinX = std::min(theStartX, theEndX);
	double aMinY = std::min(theStartY, theEndY);
	double aMaxX = std::max(theStartX, theEndX);
	double aMaxY = std::max(theStartY, theEndY);

	ulong aRMask = 0xFF0000;
	ulong aGMask = 0x00FF00;
	ulong aBMask = 0x0000FF;
	int aRedShift = 16;
	int aGreenShift = 8;
	int aBlueShift = 0;

	ulong aRRoundAdd = aRMask >> 1;
	ulong aGRoundAdd = aGMask >> 1;
	ulong aBRoundAdd = aBMask >> 1;

	uchar *aMaxTable = mApp->mAdd8BitMaxTable;
	ulong *aSurface = GetBits();

	if (true) //(mLockedSurfaceDesc.ddpfPixelFormat.dwRGBBitCount == 32)
	{
		ulong rc = ((theColor.mRed * theColor.mAlpha) / 255);
		ulong gc = ((theColor.mGreen * theColor.mAlpha) / 255);
		ulong bc = ((theColor.mBlue * theColor.mAlpha) / 255);

		double dv = theEndY - theStartY;
		double dh = theEndX - theStartX;
		int minG, maxG, G, DeltaG1, DeltaG2;
		double swap;
		int inc = 1;
		int aCurX;
		int aCurY;
		int aRowWidth = mWidth;
		int aRowAdd = aRowWidth;

		if (abs(dv) < abs(dh))
		{
			// Mostly horizontal
			if (dh < 0)
			{
				dh = -dh;
				dv = -dv;
				swap = theEndY;
				theEndY = theStartY;
				theStartY = swap;
				swap = theEndX;
				theEndX = theStartX;
				theStartX = swap;
			}

			if (dv < 0)
			{
				dv = -dv;
				inc = -1;
				aRowAdd = -aRowAdd;
			}

			ulong *aDestPixels = ((ulong *)aSurface) + ((int)theStartY * aRowWidth) + (int)theStartX;
			ulong dest = *aDestPixels;

			int r = aMaxTable[((dest & aRMask) >> aRedShift) + rc];
			int g = aMaxTable[((dest & aGMask) >> aGreenShift) + gc];
			int b = aMaxTable[((dest & aBMask) >> aBlueShift) + bc];

			*(aDestPixels++) = 0xFF000000 | (r << aRedShift) | (g << aGreenShift) | (b << aBlueShift);

			aCurY = theStartY;
			aCurX = theStartX + 1;

			G = 2 * dv - dh;
			DeltaG1 = 2 * (dv - dh);
			DeltaG2 = 2 * dv;

			while (aCurX <= theEndX)
			{
				if (G > 0)
				{
					G += DeltaG1;
					aCurY += inc;
					aDestPixels += aRowAdd;

					if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
						break;
				}
				else
					G += DeltaG2;

				dest = *aDestPixels;

				r = aMaxTable[((dest & aRMask) >> aRedShift) + rc];
				g = aMaxTable[((dest & aGMask) >> aGreenShift) + gc];
				b = aMaxTable[((dest & aBMask) >> aBlueShift) + bc];

				*(aDestPixels++) = 0xFF000000 | (r << aRedShift) | (g << aGreenShift) | (b << aBlueShift);

				aCurX++;
			}
		}
		else
		{
			// Mostly vertical
			if (dv < 0)
			{
				dh = -dh;
				dv = -dv;
				swap = theEndY;
				theEndY = theStartY;
				theStartY = swap;
				swap = theEndX;
				theEndX = theStartX;
				theStartX = swap;
			}

			if (dh < 0)
			{
				dh = -dh;
				inc = -1;
			}

			ulong *aDestPixels = ((ulong *)aSurface) + ((int)theStartY * mWidth) + (int)theStartX;

			ulong dest = *aDestPixels;

			int r = aMaxTable[((dest & aRMask) >> aRedShift) + rc];
			int g = aMaxTable[((dest & aGMask) >> aGreenShift) + gc];
			int b = aMaxTable[((dest & aBMask) >> aBlueShift) + bc];

			*aDestPixels = 0xFF000000 | (r << aRedShift) | (g << aGreenShift) | (b << aBlueShift);

			aDestPixels += aRowAdd;

			aCurX = theStartX;
			aCurY = theStartY + 1;

			G = 2 * dh - dv;
			minG = maxG = G;
			DeltaG1 = 2 * (dh - dv);
			DeltaG2 = 2 * dh;
			while (aCurY <= theEndY)
			{
				if (G > 0)
				{
					G += DeltaG1;
					aCurX += inc;
					aDestPixels += inc;

					if (aCurX < aMinX || aCurY < aMinY || aCurX > aMaxX || aCurY > aMaxY)
						break;
				}
				else
					G += DeltaG2;

				dest = *aDestPixels;

				r = aMaxTable[((dest & aRMask) >> aRedShift) + rc];
				g = aMaxTable[((dest & aGMask) >> aGreenShift) + gc];
				b = aMaxTable[((dest & aBMask) >> aBlueShift) + bc];

				*aDestPixels = 0xFF000000 | (r << aRedShift) | (g << aGreenShift) | (b << aBlueShift);

				aCurY++;
				aDestPixels += aRowAdd;
			}
		}
	}
}

void MemoryImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor,
						   int theDrawMode)
{
	if (theStartY == theEndY)
	{
		int aStartX = std::min(theStartX, theEndX);
		int aEndX = std::max(theStartX, theEndX);

		FillRect(Rect(aStartX, theStartY, aEndX - aStartX + 1, theEndY - theStartY + 1), theColor, theDrawMode);
		return;
	}
	else if (theStartX == theEndX)
	{
		int aStartY = std::min(theStartY, theEndY);
		int aEndY = std::max(theStartY, theEndY);

		FillRect(Rect(theStartX, aStartY, theEndX - theStartX + 1, aEndY - aStartY + 1), theColor, theDrawMode);
		return;
	}

	switch (theDrawMode)
	{
	case Graphics::DRAWMODE_NORMAL:
		NormalDrawLine(theStartX, theStartY, theEndX, theEndY, theColor);
		break;
	case Graphics::DRAWMODE_ADDITIVE:
		AdditiveDrawLine(theStartX, theStartY, theEndX, theEndY, theColor);
		break;
	}

	BitsChanged();
}

void MemoryImage::NormalDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY,
								   const Color &theColor)
{
	ulong *aBits = GetBits();
	ulong color = theColor.ToInt();

	int aX0 = (int)theStartX, aX1 = (int)theEndX;
	int aY0 = (int)theStartY, aY1 = (int)theEndY;
	int aXinc = 1;
	if (aY0 > aY1)
	{
		int aTempX = aX0, aTempY = aY0;
		aX0 = aX1;
		aY0 = aY1;
		aX1 = aTempX;
		aY1 = aTempY;
		double aTempXd = theStartX, aTempYd = theStartY;
		theStartX = theEndX;
		theStartY = theEndY;
		theEndX = aTempXd;
		theEndY = aTempYd;
	}

	int dx = aX1 - aX0;
	int dy = aY1 - aY0;
	double dxd = theEndX - theStartX;
	double dyd = theEndY - theStartY;
	if (dx < 0)
	{
		dx = -dx;
		aXinc = -1;
		dxd = -dxd;
	}

	if (theColor.mAlpha != 255)
	{
#define PIXEL_TYPE ulong
#define CALC_WEIGHT_A(w) (((w) * (theColor.mAlpha + 1)) >> 8)
#define BLEND_PIXEL(p)                                                                                                 \
	{                                                                                                                  \
		int aDestAlpha = dest >> 24;                                                                                   \
		int aNewDestAlpha = aDestAlpha + ((255 - aDestAlpha) * a) / 255;                                               \
		a = 255 * a / aNewDestAlpha;                                                                                   \
		oma = 256 - a;                                                                                                 \
		*(p) = (aNewDestAlpha << 24) | ((((color & 0xFF0000) * a + (dest & 0xFF0000) * oma) >> 8) & 0xFF0000) |        \
			   ((((color & 0x00FF00) * a + (dest & 0x00FF00) * oma) >> 8) & 0x00FF00) |                                \
			   ((((color & 0x0000FF) * a + (dest & 0x0000FF) * oma) >> 8) & 0x0000FF);                                 \
	}
		const int STRIDE = mWidth;

#include "Inc/GENERIC_DrawLineAA.inc"

#undef PIXEL_TYPE
#undef CALC_WEIGHT_A
#undef BLEND_PIXEL
	}
	else
	{
#define PIXEL_TYPE ulong
#define CALC_WEIGHT_A(w) (w)
#define BLEND_PIXEL(p)                                                                                                 \
	{                                                                                                                  \
		int aDestAlpha = dest >> 24;                                                                                   \
		int aNewDestAlpha = aDestAlpha + ((255 - aDestAlpha) * a) / 255;                                               \
		a = 255 * a / aNewDestAlpha;                                                                                   \
		oma = 256 - a;                                                                                                 \
		*(p) = (aNewDestAlpha << 24) | ((((color & 0xFF0000) * a + (dest & 0xFF0000) * oma) >> 8) & 0xFF0000) |        \
			   ((((color & 0x00FF00) * a + (dest & 0x00FF00) * oma) >> 8) & 0x00FF00) |                                \
			   ((((color & 0x0000FF) * a + (dest & 0x0000FF) * oma) >> 8) & 0x0000FF);                                 \
	}
		const int STRIDE = mWidth;

#include "Inc/GENERIC_DrawLineAA.inc"

#undef PIXEL_TYPE
#undef CALC_WEIGHT_A
#undef BLEND_PIXEL
	}

	BitsChanged();
}

void MemoryImage::AdditiveDrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY,
									 const Color &theColor)
{
}

void MemoryImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor,
							 int theDrawMode)
{
	if (theStartY == theEndY)
	{
		int aStartX = std::min(theStartX, theEndX);
		int aEndX = std::max(theStartX, theEndX);

		FillRect(Rect(aStartX, theStartY, aEndX - aStartX + 1, theEndY - theStartY + 1), theColor, theDrawMode);
		return;
	}
	else if (theStartX == theEndX)
	{
		int aStartY = std::min(theStartY, theEndY);
		int aEndY = std::max(theStartY, theEndY);

		FillRect(Rect(theStartX, aStartY, theEndX - theStartX + 1, aEndY - aStartY + 1), theColor, theDrawMode);
		return;
	}

	switch (theDrawMode)
	{
	case Graphics::DRAWMODE_NORMAL:
		NormalDrawLineAA(theStartX, theStartY, theEndX, theEndY, theColor);
		break;
	case Graphics::DRAWMODE_ADDITIVE:
		AdditiveDrawLineAA(theStartX, theStartY, theEndX, theEndY, theColor);
		break;
	}

	BitsChanged();
}

void MemoryImage::CommitBits()
{
	// if (gDebug)
	//	mApp->CopyToClipboard("+MemoryImage::CommitBits");

	if ((mBitsChanged) && (!mForcedMode))
	{
		// Analyze
		if (mBits != nullptr)
		{
			mHasTrans = false;
			mHasAlpha = false;

			int aSize = mWidth * mHeight;
			ulong *ptr = mBits;

			for (int i = 0; i < aSize; i++)
			{
				uchar anAlpha = (uchar)(*ptr++ >> 24);

				if (anAlpha == 0)
					mHasTrans = true;
				else if (anAlpha != 255)
					mHasAlpha = true;
			}
		}
		else if (mColorTable != nullptr)
		{
			mHasTrans = false;
			mHasAlpha = false;

			int aSize = 256;
			ulong *ptr = mColorTable;

			for (int i = 0; i < aSize; i++)
			{
				uchar anAlpha = (uchar)(*ptr++ >> 24);

				if (anAlpha == 0)
					mHasTrans = true;
				else if (anAlpha != 255)
					mHasAlpha = true;
			}
		}
		else
		{
			mHasTrans = true;
			mHasAlpha = false;
		}

		mBitsChanged = false;
	}

	// if (gDebug)
	//	mApp->CopyToClipboard("-MemoryImage::CommitBits");
}

void MemoryImage::SetImageMode(bool hasTrans, bool hasAlpha)
{
	mForcedMode = true;
	mHasTrans = hasTrans;
	mHasAlpha = hasAlpha;
}

void MemoryImage::SetVolatile(bool isVolatile)
{
	mIsVolatile = isVolatile;
}

void *MemoryImage::GetNativeAlphaData(NativeDisplay *theDisplay)
{
	if (mNativeAlphaData != nullptr)
		return mNativeAlphaData;

	CommitBits();

	const int rRightShift = 16 + (8 - theDisplay->mRedBits);
	const int gRightShift = 8 + (8 - theDisplay->mGreenBits);
	const int bRightShift = 0 + (8 - theDisplay->mBlueBits);

	const int rLeftShift = theDisplay->mRedShift;
	const int gLeftShift = theDisplay->mGreenShift;
	const int bLeftShift = theDisplay->mBlueShift;

	const int rMask = theDisplay->mRedMask;
	const int gMask = theDisplay->mGreenMask;
	const int bMask = theDisplay->mBlueMask;

	if (mColorTable == nullptr)
	{
		ulong *aSrcPtr = GetBits();

		ulong *anAlphaData = new ulong[mWidth * mHeight];

		ulong *aDestPtr = anAlphaData;
		int aSize = mWidth * mHeight;
		for (int i = 0; i < aSize; i++)
		{
			ulong val = *(aSrcPtr++);

			int anAlpha = val >> 24;

			ulong r = ((val & 0xFF0000) * (anAlpha + 1)) >> 8;
			ulong g = ((val & 0x00FF00) * (anAlpha + 1)) >> 8;
			ulong b = ((val & 0x0000FF) * (anAlpha + 1)) >> 8;

			*(aDestPtr++) = (((r >> rRightShift) << rLeftShift) & rMask) |
							(((g >> gRightShift) << gLeftShift) & gMask) |
							(((b >> bRightShift) << bLeftShift) & bMask) | (anAlpha << 24);
		}

		mNativeAlphaData = anAlphaData;
	}
	else
	{
		ulong *aSrcPtr = mColorTable;

		ulong *anAlphaData = new ulong[256];

		for (int i = 0; i < 256; i++)
		{
			ulong val = *(aSrcPtr++);

			int anAlpha = val >> 24;

			ulong r = ((val & 0xFF0000) * (anAlpha + 1)) >> 8;
			ulong g = ((val & 0x00FF00) * (anAlpha + 1)) >> 8;
			ulong b = ((val & 0x0000FF) * (anAlpha + 1)) >> 8;

			anAlphaData[i] = (((r >> rRightShift) << rLeftShift) & rMask) |
							 (((g >> gRightShift) << gLeftShift) & gMask) |
							 (((b >> bRightShift) << bLeftShift) & bMask) | (anAlpha << 24);
		}

		mNativeAlphaData = anAlphaData;
	}

	return mNativeAlphaData;
}

uchar *MemoryImage::GetRLAlphaData()
{
	CommitBits();

	if (mRLAlphaData == nullptr)
	{
		mRLAlphaData = new uchar[mWidth * mHeight];

		if (mColorTable == nullptr)
		{
			ulong *aSrcPtr;
			if (mNativeAlphaData != nullptr)
				aSrcPtr = (ulong *)mNativeAlphaData;
			else
				aSrcPtr = GetBits();

#define NEXT_SRC_COLOR (*(aSrcPtr++))

#include "Inc/MI_GetRLAlphaData.inc"

#undef NEXT_SRC_COLOR
		}
		else
		{
			uchar *aSrcPtr = mColorIndices;
			ulong *aColorTable = mColorTable;

#define NEXT_SRC_COLOR (aColorTable[*(aSrcPtr++)])

#include "Inc/MI_GetRLAlphaData.inc"

#undef NEXT_SRC_COLOR
		}
	}

	return mRLAlphaData;
}

uchar *MemoryImage::GetRLAdditiveData(NativeDisplay *theNative)
{
	if (mRLAdditiveData == nullptr)
	{
		if (mColorTable == nullptr)
		{
			ulong *aBits = (ulong *)GetNativeAlphaData(theNative);

			mRLAdditiveData = new uchar[mWidth * mHeight];

			uchar *aWPtr = mRLAdditiveData;
			ulong *aRPtr = aBits;

			if (mWidth == 1)
			{
				memset(aWPtr, 1, mHeight);
			}
			else
			{
				for (int aRow = 0; aRow < mHeight; aRow++)
				{
					int aRCount = 1;
					int aRLCount = 1;

					int aLastAClass = (((*aRPtr++) & 0xFFFFFF) != 0) ? 1 : 0;

					while (aRCount < mWidth)
					{
						aRCount++;

						int aThisAClass = (((*aRPtr++) & 0xFFFFFF) != 0) ? 1 : 0;

						if ((aThisAClass != aLastAClass) || (aRCount == mWidth))
						{
							if (aThisAClass == aLastAClass)
								aRLCount++;

							for (int i = aRLCount; i > 0; i--)
							{
								if (i >= 255)
									*aWPtr++ = 255;
								else
									*aWPtr++ = i;
							}

							if ((aRCount == mWidth) && (aThisAClass != aLastAClass))
								*aWPtr++ = 1;

							aLastAClass = aThisAClass;
							aRLCount = 1;
						}
						else
						{
							aRLCount++;
						}
					}
				}
			}
		}
		else
		{
			ulong *aNativeColorTable = (ulong *)GetNativeAlphaData(theNative);

			mRLAdditiveData = new uchar[mWidth * mHeight];

			uchar *aWPtr = mRLAdditiveData;
			uchar *aRPtr = mColorIndices;

			if (mWidth == 1)
			{
				memset(aWPtr, 1, mHeight);
			}
			else
			{
				for (int aRow = 0; aRow < mHeight; aRow++)
				{
					int aRCount = 1;
					int aRLCount = 1;

					int aLastAClass = (((aNativeColorTable[*aRPtr++]) & 0xFFFFFF) != 0) ? 1 : 0;

					while (aRCount < mWidth)
					{
						aRCount++;

						int aThisAClass = (((aNativeColorTable[*aRPtr++]) & 0xFFFFFF) != 0) ? 1 : 0;

						if ((aThisAClass != aLastAClass) || (aRCount == mWidth))
						{
							if (aThisAClass == aLastAClass)
								aRLCount++;

							for (int i = aRLCount; i > 0; i--)
							{
								if (i >= 255)
									*aWPtr++ = 255;
								else
									*aWPtr++ = i;
							}

							if ((aRCount == mWidth) && (aThisAClass != aLastAClass))
								*aWPtr++ = 1;

							aLastAClass = aThisAClass;
							aRLCount = 1;
						}
						else
						{
							aRLCount++;
						}
					}
				}
			}
		}
	}

	return mRLAdditiveData;
}

void MemoryImage::PurgeBits()
{
	mPurgeBits = true;

	if (mApp->Is3DAccelerated())
	{
		// Due to potential D3D threading issues we have to defer the texture creation
		//  and therefore the actual purging
		if (mD3DData == nullptr)
			return;
	}
	else
	{
		if ((mBits == nullptr) && (mColorIndices == nullptr))
			return;

		GetNativeAlphaData(gAppBase->mSDLInterface);
	}

	delete[] mBits;
	mBits = nullptr;

	if (mD3DData != nullptr)
	{
		delete[] mColorIndices;
		mColorIndices = nullptr;

		delete[] mColorTable;
		mColorTable = nullptr;
	}
}

void MemoryImage::DeleteSWBuffers()
{
	if ((mBits == nullptr) && (mColorIndices == nullptr))
		GetBits();

	delete[] mNativeAlphaData;
	mNativeAlphaData = nullptr;

	delete[] mRLAdditiveData;
	mRLAdditiveData = nullptr;

	delete[] mRLAlphaData;
	mRLAlphaData = nullptr;
}

void MemoryImage::Delete3DBuffers()
{
	mApp->Remove3DData(this);
}

void MemoryImage::DeleteExtraBuffers()
{
	DeleteSWBuffers();
	Delete3DBuffers();
}

void MemoryImage::ReInit()
{
	// Fix any un-palletizing
	if (mWantPal)
		Palletize();

	if (mPurgeBits)
		PurgeBits();
}

void MemoryImage::DeleteNativeData()
{
	if ((mBits == nullptr) && (mColorIndices == nullptr))
		GetBits(); // We need to keep the bits around

	delete[] mNativeAlphaData;
	mNativeAlphaData = nullptr;

	delete[] mRLAdditiveData;
	mRLAdditiveData = nullptr;
}

void MemoryImage::SetBits(ulong *theBits, int theWidth, int theHeight, bool commitBits)
{
	if (theBits != mBits)
	{
		delete[] mColorIndices;
		mColorIndices = nullptr;

		delete[] mColorTable;
		mColorTable = nullptr;

		if (theWidth != mWidth || theHeight != mHeight)
		{
			delete[] mBits;
			mBits = new ulong[theWidth * theHeight + 1];
			mWidth = theWidth;
			mHeight = theHeight;
		}
		memcpy(mBits, theBits, mWidth * mHeight * sizeof(ulong));
		mBits[mWidth * mHeight] = MEMORYCHECK_ID;

		BitsChanged();
		if (commitBits)
			CommitBits();
	}
}

void MemoryImage::Create(int theWidth, int theHeight)
{
	delete[] mBits;
	mBits = nullptr;

	mWidth = theWidth;
	mHeight = theHeight;

	// All zeros --> trans + alpha
	mHasTrans = true;
	mHasAlpha = true;

	BitsChanged();
}

ulong *MemoryImage::GetBits()
{
	if (mBits == nullptr)
	{
		int aSize = mWidth * mHeight;

		mBits = new ulong[aSize + 1];
		mBits[aSize] = MEMORYCHECK_ID;

		if (mColorTable != nullptr)
		{
			for (int i = 0; i < aSize; i++)
				mBits[i] = mColorTable[mColorIndices[i]];

			delete[] mColorIndices;
			mColorIndices = nullptr;

			delete[] mColorTable;
			mColorTable = nullptr;

			delete[] mNativeAlphaData;
			mNativeAlphaData = nullptr;
		}
		else if (mNativeAlphaData != nullptr)
		{
			NativeDisplay *aDisplay = gAppBase->mSDLInterface;

			const int rMask = aDisplay->mRedMask;
			const int gMask = aDisplay->mGreenMask;
			const int bMask = aDisplay->mBlueMask;

			const int rLeftShift = aDisplay->mRedShift + (aDisplay->mRedBits);
			const int gLeftShift = aDisplay->mGreenShift + (aDisplay->mGreenBits);
			const int bLeftShift = aDisplay->mBlueShift + (aDisplay->mBlueBits);

			ulong *aDestPtr = mBits;
			ulong *aSrcPtr = mNativeAlphaData;

			int aSize = mWidth * mHeight;
			for (int i = 0; i < aSize; i++)
			{
				ulong val = *(aSrcPtr++);

				int anAlpha = val >> 24;

				ulong r = (((((val & rMask) << 8) / (anAlpha + 1)) & rMask) << 8) >> rLeftShift;
				ulong g = (((((val & gMask) << 8) / (anAlpha + 1)) & gMask) << 8) >> gLeftShift;
				ulong b = (((((val & bMask) << 8) / (anAlpha + 1)) & bMask) << 8) >> bLeftShift;

				*(aDestPtr++) = (r << 16) | (g << 8) | (b) | (anAlpha << 24);
			}
		}
		else if ((mD3DData == nullptr) || (!mApp->mSDLInterface->RecoverBits(this)))
		{
			memset(mBits, 0, aSize * sizeof(ulong));
		}
	}

	return mBits;
}

void MemoryImage::FillRect(const Rect &theRect, const Color &theColor, int theDrawMode)
{
	ulong src = theColor.ToInt();

	ulong *aBits = GetBits();

	int oldAlpha = src >> 24;

	if (oldAlpha == 0xFF)
	{
		for (int aRow = theRect.mY; aRow < theRect.mY + theRect.mHeight; aRow++)
		{
			ulong *aDestPixels = &aBits[aRow * mWidth + theRect.mX];

			for (int i = 0; i < theRect.mWidth; i++)
				*aDestPixels++ = src;
		}
	}
	else
	{
		for (int aRow = theRect.mY; aRow < theRect.mY + theRect.mHeight; aRow++)
		{
			ulong *aDestPixels = &aBits[aRow * mWidth + theRect.mX];

			for (int i = 0; i < theRect.mWidth; i++)
			{
				ulong dest = *aDestPixels;

				int aDestAlpha = dest >> 24;
				int aNewDestAlpha = aDestAlpha + ((255 - aDestAlpha) * oldAlpha) / 255;

				int newAlpha = 255 * oldAlpha / aNewDestAlpha;

				int oma = 256 - newAlpha;

#ifdef OPTIMIZE_SOFTWARE_DRAWING
				*(aDestPixels++) = (aNewDestAlpha << 24) |
								   ((((dest & 0xFF00FF) * oma + (src & 0xFF00FF) * newAlpha) >> 8) & 0xFF00FF) |
								   ((((dest & 0x00FF00) * oma + (src & 0x00FF00) * newAlpha) >> 8) & 0x00FF00);
#else
				*(aDestPixels++) =
					(aNewDestAlpha << 24) |
					((((dest & 0x0000FF) * oma) >> 8) + (((src & 0x0000FF) * newAlpha) >> 8) & 0x0000FF) |
					((((dest & 0x00FF00) * oma) >> 8) + (((src & 0x00FF00) * newAlpha) >> 8) & 0x00FF00) |
					((((dest & 0xFF0000) * oma) >> 8) + (((src & 0xFF0000) * newAlpha) >> 8) & 0xFF0000);
#endif
			}
		}
	}

	BitsChanged();
}

void MemoryImage::ClearRect(const Rect &theRect)
{
	ulong *aBits = GetBits();

	for (int aRow = theRect.mY; aRow < theRect.mY + theRect.mHeight; aRow++)
	{
		ulong *aDestPixels = &aBits[aRow * mWidth + theRect.mX];

		for (int i = 0; i < theRect.mWidth; i++)
			*aDestPixels++ = 0;
	}

	BitsChanged();
}

void MemoryImage::Clear()
{
	ulong *ptr = GetBits();
	if (ptr != nullptr)
	{
		for (int i = 0; i < mWidth * mHeight; i++)
			*ptr++ = 0;

		BitsChanged();
	}
}

void MemoryImage::AdditiveBlt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor)
{
	theImage->mDrawn = true;

	MemoryImage *aSrcMemoryImage = dynamic_cast<MemoryImage *>(theImage);

	uchar *aMaxTable = mApp->mAdd8BitMaxTable;

	if (aSrcMemoryImage != nullptr)
	{
		if (aSrcMemoryImage->mColorTable == nullptr)
		{
			ulong *aSrcBits = aSrcMemoryImage->GetBits();

#define NEXT_SRC_COLOR (*(aSrcPtr++))
#define SRC_TYPE ulong

#include "Inc/MI_AdditiveBlt.inc"

#undef NEXT_SRC_COLOR
#undef SRC_TYPE
		}
		else
		{
			ulong *aColorTable = aSrcMemoryImage->mColorTable;
			uchar *aSrcBits = aSrcMemoryImage->mColorIndices;

#define NEXT_SRC_COLOR (aColorTable[*(aSrcPtr++)])
#define SRC_TYPE uchar

#include "Inc/MI_AdditiveBlt.inc"

#undef NEXT_SRC_COLOR
#undef SRC_TYPE
		}

		BitsChanged();
	}
}

void MemoryImage::NormalBlt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor)
{
	theImage->mDrawn = true;

	MemoryImage *aSrcMemoryImage = dynamic_cast<MemoryImage *>(theImage);

	if (aSrcMemoryImage != nullptr)
	{
		if (aSrcMemoryImage->mColorTable == nullptr)
		{
			ulong *aSrcPixelsRow =
				((ulong *)aSrcMemoryImage->GetBits()) + (theSrcRect.mY * theImage->mWidth) + theSrcRect.mX;

#define NEXT_SRC_COLOR (*(aSrcPtr++))
#define READ_SRC_COLOR (*(aSrcPtr))
#define EACH_ROW ulong *aSrcPtr = aSrcPixelsRow

#include "Inc/MI_NormalBlt.inc"

#undef NEXT_SRC_COLOR
#undef READ_SRC_COLOR
#undef EACH_ROW
		}
		else
		{
			ulong *aColorTable = aSrcMemoryImage->mColorTable;
			uchar *aSrcPixelsRow = aSrcMemoryImage->mColorIndices + (theSrcRect.mY * theImage->mWidth) + theSrcRect.mX;

#define NEXT_SRC_COLOR (aColorTable[*(aSrcPtr++)])
#define READ_SRC_COLOR (aColorTable[*(aSrcPtr)])
#define EACH_ROW uchar *aSrcPtr = aSrcPixelsRow

#include "Inc/MI_NormalBlt.inc"

#undef NEXT_SRC_COLOR
#undef READ_SRC_COLOR
#undef EACH_ROW
		}

		BitsChanged();
	}
}

void MemoryImage::Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor,
					  int theDrawMode)
{
	theImage->mDrawn = true;

	switch (theDrawMode)
	{
	case Graphics::DRAWMODE_NORMAL:
		NormalBlt(theImage, theX, theY, theSrcRect, theColor);
		break;
	case Graphics::DRAWMODE_ADDITIVE:
		AdditiveBlt(theImage, theX, theY, theSrcRect, theColor);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MemoryImage::BltF(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect,
					   const Color &theColor, int theDrawMode)
{
	theImage->mDrawn = true;

	BltRotated(theImage, theX, theY, theSrcRect, theClipRect, theColor, theDrawMode, 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool MemoryImage::BltRotatedClipHelper(float &theX, float &theY, const Rect &theSrcRect, const Rect &theClipRect,
									   double theRot, FRect &theDestRect, float theRotCenterX, float theRotCenterY)
{
	// Clipping Code (this used to be in Graphics::DrawImageRotated)
	float aCos = cosf(theRot);
	float aSin = sinf(theRot);

	// Map the four corners and find the bounding rectangle
	float px[4] = {0.f, (float)theSrcRect.mWidth, (float)theSrcRect.mWidth, 0.f};
	float py[4] = {0.f, 0.f, (float)theSrcRect.mHeight, (float)theSrcRect.mHeight};
	float aMinX = 10000000.f;
	float aMaxX = -10000000.f;
	float aMinY = 10000000.f;
	float aMaxY = -10000000.f;

	for (int i = 0; i < 4; i++)
	{
		float ox = px[i] - theRotCenterX;
		float oy = py[i] - theRotCenterY;

		px[i] = (theRotCenterX + ox * aCos + oy * aSin) + theX;
		py[i] = (theRotCenterY + oy * aCos - ox * aSin) + theY;

		if (px[i] < aMinX)
			aMinX = px[i];
		if (px[i] > aMaxX)
			aMaxX = px[i];
		if (py[i] < aMinY)
			aMinY = py[i];
		if (py[i] > aMaxY)
			aMaxY = py[i];
	}

	FRect aClipRect(theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight);

	FRect aDestRect = FRect(aMinX, aMinY, aMaxX - aMinX, aMaxY - aMinY).Intersection(aClipRect);
	if ((aDestRect.mWidth <= 0) || (aDestRect.mHeight <= 0)) // nothing to draw
		return false;

	theDestRect = aDestRect;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool MemoryImage::StretchBltClipHelper(const Rect &theSrcRect, const Rect &theClipRect, const Rect &theDestRect,
									   FRect &theSrcRectOut, Rect &theDestRectOut)
{
	theDestRectOut =
		Rect(theDestRect.mX, theDestRect.mY, theDestRect.mWidth, theDestRect.mHeight).Intersection(theClipRect);

	double aXFactor = theSrcRect.mWidth / (double)theDestRect.mWidth;
	double aYFactor = theSrcRect.mHeight / (double)theDestRect.mHeight;

	theSrcRectOut = FRect(theSrcRect.mX + (theDestRectOut.mX - theDestRect.mX) * aXFactor,
						  theSrcRect.mY + (theDestRectOut.mY - theDestRect.mY) * aYFactor,
						  theSrcRect.mWidth + (theDestRectOut.mWidth - theDestRect.mWidth) * aXFactor,
						  theSrcRect.mHeight + (theDestRectOut.mHeight - theDestRect.mHeight) * aYFactor);

	return theSrcRectOut.mWidth > 0 && theSrcRectOut.mHeight > 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool MemoryImage::StretchBltMirrorClipHelper(const Rect &theSrcRect, const Rect &theClipRect, const Rect &theDestRect,
											 FRect &theSrcRectOut, Rect &theDestRectOut)
{
	theDestRectOut =
		Rect(theDestRect.mX, theDestRect.mY, theDestRect.mWidth, theDestRect.mHeight).Intersection(theClipRect);

	double aXFactor = theSrcRect.mWidth / (double)theDestRect.mWidth;
	double aYFactor = theSrcRect.mHeight / (double)theDestRect.mHeight;

	int aTotalClip = theDestRect.mWidth - theDestRectOut.mWidth;
	int aLeftClip = theDestRectOut.mX - theDestRect.mX;
	int aRightClip = aTotalClip - aLeftClip;

	theSrcRectOut =
		FRect(theSrcRect.mX + (aRightClip)*aXFactor, theSrcRect.mY + (theDestRectOut.mY - theDestRect.mY) * aYFactor,
			  theSrcRect.mWidth + (theDestRectOut.mWidth - theDestRect.mWidth) * aXFactor,
			  theSrcRect.mHeight + (theDestRectOut.mHeight - theDestRect.mHeight) * aYFactor);

	return theSrcRectOut.mWidth > 0 && theSrcRectOut.mHeight > 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void MemoryImage::BltRotated(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect,
							 const Color &theColor, int theDrawMode, double theRot, float theRotCenterX,
							 float theRotCenterY)
{
	theImage->mDrawn = true;

	// This BltRotatedClipHelper clipping used to happen in Graphics::DrawImageRotated
	FRect aDestRect;
	if (!BltRotatedClipHelper(theX, theY, theSrcRect, theClipRect, theRot, aDestRect, theRotCenterX, theRotCenterY))
		return;

	MemoryImage *aMemoryImage = dynamic_cast<MemoryImage *>(theImage);
	uchar *aMaxTable = mApp->mAdd8BitMaxTable;

	if (aMemoryImage != nullptr)
	{
		if (aMemoryImage->mColorTable == nullptr)
		{
			ulong *aSrcBits = aMemoryImage->GetBits() + theSrcRect.mX + theSrcRect.mY * theSrcRect.mWidth;

#define SRC_TYPE ulong
#define READ_COLOR(ptr) (*(ptr))

			if (theDrawMode == Graphics::DRAWMODE_NORMAL)
			{
#include "Inc/MI_BltRotated.inc"
			}
			else
			{
#include "Inc/MI_BltRotated_Additive.inc"
			}

#undef SRC_TYPE
#undef READ_COLOR
		}
		else
		{
			ulong *aColorTable = aMemoryImage->mColorTable;
			uchar *aSrcBits = aMemoryImage->mColorIndices + theSrcRect.mX + theSrcRect.mY * theSrcRect.mWidth;

#define SRC_TYPE uchar
#define READ_COLOR(ptr) (aColorTable[*(ptr)])

			if (theDrawMode == Graphics::DRAWMODE_NORMAL)
			{
#include "Inc/MI_BltRotated.inc"
			}
			else
			{
#include "Inc/MI_BltRotated_Additive.inc"
			}

#undef SRC_TYPE
#undef READ_COLOR
		}

		BitsChanged();
	}
}

void MemoryImage::SlowStretchBlt(Image *theImage, const Rect &theDestRect, const FRect &theSrcRect,
								 const Color &theColor, int theDrawMode)
{
	theImage->mDrawn = true;

	// This thing was a pain to write.  I bet i could have gotten something just as good
	// from some Graphics Gems book.

	ulong *aDestEnd = GetBits() + (mWidth * mHeight);

	MemoryImage *aSrcMemoryImage = dynamic_cast<MemoryImage *>(theImage);

	if (aSrcMemoryImage != nullptr)
	{
		if (aSrcMemoryImage->mColorTable == nullptr)
		{
			ulong *aSrcBits = aSrcMemoryImage->GetBits();

#define SRC_TYPE ulong
#define READ_COLOR(ptr) (*(ptr))

#include "Inc/MI_SlowStretchBlt.inc"

#undef SRC_TYPE
#undef READ_COLOR
		}
		else
		{
			ulong *aColorTable = aSrcMemoryImage->mColorTable;
			uchar *aSrcBits = aSrcMemoryImage->mColorIndices;

#define SRC_TYPE uchar
#define READ_COLOR(ptr) (aColorTable[*(ptr)])

#include "Inc/MI_SlowStretchBlt.inc"

#undef SRC_TYPE
#undef READ_COLOR
		}

		BitsChanged();
	}
}

// TODO: Make the special version
void MemoryImage::FastStretchBlt(Image *theImage, const Rect &theDestRect, const FRect &theSrcRect,
								 const Color &theColor, int theDrawMode)
{
	theImage->mDrawn = true;

	MemoryImage *aSrcMemoryImage = dynamic_cast<MemoryImage *>(theImage);

	if (aSrcMemoryImage != nullptr)
	{
		ulong *aDestPixelsRow = ((ulong *)GetBits()) + (theDestRect.mY * mWidth) + theDestRect.mX;
		ulong *aSrcPixelsRow = (ulong *)aSrcMemoryImage->GetBits();
		;

		double aSrcY = theSrcRect.mY;

		double anAddX = theSrcRect.mWidth / theDestRect.mWidth;
		double anAddY = theSrcRect.mHeight / theDestRect.mHeight;

		if (theColor == Color::White)
		{
			for (int y = 0; y < theDestRect.mHeight; y++)
			{
				double aSrcX = theSrcRect.mX;

				ulong *aDestPixels = aDestPixelsRow;

				for (int x = 0; x < theDestRect.mWidth; x++)
				{
					aSrcX += anAddX;

					ulong *aSrcPixels = aSrcPixelsRow + ((int)aSrcX) + (aSrcMemoryImage->mWidth * ((int)aSrcY));
					ulong src = *aSrcPixels;

					ulong dest = *aDestPixels;

					int a = src >> 24;

					if (a != 0)
					{
						int aDestAlpha = dest >> 24;
						int aNewDestAlpha = aDestAlpha + ((255 - aDestAlpha) * a) / 255;

						a = 255 * a / aNewDestAlpha;

						int oma = 256 - a;

						*(aDestPixels++) =
							(aNewDestAlpha << 24) |
							((((dest & 0x0000FF) * oma) >> 8) + (((src & 0x0000FF) * a) >> 8) & 0x0000FF) |
							((((dest & 0x00FF00) * oma) >> 8) + (((src & 0x00FF00) * a) >> 8) & 0x00FF00) |
							((((dest & 0xFF0000) * oma) >> 8) + (((src & 0xFF0000) * a) >> 8) & 0xFF0000);
					}
					else
						aDestPixels++;
				}

				aDestPixelsRow += mWidth;
				aSrcY += anAddY;
			}
		}
		else
		{
		}
	}

	BitsChanged();
}

void MemoryImage::StretchBlt(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect &theClipRect,
							 const Color &theColor, int theDrawMode, bool fastStretch)
{
	theImage->mDrawn = true;

	Rect aDestRect;
	FRect aSrcRect;

	if (!StretchBltClipHelper(theSrcRect, theClipRect, theDestRect, aSrcRect, aDestRect))
		return;

	if (fastStretch)
		FastStretchBlt(theImage, aDestRect, aSrcRect, theColor, theDrawMode);
	else
		SlowStretchBlt(theImage, aDestRect, aSrcRect, theColor, theDrawMode);
}

void MemoryImage::BltMatrixHelper(Image *theImage, float x, float y, const Matrix3 &theMatrix,
								  const Rect &theClipRect, const Color &theColor, int theDrawMode,
								  const Rect &theSrcRect, void *theSurface, int theBytePitch, int thePixelFormat,
								  bool blend)
{
	MemoryImage *anImage = dynamic_cast<MemoryImage *>(theImage);
	if (anImage == nullptr)
		return;

	float w2 = theSrcRect.mWidth / 2.0f;
	float h2 = theSrcRect.mHeight / 2.0f;

	float u0 = (float)theSrcRect.mX / theImage->mWidth;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / theImage->mWidth;
	float v0 = (float)theSrcRect.mY / theImage->mHeight;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / theImage->mHeight;

	SWHelper::XYZStruct aVerts[4] = {{-w2, -h2, u0, v0, (long)0xFFFFFFFF},
									 {w2, -h2, u1, v0, (long)0xFFFFFFFF},
									 {-w2, h2, u0, v1, (long)0xFFFFFFFF},
									 {w2, h2, u1, v1, (long)0xFFFFFFFF}};

	for (int i = 0; i < 4; i++)
	{
		Vector3 v(aVerts[i].mX, aVerts[i].mY, 1);
		v = theMatrix * v;
		aVerts[i].mX = v.x + x - 0.5f;
		aVerts[i].mY = v.y + y - 0.5f;
	}

	SWHelper::SWDrawShape(aVerts, 4, anImage, theColor, theDrawMode, theClipRect, theSurface, theBytePitch,
						  thePixelFormat, blend, false);
}

void MemoryImage::BltMatrix(Image *theImage, float x, float y, const Matrix3 &theMatrix, const Rect &theClipRect,
							const Color &theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
	theImage->mDrawn = true;

	ulong *aSurface = GetBits();
	int aPitch = mWidth * 4;
	int aFormat = 0x8888;
	if (mForcedMode && !mHasAlpha && !mHasTrans)
		aFormat = 0x888;

	BltMatrixHelper(theImage, x, y, theMatrix, theClipRect, theColor, theDrawMode, theSrcRect, aSurface, aPitch,
					aFormat, blend);
	BitsChanged();
}

void MemoryImage::BltTrianglesTexHelper(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles,
										const Rect &theClipRect, const Color &theColor, int theDrawMode,
										void *theSurface, int theBytePitch, int thePixelFormat, float tx, float ty,
										bool blend)
{
	MemoryImage *anImage = dynamic_cast<MemoryImage *>(theTexture);
	//	if (anImage==nullptr)
	//		return;

	int aColor = theColor.ToInt();
	for (int i = 0; i < theNumTriangles; i++)
	{
		bool vertexColor = false;

		SWHelper::XYZStruct aVerts[3];
		for (int j = 0; j < 3; j++)
		{
			aVerts[j].mX = theVertices[i][j].x + tx;
			aVerts[j].mY = theVertices[i][j].y + ty;
			aVerts[j].mU = theVertices[i][j].u;
			aVerts[j].mV = theVertices[i][j].v;
			aVerts[j].mDiffuse = theVertices[i][j].color;

			if (aVerts[j].mDiffuse != 0)
				vertexColor = true;
		}

		SWHelper::SWDrawShape(aVerts, 3, anImage, theColor, theDrawMode, theClipRect, theSurface, theBytePitch,
							  thePixelFormat, blend, vertexColor);
	}
}

void MemoryImage::FillScanLinesWithCoverage(Span *theSpans, int theSpanCount, const Color &theColor, int theDrawMode,
											const BYTE *theCoverage, int theCoverX, int theCoverY, int theCoverWidth,
											int theCoverHeight)
{
	ulong *theBits = GetBits();
	ulong src = theColor.ToInt();
	for (int i = 0; i < theSpanCount; ++i)
	{
		Span *aSpan = &theSpans[i];
		int x = aSpan->mX - theCoverX;
		int y = aSpan->mY - theCoverY;

		ulong *aDestPixels = &theBits[aSpan->mY * mWidth + aSpan->mX];
		const BYTE *aCoverBits = &theCoverage[y * theCoverWidth + x];
		for (int w = 0; w < aSpan->mWidth; ++w)
		{
			int cover = *aCoverBits++ + 1;
			int a = (cover * theColor.mAlpha) >> 8;
			int oma;
			ulong dest = *aDestPixels;

			if (a > 0)
			{
				int aDestAlpha = dest >> 24;
				int aNewDestAlpha = aDestAlpha + ((255 - aDestAlpha) * a) / 255;

				a = 255 * a / aNewDestAlpha;
				oma = 256 - a;
				*(aDestPixels++) = (aNewDestAlpha << 24) |
								   ((((dest & 0x0000FF) * oma + (src & 0x0000FF) * a) >> 8) & 0x0000FF) |
								   ((((dest & 0x00FF00) * oma + (src & 0x00FF00) * a) >> 8) & 0x00FF00) |
								   ((((dest & 0xFF0000) * oma + (src & 0xFF0000) * a) >> 8) & 0xFF0000);
			}
		}
	}
	BitsChanged();
}

void MemoryImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles,
								  const Rect &theClipRect, const Color &theColor, int theDrawMode, float tx, float ty,
								  bool blend)
{
	theTexture->mDrawn = true;

	ulong *aSurface = GetBits();

	int aPitch = mWidth * 4;
	int aFormat = 0x8888;
	if (mForcedMode && !mHasAlpha && !mHasTrans)
		aFormat = 0x888;

	BltTrianglesTexHelper(theTexture, theVertices, theNumTriangles, theClipRect, theColor, theDrawMode, aSurface,
						  aPitch, aFormat, tx, ty, blend);
	BitsChanged();
}

bool MemoryImage::Palletize()
{
	CommitBits();

	if (mColorTable != nullptr)
		return true;

	GetBits();

	if (mBits == nullptr)
		return false;

	mColorIndices = new uchar[mWidth * mHeight];
	mColorTable = new ulong[256];

	if (!Quantize8Bit(mBits, mWidth, mHeight, mColorIndices, mColorTable))
	{
		delete[] mColorIndices;
		mColorIndices = nullptr;

		delete[] mColorTable;
		mColorTable = nullptr;

		mWantPal = false;

		return false;
	}

	delete[] mBits;
	mBits = nullptr;

	delete[] mNativeAlphaData;
	mNativeAlphaData = nullptr;

	mWantPal = true;

	return true;
}
