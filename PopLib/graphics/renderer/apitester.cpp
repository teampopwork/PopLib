#include "apitester.hpp"

using namespace PopLib;

bool APITester::IsSDLRendererAvailable(SDL_Window *window)
{
	return SDL_GetNumRenderDrivers() > 0;
}

bool APITester::IsOpenGLAvailable(SDL_Window *window)
{
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == nullptr)
		return false;

	SDL_GL_DestroyContext(context);

	return true;
}

bool APITester::IsDirect3DAvailable(SDL_Window *window)
{
#ifdef _WIN32
	// TODO: Implement Direct3D renderer
	return false;
#else
	return false;
#endif
}