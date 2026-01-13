#include "imguimanager.hpp"
#include "appbase.hpp"

// renderers
#include "graphics/renderer/glrenderer.hpp"
#include "graphics/renderer/sdlrenderer.hpp"

using namespace PopLib;

bool demoWind = false;

////////////////////////////

static std::vector<ImGuiWindowEntry> gWindowList;

void RegisterImGuiWindow(const char *name, bool *enabled, const ImGuiManager::WindowFunction &func)
{
	gWindowList.push_back({name, enabled, func});
}

void RegisterImGuiWindows()
{
	for (auto &entry : gWindowList)
	{
		if (entry.enabled && *entry.enabled)
		{
			gAppBase->mIGUIManager->AddWindow(entry.func);
		}
	}
}

////////////////////////////

ImGuiManager::ImGuiManager(Renderer *theInterface)
{
	mRenderer = theInterface;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io; // uhhhhhh
	ImGui::StyleColorsDark();

	switch (gAppBase->mRendererAPI)
	{
		case Renderers::SDL:
		{
			SDLRenderer *aInterface = (SDLRenderer*)mRenderer;
			ImGui_ImplSDL3_InitForSDLRenderer(gAppBase->mWindow, aInterface->mRenderer);
			ImGui_ImplSDLRenderer3_Init(aInterface->mRenderer);
			break;
		}
		case Renderers::OpenGL:
		{
			GLRenderer *aInterface = (GLRenderer*)mRenderer;
			ImGui_ImplSDL3_InitForOpenGL(gAppBase->mWindow, aInterface->mContext);
			ImGui_ImplOpenGL3_Init();
			break;
		}
		default:
		{
			// in appbase.cpp we're using SDL as default, so do we here
			SDLRenderer *aInterface = (SDLRenderer*)mRenderer;
			ImGui_ImplSDL3_InitForSDLRenderer(gAppBase->mWindow, aInterface->mRenderer);
			ImGui_ImplSDLRenderer3_Init(aInterface->mRenderer);
			break;
		}
	}
}

void ImGuiManager::RenderAll(void)
{
	// simple yet effective
	for (const auto &entry : gWindowList)
	{
		if (entry.enabled && *entry.enabled)
			entry.func();
	}
}

void ImGuiManager::Frame(void)
{
	switch (gAppBase->mRendererAPI)
	{
		case Renderers::SDL:
		{
			ImGui_ImplSDLRenderer3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			break;
		}
		case Renderers::OpenGL:
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			break;
		}
		default:
		{
			// in appbase.cpp we're using SDL as default, so do we here
			ImGui_ImplSDLRenderer3_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			break;
		}
	}
	ImGui::NewFrame();

	if (demoWind)
		ImGui::ShowDemoWindow();

	RenderAll();

	ImGui::Render();
}

ImGuiManager::~ImGuiManager()
{
	switch (gAppBase->mRendererAPI)
	{
		case Renderers::SDL:
		{
			ImGui_ImplSDLRenderer3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			break;
		}
		case Renderers::OpenGL:
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			break;
		}
		default:
		{
			// in appbase.cpp we're using SDL as default, so do we here
			ImGui_ImplSDLRenderer3_Shutdown();
			ImGui_ImplSDL3_Shutdown();
			break;
		}
	}

	ImGui::DestroyContext();
}