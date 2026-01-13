#ifndef __STEAM_HPP__
#define __STEAM_HPP__

#pragma once

#include <steam/steam_api.h>
#include <string>

namespace PopLib
{

class SteamAPI
{
  public:
	SteamAPI();
	~SteamAPI();

	bool Init(const std::string &appId = "");
	void Shutdown();
	void RunCallbacks();

	bool IsInitialized() const
	{
		return mInitialized;
	}
	bool IsLoggedIn() const;
	std::string GetPersonaName() const;
	CSteamID GetSteamID() const;

	bool SetAchievement(const char *name);
	bool ClearAchievement(const char *name);
	bool StoreStats();

	bool SetRichPresence(const char *key, const char *value);
	void ClearRichPresence();

  private:
	bool mInitialized = false;
};

} // namespace PopLib

#endif
