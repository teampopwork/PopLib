#ifndef __OPENMPTMUSICINTERFACE_HPP__
#define __OPENMPTMUSICINTERFACE_HPP__

#pragma once

#include "musicinterface.hpp"
#include "miniaudio.h"
#include <map>
#include <string>
#include <mutex>
#include <memory>
#include <cstdint>

namespace PopLib
{

/**
 * @brief openmpt music info
 */
struct OpenMPTMusicInfo
{
	void *mHMusic;		 // libopenmpt module ptr if module
	ma_decoder mDecoder; // miniaudio decoder for MP3/WAV/etc
	bool mIsModule;		 // true = libopenmpt module, false = miniaudio decoder

	StreamData mStream;
	double mVolume;
	double mVolumeAdd;
	double mVolumeCap;
	bool mStopOnFade;

	std::mutex mDecoderMutex; // protects mDecoder / seek / uninit / read
	bool mLoop;

	OpenMPTMusicInfo();
};

/// @brief list (store pointers to avoid copying non-copyable members)
typedef std::map<int, std::unique_ptr<OpenMPTMusicInfo>> OpenMPTMusicMap;

/**
 * @brief OpenMPT music interface
 */
class OpenMPTMusicInterface : public MusicInterface
{
  public:
	/// @brief list
	OpenMPTMusicMap mMusicMap;
	/// @brief maximum music volume (compat)
	int mMaxMusicVolume;
	/// @brief music loading flags (compat)
	int mMusicLoadFlags;

  public:
	/// @brief constructor
	OpenMPTMusicInterface();
	/// @brief destructor
	virtual ~OpenMPTMusicInterface();

	/// @brief loads music by id
	virtual bool LoadMusic(int theSongId, const std::string &theFileName);
	/// @brief plays music by id
	virtual void PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false);
	/// @brief stops music by id
	virtual void StopMusic(int theSongId);
	/// @brief stops all music
	virtual void StopAllMusic();
	/// @brief unloads music by id
	virtual void UnloadMusic(int theSongId);
	/// @brief unloads all music
	virtual void UnloadAllMusic();
	/// @brief pause all music
	virtual void PauseAllMusic();
	/// @brief resume all music
	virtual void ResumeAllMusic();
	/// @brief pauses music by id
	virtual void PauseMusic(int theSongId);
	/// @brief resumes music by id
	virtual void ResumeMusic(int theSongId);
	/// @brief fades in music by id
	virtual void FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false);
	/// @brief fades out music by id
	virtual void FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004);
	/// @brief fades out all music
	virtual void FadeOutAll(bool stopSong = true, double theSpeed = 0.004);
	/// @brief sets song volume by id
	virtual void SetSongVolume(int theSongId, double theVolume);
	/// @brief sets song maximum volume by id
	virtual void SetSongMaxVolume(int theSongId, double theMaxVolume);
	/// @brief is song by id playing?
	virtual bool IsPlaying(int theSongId);

	/// @brief sets global volume
	virtual void SetVolume(double theVolume);
	/// @brief sets music amplify
	virtual void SetMusicAmplify(int theSongId, double theAmp);
	/// @brief music update (call from main thread)
	virtual void Update();

	/// @brief functions for dealing with MODs
	int GetMusicOrder(int theSongId);

  public:
	/// @brief start the miniaudio device
	void StartDevice();
	/// @brief stop the miniaudio device
	void StopDevice();

  private:
	/// @brief callback invoked from miniaudio (defined in .cpp)
	void dataCallback(float *outF32, ma_uint32 frameCount);

	/// @brief miniaudio device object (header includes miniaudio.h so this is known)
	ma_device mDevice;

	/// @brief whether mDevice was successfully initialized
	bool mDeviceInitialized;

	/// @brief mutex protecting mMusicMap and other state
	std::mutex mMutex;
};

} // namespace PopLib

#endif // __OPENMPTMUSICINTERFACE_HPP__
