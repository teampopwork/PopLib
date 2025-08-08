#define POPLIB_SKIP_INT64_TYPEDEF
#include <SDL3/SDL.h>
#include "discord.hpp"
#include "debug/debug.hpp"

using namespace PopLib;

DiscordRPC::DiscordRPC(const char *theAppID)
{
	mStartTime = SDL_GetTicks();
	mRPCData = {"test", "test", "icon", "icon"};
	mSendRPC = true;
	mAppID = theAppID;
	mHasInitialized = false;
	InitRPC();
}

DiscordRPC::~DiscordRPC()
{
	discord::RPCManager::get().shutdown();
}

bool DiscordRPC::InitRPC()
{
	discord::RPCManager::get()
		.setClientID(mAppID)
		.onReady([](discord::User const &user) {
			SDL_Log("Discord: connected to user %s#%s - %s", user.username.c_str(), user.discriminator.c_str(),
					user.id.c_str());
		})
		.onDisconnected([](int errcode, std::string_view message) {
			SDL_Log("Discord: disconnected with error code %i - %s", errcode, std::string(message));
		})
		.onErrored([](int errcode, std::string_view message) {
			SDL_Log("Discord: error with code %i - %s", errcode, std::string(message));
		})
		.onJoinGame([](std::string_view joinSecret) { SDL_Log("Discord: join game - %s", std::string(joinSecret)); })
		.onSpectateGame([](std::string_view spectateSecret) {
			SDL_Log("Discord: spectate game - %s", std::string(spectateSecret));
		})
		.onJoinRequest([](discord::User const &user) {
			SDL_Log("Discord: join request from %s#%s - %s", user.username.c_str(), user.discriminator.c_str(),
					user.id.c_str());
		});
	discord::RPCManager::get().initialize();
	mHasInitialized = true;
	return true;
}

void DiscordRPC::UpdateRPC()
{
	if (!mSendRPC)
	{
		discord::RPCManager::get().clearPresence();
		return;
	}
	auto &rpc = discord::RPCManager::get();

	std::time_t current_time = std::time(nullptr);
	Uint32 elapsed_ms = SDL_GetTicks() - mStartTime;
	std::time_t startTimestamp = current_time - (elapsed_ms / 1000);

	rpc.getPresence()
		.setState(mRPCData.mState)
		.setDetails(mRPCData.mDetails)
		.setStartTimestamp(startTimestamp)
		.setEndTimestamp(current_time)
		.setLargeImageKey(mRPCData.mLargeImageName)
		.setSmallImageKey(mRPCData.mSmallImageName)
		.setInstance(false)
		.refresh();
}