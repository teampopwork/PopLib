#include "glrenderer.hpp"
#include "appbase.hpp"
#include "math/matrix.hpp"
#include "graphics/graphics.hpp"
#include "misc/autocrit.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "debug/log.hpp"

using namespace PopLib;

#define MAX_VERTICES 16384

const char *gVertexShaderSrc = R"glsl(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
layout(location = 2) in vec4 aColor;

uniform mat4 uProjection;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
    vTexCoord = aTex;
    vColor = aColor;
}
)glsl";

const char *gFragmentShaderSrc = R"glsl(
#version 330 core

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform bool uUseTexture;

out vec4 FragColor;

void main() {
    if (uUseTexture)
        FragColor = texture(uTexture, vTexCoord) * vColor;
    else
        FragColor = vColor;
}
)glsl";

GLRenderer::GLRenderer(AppBase *theApp)
{
	mApp = theApp;
	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
	mDisplayWidth = mWidth;
	mDisplayHeight = mHeight;
	mPresentationRect = Rect(0, 0, mWidth, mHeight);
	mScreenImage = nullptr;
	mHasInitiated = false;
	mIs3D = true;
	mMillisecondsPerFrame = 0;
	mRefreshRate = 0;
}

GLRenderer::~GLRenderer()
{
	Cleanup();
}

int GLRenderer::Init()
{
	int aResult = RESULT_OK;

	if (mHasInitiated)
		Cleanup();

	mRGBBits = 32;

	mRedBits = 8;
	mGreenBits = 8;
	mBlueBits = 8;

	mRedShift = 0;
	mGreenShift = 8;
	mBlueShift = 16;

	mRedMask = (0xFFU << mRedShift);
	mGreenMask = (0xFFU << mGreenShift);
	mBlueMask = (0xFFU << mBlueShift);

	aResult = (InitGLContext() && InitBuffers()) ? aResult : RESULT_FAIL;
	mHasInitiated = true;
	return aResult;
}

void GLRenderer::Cleanup()
{
	if (mScreenImage)
		delete mScreenImage;

	if (mDefaultShader)
		delete mDefaultShader;

	GLImageSet::iterator anItr;
	for (anItr = mImageSet.begin(); anItr != mImageSet.end(); ++anItr)
	{
		GLImage *anImage = *anItr;
		GLTextureData *aData = (GLTextureData *)anImage->mGPUData;
		delete aData;
		anImage->mGPUData = nullptr;
	}
	mImageSet.clear();

	for (auto &textEntry : mTextTextureCache)
	{
		glDeleteTextures(1, &textEntry.second.textureID);
	}
	mTextTextureCache.clear();

	mCommandBuffer.clear();

	SDL_GL_DestroyContext(mContext);

	glDeleteBuffers(1, &mVBO);
	glDeleteVertexArrays(1, &mVAO);
}

bool GLRenderer::InitGLContext()
{
	mContext = SDL_GL_CreateContext(mApp->mWindow);

	if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
	{
		// ERRORS HERE
		return false;
	}

	SDL_GL_MakeCurrent(mApp->mWindow, mContext);

	mDefaultShader = new GLShader();
	mDefaultShader->LoadFromSource(gVertexShaderSrc, gFragmentShaderSrc);

	SetVideoOnlyDraw(false);

	return true;
}

bool GLRenderer::InitBuffers()
{
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);

	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, mPos));

	// texcoord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, mTexCoord));

	// color
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, mColor));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

bool GLRenderer::Redraw(Rect *theClipRect)
{
	SDL_GL_SwapWindow(mApp->mWindow);

	if (mCommandBuffer.empty())
		return !gRendererPreDrawError;

	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(mPresentationRect.mX, mPresentationRect.mY, mPresentationRect.mWidth, mPresentationRect.mHeight);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);

	for (const auto &cmd : mCommandBuffer)
	{
		if (cmd.mVertices.size() > MAX_VERTICES)
			continue; // Add a warning

		ApplyBlendMode(cmd.mBlendMode);
		GLShader *aShaderToUse;
		if (cmd.mShader != nullptr)
			aShaderToUse = cmd.mShader;
		else
			aShaderToUse = mDefaultShader;

		if (cmd.mClipRect != nullptr)
		{
			glEnable(GL_SCISSOR_TEST);
			// glScissor(cmd.mClipRect->mX, cmd.mClipRect->mY, cmd.mClipRect->mWidth, cmd.mClipRect->mHeight);
		}

		aShaderToUse->Use();
		aShaderToUse->SetUniform("uProjection", mProjection);
		aShaderToUse->SetUniform("uUseTexture", (cmd.mTextureID != 0));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cmd.mTextureID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, cmd.mVertices.size() * sizeof(Vertex), cmd.mVertices.data());
		glDrawArrays(cmd.mPrimitiveType, 0, (GLsizei)cmd.mVertices.size());
	}

	mCommandBuffer.clear();

	glDisable(GL_SCISSOR_TEST);

	return !gRendererPreDrawError;
}

void GLRenderer::ApplyBlendMode(BlendMode mode)
{
	auto it = blend_mode_funcs.find(mode);
	if (it == blend_mode_funcs.end())
		return;

	const auto &blend = it->second;

	if (blend.enable_blend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(blend.src, blend.dst);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void GLRenderer::SetVideoOnlyDraw(bool videoOnly)
{
	if (mScreenImage)
		delete mScreenImage;
	mScreenImage = new GLImage();
	mScreenImage->Create(mWidth, mHeight);
	mScreenImage->mWidth = mWidth;
	mScreenImage->mHeight = mHeight;
	mScreenImage->SetImageMode(false, false);
}

GPUImage *GLRenderer::GetScreenImage()
{
	return mScreenImage;
}

std::unique_ptr<ImageData> GLRenderer::CaptureFrameBuffer()
{
	uint8_t *pixels = new uint8_t[3 * mWidth * mHeight];

	glReadPixels(0, 0, mWidth, mHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	auto image = std::make_unique<ImageData>();
	image->width = mWidth;
	image->height = mHeight;
	image->pixels.resize(mWidth * mHeight * 4);

	uint8_t *src = static_cast<uint8_t *>(pixels);
	uint8_t *dst = image->pixels.data();

	for (int i = 0; i < mWidth * mHeight * 4; i += 4)
	{
		dst[i + 0] = src[i + 2]; // R
		dst[i + 1] = src[i + 1]; // G
		dst[i + 2] = src[i + 0]; // B
		dst[i + 3] = src[i + 3]; // A
	}

	return image;
}

bool GLRenderer::CreateImageTexture(GPUImage *theImage)
{
	bool wantPurge = false;

	if (theImage->mGPUData == nullptr)
	{
		theImage->mGPUData = new GLTextureData();

		// The actual purging was deferred
		wantPurge = theImage->mPurgeBits;

		AutoCrit aCrit(mCritSect); // Make images thread safe
		mImageSet.insert(static_cast<GLImage *>(theImage));
	}

	GLTextureData *aData = static_cast<GLTextureData *>(theImage->mGPUData);
	aData->CheckCreateTextures(static_cast<GLImage *>(theImage));

	if (wantPurge)
		theImage->PurgeBits();

	return true;
}

void GLRenderer::AddImage(Image *theImage)
{
	AutoCrit anAutoCrit(mCritSect);

	mImageSet.insert((GLImage *)theImage);
}

void GLRenderer::RemoveImage(Image *theImage)
{
	AutoCrit anAutoCrit(mCritSect);

	GLImageSet::iterator anItr = mImageSet.find((GLImage *)theImage);
	if (anItr != mImageSet.end())
		mImageSet.erase(anItr);
}

void GLRenderer::Remove3DData(GPUImage *theImage)
{
	if (theImage->mGPUData != nullptr)
	{
		delete (GLTextureData *)theImage->mGPUData;
		theImage->mGPUData = nullptr;

		AutoCrit aCrit(mCritSect); // Make images thread safe
		mImageSet.erase(static_cast<GLImage *>(theImage));
	}
}
void GLRenderer::GetOutputSize(int *outWidth, int *outHeight)
{
	GLint m_viewport[4];

	glGetIntegerv(GL_VIEWPORT, m_viewport);
	*outWidth = m_viewport[2];
	*outHeight = m_viewport[3];
}

void GLRenderer::UpdateViewport()
{
	if (SDL_GetCurrentThreadID() != SDL_GetThreadID(nullptr))
		return;

	int aWindowWidth, aWindowHeight;
	if (!SDL_GetWindowSize(mApp->mWindow, &aWindowWidth, &aWindowHeight))
		return;

	float windowAspect = (float)aWindowWidth / aWindowHeight;
	float logicalAspect = (float)mWidth / mHeight;

	int vpX, vpY, vpW, vpH;

	if (windowAspect > logicalAspect)
	{
		vpH = aWindowHeight;
		vpW = (int)(logicalAspect * vpH);
		vpX = (aWindowWidth - vpW) / 2;
		vpY = 0;
	}
	else
	{
		vpW = aWindowWidth;
		vpH = (int)(vpW / logicalAspect);
		vpX = 0;
		vpY = (aWindowHeight - vpH) / 2;
	}

	// Set the OpenGL viewport
	glViewport(vpX, vpY, vpW, vpH);

	mPresentationRect = Rect(vpX, vpY, vpW, vpH);

	mProjection =
		glm::ortho(0.0f, (float)mPresentationRect.mWidth, (float)mPresentationRect.mHeight, 0.0f, -1.0f, 1.0f) *
		glm::mat4(1.0f);
}

bool GLRenderer::RecoverBits(GPUImage *theImage)
{
	if (theImage->mGPUData == nullptr)
		return false;

	GLTextureData *aData = (GLTextureData *)theImage->mGPUData;
	if (aData->mBitsChangedCount != theImage->mBitsChangedCount) // bits have changed since texture was created
		return false;

	// Reverse the process: copy texture data to theImage
	void *pixels = nullptr;
	glBindTexture(GL_TEXTURE_2D, aData->mTextureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	theImage->SetBits((uint32_t *)pixels, aData->mWidth, aData->mHeight);

	return true;
}

GLTextureData::GLTextureData()
{
	mWidth = 0;
	mHeight = 0;
	mBitsChangedCount = 0;
	mTextureID = 0;
}

GLTextureData::~GLTextureData()
{
	ReleaseTextures();
}

void GLTextureData::ReleaseTextures()
{
	if (mTextureID != 0)
		glDeleteTextures(1, &mTextureID);
}

void GLTextureData::CreateTextures(GLImage *theImage)
{
	theImage->DeleteSWBuffers(); // we don't need the software buffers anymore
	theImage->CommitBits();

	bool createTexture = false;

	// only recreate the texture if the dimensions or image data have changed
	if (mWidth != theImage->mWidth || mHeight != theImage->mHeight || mBitsChangedCount != theImage->mBitsChangedCount)
	{
		ReleaseTextures();
		createTexture = true;
	}

	int aWidth = theImage->GetWidth();
	int aHeight = theImage->GetHeight();

	if (createTexture)
	{
		glGenTextures(1, &mTextureID);
		glBindTexture(GL_TEXTURE_2D, mTextureID);

		bool doNearestFilter = theImage->mImageFlags & GPUImageFlag_NearestFiltering;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, doNearestFilter ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, doNearestFilter ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, aWidth, aHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, theImage->GetBits());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else if (mBitsChangedCount != theImage->mBitsChangedCount)
	{
		void *bits = theImage->GetBits();
		if (bits)
		{
			glGenTextures(1, &mTextureID);
			glBindTexture(GL_TEXTURE_2D, mTextureID);

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, aWidth, aHeight, GL_BGRA, GL_UNSIGNED_BYTE, bits);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			LOG_ERROR("Error: Image bits are nullptr, cannot update texture.");
		}
	}

	mWidth = theImage->mWidth;
	mHeight = theImage->mHeight;
	mBitsChangedCount = theImage->mBitsChangedCount;
}

void GLTextureData::CheckCreateTextures(GLImage *theImage)
{
	if (mTextureID != 0)
	{
		if (mWidth != theImage->mWidth || mHeight != theImage->mHeight ||
			mBitsChangedCount != theImage->mBitsChangedCount)
			CreateTextures(theImage);
		return;
	}
	CreateTextures(theImage);
}

int GLTextureData::GetMemSize()
{
	int aSize = 0;

	aSize = 4 * mWidth * mHeight; // TODO: ADD MORE PIXEL FORMATS

	return aSize;
}

bool GLRenderer::PreDraw()
{
	return true;
}

GLImage *GLRenderer::SetupImage(Image *theImage)
{
	GLImage *aImg = static_cast<GLImage *>(theImage);

	if (aImg->mGPUData == nullptr)
		CreateImageTexture(aImg);
	return aImg;
}

void GLRenderer::AddCommand(const GLDrawCommand &command)
{
	mCommandBuffer.push_back(command);
}

/////////////////////////////////////////////////////////////////
///				DRAWING/BLITTING FUNCTIONS		    	   //////
/////////////////////////////////////////////////////////////////

GLTextCacheEntry GLRenderer::GetOrCreateText(const PopString &theText, TTF_Font *theFont)
{
	auto it = mTextTextureCache.find(theText);
	if (it != mTextTextureCache.end())
	{
		return it->second;
	}

	SDL_Color aColor = {255, 255, 255, 255};
	SDL_Surface *textSurface = TTF_RenderText_Blended(theFont, theText.c_str(), 0, aColor);

	SDL_Surface *convertedSurface = nullptr;

	GLuint aTexID;
	glGenTextures(1, &aTexID);
	glBindTexture(GL_TEXTURE_2D, aTexID);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, textSurface->pitch / SDL_BYTESPERPIXEL(textSurface->format));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textSurface->w, textSurface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE,
				 textSurface->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	GLTextCacheEntry entry;
	entry.textureID = aTexID;
	entry.mWidth = textSurface->w;
	entry.mHeight = textSurface->h;
	mTextTextureCache[theText] = entry;

	SDL_DestroySurface(textSurface);
	return entry;
}

void GLRenderer::DrawText(int theY, int theX, const PopString &theText, const Color &theColor, TTF_Font *theFont)
{
	GLTextCacheEntry aTextEntry = GetOrCreateText(theText, theFont);

	GLDrawCommand aCmd;
	aCmd.mClipRect = new Rect(0, 0, mPresentationRect.mWidth, mPresentationRect.mHeight);
	aCmd.mTextureID = aTextEntry.textureID;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(Graphics::DRAWMODE_NORMAL);

	glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + aTextEntry.mWidth, theY};
	glm::vec2 p2 = {theX + aTextEntry.mWidth, theY + aTextEntry.mHeight};
	glm::vec2 p3 = {theX, theY + aTextEntry.mHeight};

	glm::vec2 uv0 = {0, 0};
	glm::vec2 uv1 = {1, 0};
	glm::vec2 uv2 = {1, 1};
	glm::vec2 uv3 = {0, 1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({p0, uv0, aColor});
	aCmd.mVertices.push_back({p1, uv1, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p3, uv3, aColor});
	aCmd.mVertices.push_back({p0, uv0, aColor});

	AddCommand(aCmd);
}

void GLRenderer::Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor,
					 int theDrawMode, bool linearFilter)
{
	GLImage *aImg = SetupImage(theImage);

	GLDrawCommand aCmd;
	aCmd.mClipRect = nullptr;
	aCmd.mTextureID = static_cast<GLTextureData *>(aImg->mGPUData)->mTextureID;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + theSrcRect.mWidth, theY};
	glm::vec2 p2 = {theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
	glm::vec2 p3 = {theX, theY + theSrcRect.mHeight};

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({p0, uv0, aColor});
	aCmd.mVertices.push_back({p1, uv1, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p3, uv3, aColor});
	aCmd.mVertices.push_back({p0, uv0, aColor});

	AddCommand(aCmd);
}

void GLRenderer::BltClipF(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect *theClipRect,
						  const Color &theColor, int theDrawMode)
{
	GLImage *aImg = SetupImage(theImage);

	GLDrawCommand aCmd;
	aCmd.mClipRect = theClipRect;
	aCmd.mTextureID = static_cast<GLTextureData *>(aImg->mGPUData)->mTextureID;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + theSrcRect.mWidth, theY};
	glm::vec2 p2 = {theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
	glm::vec2 p3 = {theX, theY + theSrcRect.mHeight};

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({p0, uv0, aColor});
	aCmd.mVertices.push_back({p1, uv1, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p3, uv3, aColor});
	aCmd.mVertices.push_back({p0, uv0, aColor});

	AddCommand(aCmd);
}

void GLRenderer::BltMirror(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Color &theColor,
						   int theDrawMode, bool linearFilter)
{
	GLImage *aImg = SetupImage(theImage);

	GLDrawCommand aCmd;
	aCmd.mClipRect = nullptr;
	aCmd.mTextureID = static_cast<GLTextureData *>(aImg->mGPUData)->mTextureID;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + theSrcRect.mWidth, theY};
	glm::vec2 p2 = {theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
	glm::vec2 p3 = {theX, theY + theSrcRect.mHeight};

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	std::swap(u0, u1);

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({p0, uv0, aColor});
	aCmd.mVertices.push_back({p1, uv1, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p3, uv3, aColor});
	aCmd.mVertices.push_back({p0, uv0, aColor});

	AddCommand(aCmd);
}

void GLRenderer::StretchBlt(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect *theClipRect,
							const Color &theColor, int theDrawMode, bool fastStretch, bool mirror)
{
	GLImage *aImg = SetupImage(theImage);

	GLDrawCommand aCmd;
	aCmd.mClipRect = theClipRect;
	aCmd.mTextureID = static_cast<GLTextureData *>(aImg->mGPUData)->mTextureID;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	glm::vec2 p0 = {theDestRect.mX, theDestRect.mY};
	glm::vec2 p1 = {theDestRect.mX + theDestRect.mWidth, theDestRect.mY};
	glm::vec2 p2 = {theDestRect.mX + theDestRect.mWidth, theDestRect.mY + theDestRect.mHeight};
	glm::vec2 p3 = {theDestRect.mX, theDestRect.mY + theDestRect.mHeight};

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	if (mirror)
	{
		std::swap(u0, u1);
	}

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({p0, uv0, aColor});
	aCmd.mVertices.push_back({p1, uv1, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p3, uv3, aColor});
	aCmd.mVertices.push_back({p0, uv0, aColor});

	AddCommand(aCmd);
}

glm::vec2 RotatePointAroundPivot(const glm::vec2 point, const glm::vec2 center, float angleRad)
{
	float sinValue = sin(angleRad);
	float cosValue = cos(angleRad);

	glm::vec2 translation = point - center;

	glm::vec2 rotation = {translation.x * cosValue - translation.y * sinValue,
						  translation.x * sinValue + translation.y * cosValue};

	return rotation + center;
}

void GLRenderer::BltRotated(Image *theImage, float theX, float theY, const Rect *theClipRect, const Color &theColor,
							int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY,
							const Rect &theSrcRect)
{
	GLImage *aImg = SetupImage(theImage);

	GLDrawCommand aCmd;
	aCmd.mClipRect = theClipRect;
	aCmd.mTextureID = static_cast<GLTextureData *>(aImg->mGPUData)->mTextureID;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	glm::vec2 p0 = {theX, theY};
	glm::vec2 p1 = {theX + theSrcRect.mWidth, theY};
	glm::vec2 p2 = {theX + theSrcRect.mWidth, theY + theSrcRect.mHeight};
	glm::vec2 p3 = {theX, theY + theSrcRect.mHeight};

	float radians = glm::radians(theRot);
	glm::vec2 center = {theRotCenterX + theX, theRotCenterY + theY};
	p0 = RotatePointAroundPivot(p0, center, radians);
	p1 = RotatePointAroundPivot(p1, center, radians);
	p2 = RotatePointAroundPivot(p2, center, radians);
	p3 = RotatePointAroundPivot(p3, center, radians);

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({p0, uv0, aColor});
	aCmd.mVertices.push_back({p1, uv1, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p3, uv3, aColor});
	aCmd.mVertices.push_back({p0, uv0, aColor});

	AddCommand(aCmd);
}

glm::vec2 TransformToPointGLM(float x, float y, const Matrix3 &m, float aTransX = 0, float aTransY = 0)
{
	glm::vec2 result;
	result.x = m.m00 * x + m.m01 * y + m.m02 + aTransX;
	result.y = m.m10 * x + m.m11 * y + m.m12 + aTransY;
	return result;
}

void GLRenderer::BltTransformed(Image *theImage, const Rect *theClipRect, const Color &theColor, int theDrawMode,
								const Rect &theSrcRect, const Matrix3 &theTransform, bool linearFilter, float theX,
								float theY, bool center)
{
	GLImage *aImg = SetupImage(theImage);

	GLDrawCommand aCmd;
	aCmd.mClipRect = theClipRect;
	aCmd.mTextureID = static_cast<GLTextureData *>(aImg->mGPUData)->mTextureID;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	float aWidth = static_cast<float>(theSrcRect.mWidth);
	float aHeight = static_cast<float>(theSrcRect.mHeight);

	glm::vec2 origin = {0.0f, 0.0f};
	if (center)
		origin = {aWidth * 0.5f, aHeight * 0.5f};

	glm::vec2 localP0 = {-origin.x, -origin.y};
	glm::vec2 localP1 = {aWidth - origin.x, -origin.y};
	glm::vec2 localP2 = {aWidth - origin.x, aHeight - origin.y};
	glm::vec2 localP3 = {-origin.x, aHeight - origin.y};

	glm::vec2 p0 = TransformToPointGLM(localP0.x, localP0.y, theTransform, theX, theY);
	glm::vec2 p1 = TransformToPointGLM(localP1.x, localP1.y, theTransform, theX, theY);
	glm::vec2 p2 = TransformToPointGLM(localP2.x, localP2.y, theTransform, theX, theY);
	glm::vec2 p3 = TransformToPointGLM(localP3.x, localP3.y, theTransform, theX, theY);

	float u0 = (float)theSrcRect.mX / (float)theImage->mWidth;
	float v0 = (float)theSrcRect.mY / (float)theImage->mHeight;
	float u1 = (float)(theSrcRect.mX + theSrcRect.mWidth) / (float)theImage->mWidth;
	float v1 = (float)(theSrcRect.mY + theSrcRect.mHeight) / (float)theImage->mHeight;

	glm::vec2 uv0 = {u0, v0};
	glm::vec2 uv1 = {u1, v0};
	glm::vec2 uv2 = {u1, v1};
	glm::vec2 uv3 = {u0, v1};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({p0, uv0, aColor});
	aCmd.mVertices.push_back({p1, uv1, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p2, uv2, aColor});
	aCmd.mVertices.push_back({p3, uv3, aColor});
	aCmd.mVertices.push_back({p0, uv0, aColor});

	AddCommand(aCmd);
}

void GLRenderer::DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor,
						  int theDrawMode)
{
	GLDrawCommand aCmd;
	aCmd.mPrimitiveType = GL_LINES;
	aCmd.mTextureID = 0;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);
	aCmd.mClipRect = new Rect(0, 0, mPresentationRect.mWidth, mPresentationRect.mHeight);
	glm::vec4 color =
		glm::vec4(theColor.mRed / 255.0f, theColor.mGreen / 255.0f, theColor.mBlue / 255.0f, theColor.mAlpha / 255.0f);
	aCmd.mVertices.push_back({{theStartX, theStartY}, {}, color});
	aCmd.mVertices.push_back({{theEndX, theEndY}, {}, color});

	AddCommand(aCmd);
}

void GLRenderer::FillRect(const Rect &theRect, const Color &theColor, int theDrawMode)
{
	GLDrawCommand aCmd;
	aCmd.mClipRect = new Rect(0, 0, mPresentationRect.mWidth, mPresentationRect.mHeight);
	aCmd.mTextureID = 0;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	glm::vec2 p0 = {theRect.mX, theRect.mY};
	glm::vec2 p1 = {theRect.mX + theRect.mWidth, theRect.mY};
	glm::vec2 p2 = {theRect.mX + theRect.mWidth, theRect.mY + theRect.mHeight};
	glm::vec2 p3 = {theRect.mX, theRect.mY + theRect.mHeight};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({p0, {}, aColor});
	aCmd.mVertices.push_back({p1, {}, aColor});
	aCmd.mVertices.push_back({p2, {}, aColor});
	aCmd.mVertices.push_back({p2, {}, aColor});
	aCmd.mVertices.push_back({p3, {}, aColor});
	aCmd.mVertices.push_back({p0, {}, aColor});

	AddCommand(aCmd);
}

void GLRenderer::DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor,
							  int theDrawMode)
{
	GLDrawCommand aCmd;
	aCmd.mClipRect = new Rect(0, 0, mPresentationRect.mWidth, mPresentationRect.mHeight);
	aCmd.mTextureID = 0;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	glm::vec2 vert0 = {p1.x, p1.y};
	glm::vec2 vert1 = {p2.x, p2.y};
	glm::vec2 vert2 = {p3.x, p3.y};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({vert0, {p1.u, p1.v}, aColor});
	aCmd.mVertices.push_back({vert1, {p2.u, p2.v}, aColor});
	aCmd.mVertices.push_back({vert2, {p3.u, p3.v}, aColor});

	AddCommand(aCmd);
}

void GLRenderer::DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor,
								 int theDrawMode, Image *theTexture, bool blend)
{
	GLImage *aImg = SetupImage(theTexture);

	GLDrawCommand aCmd;
	aCmd.mClipRect = new Rect(0, 0, mPresentationRect.mWidth, mPresentationRect.mHeight);
	aCmd.mTextureID = static_cast<GLTextureData *>(aImg->mGPUData)->mTextureID;
	aCmd.mPrimitiveType = GL_TRIANGLES;
	aCmd.mBlendMode = ChooseBlendMode(theDrawMode);

	glm::vec2 vert0 = {p1.x, p1.y};
	glm::vec2 vert1 = {p2.x, p2.y};
	glm::vec2 vert2 = {p3.x, p3.y};

	glm::vec4 aColor = {(float)theColor.mRed / 255.0f, (float)theColor.mGreen / 255.0f, (float)theColor.mBlue / 255.0f,
						(float)theColor.mAlpha / 255.0f};

	aCmd.mVertices.push_back({vert0, {p1.u, p1.v}, aColor});
	aCmd.mVertices.push_back({vert1, {p2.u, p2.v}, aColor});
	aCmd.mVertices.push_back({vert2, {p3.u, p3.v}, aColor});

	AddCommand(aCmd);
}

void GLRenderer::DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor,
								  int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
	for (int aTriangleNum = 0; aTriangleNum < theNumTriangles; aTriangleNum++)
	{
		TriVertex v0 = theVertices[aTriangleNum][0];
		TriVertex v1 = theVertices[aTriangleNum][1];
		TriVertex v2 = theVertices[aTriangleNum][2];

		v0.x += tx;
		v0.y += ty;
		v1.x += tx;
		v1.y += ty;
		v2.x += tx;
		v2.y += ty;

		DrawTriangleTex(v0, v1, v2, theColor, theDrawMode, theTexture, blend);
	}
}

void GLRenderer::DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor,
									   int theDrawMode, Image *theTexture, float tx, float ty, bool blend)
{
	TriVertex aList[100][3];
	int aTriNum = 0;
	while (aTriNum < theNumTriangles)
	{
		int aMaxTriangles = std::min(100, theNumTriangles - aTriNum);
		for (int i = 0; i < aMaxTriangles; i++)
		{
			aList[i][0] = theVertices[aTriNum];
			aList[i][1] = theVertices[aTriNum + 1];
			aList[i][2] = theVertices[aTriNum + 2];
			aTriNum++;
		}
		DrawTrianglesTex(aList, aMaxTriangles, theColor, theDrawMode, theTexture, tx, ty, blend);
	}
}

void GLRenderer::FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor,
						  int theDrawMode, int tx, int ty)
{
	if (theNumVertices < 3)
		return;

	for (int i = 1; i < theNumVertices - 1; ++i)
	{
		TriVertex v0, v1, v2;

		v0.x = theVertices[0].mX + tx;
		v0.y = theVertices[0].mY + ty;

		v1.x = theVertices[i].mX + tx;
		v1.y = theVertices[i].mY + ty;

		v2.x = theVertices[i + 1].mX + tx;
		v2.y = theVertices[i + 1].mY + ty;

		DrawTriangle(v0, v1, v2, theColor, theDrawMode);
	}
}

void GLRenderer::BltTexture(Texture *theTexture, const Rect &theSrcRect, const Rect &theDestRect, const Color &theColor,
							int theDrawMode)
{
}
