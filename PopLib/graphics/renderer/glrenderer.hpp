#ifndef __GLRENDERER_HPP__
#define __GLRENDERER_HPP__

#pragma once

#include "graphics/renderer.hpp"
#include "glshader.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include "glimage.hpp"

#include <unordered_map>

namespace PopLib
{
class AppBase;

struct GLBlendFunc
{
	GLenum src;
	GLenum dst;
	bool enable_blend = true;
};

inline const std::unordered_map<BlendMode, GLBlendFunc> blend_mode_funcs = {
	{BlendMode::BLENDMODE_BLEND, {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, true}},
	{BlendMode::BLENDMODE_ADD, {GL_SRC_ALPHA, GL_ONE, true}},
	{BlendMode::BLENDMODE_MUL, {GL_DST_COLOR, GL_ZERO, true}}};

struct Vertex
{
	glm::vec2 mPos;
	glm::vec2 mTexCoord;
	glm::vec4 mColor;
};

struct GLTextCacheEntry
{
	GLuint textureID;
	int mWidth;
	int mHeight;
};

struct GLDrawCommand
{
	GLenum mPrimitiveType;
	GLuint mTextureID;
	BlendMode mBlendMode;
	std::vector<Vertex> mVertices;
	GLShader *mShader = nullptr;
	const Rect *mClipRect = nullptr;
};

struct GLTextureData
{
  public:
	GLuint mTextureID;
	int mWidth;
	int mHeight;
	int mBitsChangedCount;

	GLTextureData();
	~GLTextureData();

	void ReleaseTextures();

	void CreateTextures(GLImage *theImage);
	void CheckCreateTextures(GLImage *theImage);

	int GetMemSize();
};

typedef std::set<GLImage *> GLImageSet;

class GLRenderer : public Renderer
{
  public:
	GLuint mVAO;
	GLuint mVBO;
	SDL_GLContext mContext;
	std::vector<GLDrawCommand> mCommandBuffer;
	GLShader *mDefaultShader;
	GLImageSet mImageSet;
	glm::mat4 mProjection;
	std::unordered_map<std::string, GLTextCacheEntry> mTextTextureCache;

  public:
	GLRenderer(AppBase *theApp);
	virtual ~GLRenderer();

	virtual void Cleanup();

	virtual void AddImage(Image *theImage);
	virtual void RemoveImage(Image *theImage);
	virtual void Remove3DData(GPUImage *theImage);

	virtual GPUImage *NewGPUImage()
	{
		return new GLImage(this);
	}

	virtual void GetOutputSize(int *outWidth, int *outHeight);

	virtual GPUImage *GetScreenImage();
	virtual void UpdateViewport();
	virtual int Init();

	bool InitGLContext();
	bool InitBuffers();
	GLImage *SetupImage(Image *theImage);

	virtual bool Redraw(Rect *theClipRect);
	virtual void SetVideoOnlyDraw(bool videoOnly);

	virtual std::unique_ptr<ImageData> CaptureFrameBuffer();

	virtual bool PreDraw();

	virtual bool CreateImageTexture(GPUImage *theImage);
	virtual bool RecoverBits(GPUImage *theImage);

	GLTextCacheEntry GetOrCreateText(const std::string &theText, TTF_Font *theFont);
	virtual void DrawText(int theY, int theX, const PopString &theText, const Color &theColor, TTF_Font *theFont);
	void AddCommand(const GLDrawCommand &command);
	void ApplyBlendMode(BlendMode mode);

	virtual void Blt(Image *theImage, int theX, int theY, const Rect &theSrcRect, const Color &theColor,
					 int theDrawMode, bool linearFilter = false);

	virtual void BltClipF(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Rect *theClipRect,
						  const Color &theColor, int theDrawMode);

	virtual void BltMirror(Image *theImage, float theX, float theY, const Rect &theSrcRect, const Color &theColor,
						   int theDrawMode, bool linearFilter = false);

	virtual void StretchBlt(Image *theImage, const Rect &theDestRect, const Rect &theSrcRect, const Rect *theClipRect,
							const Color &theColor, int theDrawMode, bool fastStretch, bool mirror = false);

	virtual void BltRotated(Image *theImage, float theX, float theY, const Rect *theClipRect, const Color &theColor,
							int theDrawMode, double theRot, float theRotCenterX, float theRotCenterY,
							const Rect &theSrcRect);

	virtual void BltTransformed(Image *theImage, const Rect *theClipRect, const Color &theColor, int theDrawMode,
								const Rect &theSrcRect, const Matrix3 &theTransform, bool linearFilter, float theX = 0,
								float theY = 0, bool center = false);

	virtual void DrawLine(double theStartX, double theStartY, double theEndX, double theEndY, const Color &theColor,
						  int theDrawMode);

	virtual void FillRect(const Rect &theRect, const Color &theColor, int theDrawMode);

	virtual void DrawTriangle(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor,
							  int theDrawMode);

	virtual void DrawTriangleTex(const TriVertex &p1, const TriVertex &p2, const TriVertex &p3, const Color &theColor,
								 int theDrawMode, Image *theTexture, bool blend = true);

	virtual void DrawTrianglesTex(const TriVertex theVertices[][3], int theNumTriangles, const Color &theColor,
								  int theDrawMode, Image *theTexture, float tx = 0, float ty = 0, bool blend = true);

	virtual void DrawTrianglesTexStrip(const TriVertex theVertices[], int theNumTriangles, const Color &theColor,
									   int theDrawMode, Image *theTexture, float tx = 0, float ty = 0,
									   bool blend = true);

	virtual void FillPoly(const Point theVertices[], int theNumVertices, const Rect *theClipRect, const Color &theColor,
						  int theDrawMode, int tx, int ty);

	virtual void BltTexture(Texture *theTexture, const Rect &theSrcRect, const Rect &theDestRect, const Color &theColor,
							int theDrawMode);
};

} // namespace PopLib

#endif // __GLRENDERER_HPP__