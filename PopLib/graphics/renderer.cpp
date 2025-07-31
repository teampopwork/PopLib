#include "renderer.hpp"
#include "graphics.hpp"

using namespace PopLib;

bool PopLib::gRendererPreDrawError = false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Renderer::Renderer()
{
	mRGBBits = 0;

	mRedMask = 0;
	mGreenMask = 0;
	mBlueMask = 0;

	mRedBits = 0;
	mGreenBits = 0;
	mBlueBits = 0;

	mRedShift = 0;
	mGreenShift = 0;
	mBlueShift = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Renderer::~Renderer()
{
}


BlendMode Renderer::ChooseBlendMode(int theBlendMode)
{
	BlendMode theBBlendMode;
	switch (theBlendMode)
	{
	case Graphics::DRAWMODE_ADDITIVE:
		theBBlendMode = BLENDMODE_ADD;
		break;
	default:
	case Graphics::DRAWMODE_NORMAL:
		theBBlendMode = BLENDMODE_BLEND;
		break;
	}
	return theBBlendMode;
}
