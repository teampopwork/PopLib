#include "steam.hpp"
#include "debug/log.hpp"
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
		LOG_INFO("SteamAPI: Writing appid.txt with App ID %s", appId.c_str());

		std::ofstream appidFile("steam_appid.txt", std::ios::out | std::ios::trunc);
		if (appidFile.is_open())
		{
			appidFile << appId;
			appidFile.close();
			LOG_INFO("SteamAPI: steam_appid.txt written successfully");
		}
		else
		{
			LOG_ERROR("SteamAPI: Failed to write steam_appid.txt");
			return false;
		}
	}

	mInitialized = SteamAPI_Init();
	if (!mInitialized)
	{
		LOG_INFO("SteamAPI: Initialization failed");
		return false;
	}

	LOG_INFO("SteamAPI: Initialized successfully. User: %s", GetPersonaName().c_str());
	return true;
}

void SteamAPI::Shutdown()
{
	if (mInitialized)
	{
		SteamAPI_Shutdown();
		LOG_INFO("SteamAPI: Shutdown complete");
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
