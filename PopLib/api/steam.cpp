#include "steam.hpp"
#include <SDL3/SDL.h>
#include <fstream>

using namespace PopLib;

SteamAPI::SteamAPI()
{
}

SteamAPI::~SteamAPI()
{
	Shutdown();
}

bool SteamAPI::Init(const std::string &appId)
{
	if (!appId.empty())
	{
		SDL_Log("SteamAPI: Writing appid.txt with App ID %s", appId.c_str());

		std::ofstream appidFile("steam_appid.txt", std::ios::out | std::ios::trunc);
		if (appidFile.is_open())
		{
			appidFile << appId;
			appidFile.close();
			SDL_Log("SteamAPI: steam_appid.txt written successfully.");
		}
		else
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SteamAPI: Failed to write steam_appid.txt");
			return false;
		}
	}

	mInitialized = SteamAPI_Init();
	if (!mInitialized)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SteamAPI: Initialization failed");
		return false;
	}

	SDL_Log("SteamAPI: Initialized successfully. User: %s", GetPersonaName().c_str());
	return true;
}

void SteamAPI::Shutdown()
{
	if (mInitialized)
	{
		SteamAPI_Shutdown();
		SDL_Log("SteamAPI: Shutdown complete");
		mInitialized = false;
	}
}

void SteamAPI::RunCallbacks()
{
	if (mInitialized)
		SteamAPI_RunCallbacks();
}

bool SteamAPI::IsLoggedIn() const
{
	return mInitialized && SteamUser() && SteamUser()->BLoggedOn();
}

std::string SteamAPI::GetPersonaName() const
{
	if (SteamFriends())
		return SteamFriends()->GetPersonaName();
	return {};
}

CSteamID SteamAPI::GetSteamID() const
{
	if (SteamUser())
		return SteamUser()->GetSteamID();
	return {};
}

bool SteamAPI::SetAchievement(const char *name)
{
	if (!SteamUserStats())
		return false;
	bool res = SteamUserStats()->SetAchievement(name);
	if (res)
		SteamUserStats()->StoreStats();
	return res;
}

bool SteamAPI::ClearAchievement(const char *name)
{
	if (!SteamUserStats())
		return false;
	bool res = SteamUserStats()->ClearAchievement(name);
	if (res)
		SteamUserStats()->StoreStats();
	return res;
}

bool SteamAPI::StoreStats()
{
	return SteamUserStats() && SteamUserStats()->StoreStats();
}

bool SteamAPI::SetRichPresence(const char *key, const char *value)
{
	return SteamFriends() && SteamFriends()->SetRichPresence(key, value);
}

void SteamAPI::ClearRichPresence()
{
	if (SteamFriends())
		SteamFriends()->ClearRichPresence();
}
