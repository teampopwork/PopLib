#include "glimage.hpp"
#include "misc/critsect.hpp"
#include "debug/log.hpp"
#include "glrenderer.hpp"
#include "appbase.hpp"
#include "../image.hpp"

using namespace PopLib;

GLImage::GLImage() : GPUImage()
{
	mRenderer = gAppBase->mRenderer;
	mRenderer->AddImage(this);
}

GLImage::GLImage(Renderer *theRenderer) : GPUImage()
{
	mRenderer = theRenderer;
	mRenderer->AddImage(this);
}

GLImage::~GLImage()
{
	mRenderer->RemoveImage(this);
}

void GLImage::Create(int theWidth, int theHeight)
{
	delete[] mBits;

	mBits = nullptr;

	mWidth = theWidth;
	mHeight = theHeight;

	mHasTrans = true;
	mHasAlpha = true;

	BitsChanged();
}

bool GLImage::PolyFill3D(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor,
						 int theDrawMode, int tx, int ty)
{
	mRenderer->FillPoly(theVertices, theNumVertices, theClipRect, theColor, theDrawMode, tx, ty);
	return true;
}

void GLImage::FillRect(const Rect &theRect, const Color &theColor, int theDrawMode)
{
	mRenderer->FillRect(theRect, theColor, theDrawMode);
}

void GLImage::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor,
					   int theDrawMode)
{
	mRenderer->DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
}

void GLImage::DrawLineAA(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor,
						 int theDrawMode)
{
	mRenderer->DrawLine(theStartX, theStartY, theEndX, theEndY, theColor, theDrawMode);
}

void GLImage::Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor, int theDrawMode)
{
	theImage->mDrawn = true;

	CommitBits();

	mRenderer->Blt(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
}

void GLImage::BltF(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect,
				   const Color &theColor, int theDrawMode)
{
	theImage->mDrawn = true;

	FRect aClipRect(theClipRect.mX, theClipRect.mY, theClipRect.mWidth, theClipRect.mHeight);
	FRect aDestRect(theX, theY, theSrcRect.mWidth, theSrcRect.mHeight);

	FRect anIntersect = aDestRect.Intersection(aClipRect);
	if (anIntersect.mWidth != aDestRect.mWidth || anIntersect.mHeight != aDestRect.mHeight)
	{
		if (anIntersect.mWidth != 0 && anIntersect.mHeight != 0)
			mRenderer->BltClipF(theImage, theX, theY, theSrcRect, &theClipRect, theColor, theDrawMode);
	}
	else
		mRenderer->Blt(theImage, theX, theY, theSrcRect, theColor, theDrawMode, true);
}

void GLImage::BltRotated(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect &theClipRect,
						 const Color &theColor, int theDrawMode, double theRot, float theRotCenterX,
						 float theRotCenterY)
{
	theImage->mDrawn = true;

	CommitBits();

	mRenderer->BltRotated(theImage, theX, theY, &theClipRect, theColor, theDrawMode, theRot, theRotCenterX,
						  theRotCenterY, theSrcRect);
}

void GLImage::StretchBlt(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect &theClipRect,
						 const Color &theColor, int theDrawMode, bool fastStretch)
{
	theImage->mDrawn = true;

	CommitBits();

	mRenderer->StretchBlt(theImage, theDestRect, theSrcRect, &theClipRect, theColor, theDrawMode, fastStretch);
}

void GLImage::BltMatrix(Image *theImage, float x, float y, const Matrix3 &theMatrix, const Rect &theClipRect,
						const Color &theColor, int theDrawMode, const Rect &theSrcRect, bool blend)
{
	theImage->mDrawn = true;

	mRenderer->BltTransformed(theImage, &theClipRect, theColor, theDrawMode, theSrcRect, theMatrix, blend, x, y, true);
}

void GLImage::BltTrianglesTex(Image *theTexture, const TriVertex theVertices[][3], int theNumTriangles,
							  const Rect &theClipRect, const Color &theColor, int theDrawMode, float tx, float ty,
							  bool blend)
{
	theTexture->mDrawn = true;

	mRenderer->DrawTrianglesTex(theVertices, theNumTriangles, theColor, theDrawMode, theTexture, tx, ty, blend);
}

void GLImage::BltMirror(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor,
						int theDrawMode)
{
	theImage->mDrawn = true;

	CommitBits();

	mRenderer->BltMirror(theImage, theX, theY, theSrcRect, theColor, theDrawMode);
}

void GLImage::StretchBltMirror(Image *theImage, const Rect &theDestRectOrig, const Rect &theSrcRect,
							   const Rect &theClipRect, const Color &theColor, int theDrawMode, bool fastStretch)
{
	theImage->mDrawn = true;

	CommitBits();

	mRenderer->StretchBlt(theImage, theDestRectOrig, theSrcRect, &theClipRect, theColor, theDrawMode, fastStretch,
						  true);
}

void GLImage::FillScanLinesWithCoverage(Span *theSpans, int theSpanCount, const Color &theColor, int theDrawMode,
										const BYTE *theCoverage, int theCoverX, int theCoverY, int theCoverWidth,
										int theCoverHeight)
{
	if (theSpanCount == 0)
		return;

	int l = theSpans[0].mX, t = theSpans[0].mY;
	int r = l + theSpans[0].mWidth, b = t;
	for (int i = 1; i < theSpanCount; ++i)
	{
		l = std::min(theSpans[i].mX, l);
		r = std::max(theSpans[i].mX + theSpans[i].mWidth - 1, r);
		t = std::min(theSpans[i].mY, t);
		b = std::max(theSpans[i].mY + theSpans[i].mWidth - 1, b);
	}
	for (int i = 0; i < theSpanCount; ++i)
	{
		theSpans[i].mX -= l;
		theSpans[i].mY -= t;
	}

	MemoryImage aTempImage;
	aTempImage.Create(r - l + 1, b - t + 1);
	aTempImage.FillScanLinesWithCoverage(theSpans, theSpanCount, theColor, theDrawMode, theCoverage, theCoverX - l,
										 theCoverY - t, theCoverWidth, theCoverHeight);
	Blt(&aTempImage, l, t, Rect(0, 0, r - l + 1, b - t + 1), Color::White, theDrawMode);
	return;
}

bool GLImage::Check3D(GLImage *theImage)
{
	return true;
}

bool GLImage::Check3D(Image *theImage)
{
	GLImage *anImage = dynamic_cast<GLImage *>(theImage);
	return anImage != nullptr;
}

void GLImage::PurgeBits()
{
	mPurgeBits = true;

	CommitBits();
	GetBits();

	MemoryImage::PurgeBits();
}