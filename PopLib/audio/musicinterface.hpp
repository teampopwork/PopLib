#ifndef __MUSICINTERFACE_HPP__
#define __MUSICINTERFACE_HPP__

#pragma once

#include "common.hpp"

namespace PopLib
{

struct StreamData
{
    unsigned char *mStreamData = nullptr;
    void *mStreamHandle = nullptr;
};

/**
 * @brief music interface
 *
 * parent
 */
class MusicInterface
{
  public:
	/// @brief constructor
	MusicInterface();
	/// @brief destructor
	virtual ~MusicInterface();

	/// @brief loads music by id
	/// @param theSongId
	/// @param theFileName
	/// @return true if success
	virtual bool LoadMusic(int theSongId, const std::string &theFileName);
	/// @brief plays music by id
	/// @param theSongId
	/// @param theOffset
	/// @param noLoop
	virtual void PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false);
	/// @brief stops music by id
	/// @param theSongId
	virtual void StopMusic(int theSongId);
	/// @brief stops all music
	virtual void StopAllMusic();
	/// @brief unloads music by id
	/// @param theSongId
	virtual void UnloadMusic(int theSongId);
	/// @brief unloads all music
	virtual void UnloadAllMusic();
	/// @brief pause all music
	virtual void PauseAllMusic();
	/// @brief resume all music
	virtual void ResumeAllMusic();
	/// @brief pauses music by id
	/// @param theSongId
	virtual void PauseMusic(int theSongId);
	/// @brief resumes music by id
	/// @param theSongId
	virtual void ResumeMusic(int theSongId);
	/// @brief fades in music by id
	/// @param theSongId
	/// @param theOffset
	/// @param theSpeed
	/// @param noLoop
	virtual void FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false);
	/// @brief fades out music by id
	/// @param theSongId
	/// @param stopSong
	/// @param theSpeed
	virtual void FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004);
	/// @brief fades out all music
	/// @param stopSong
	/// @param theSpeed
	virtual void FadeOutAll(bool stopSong = true, double theSpeed = 0.004);
	/// @brief sets song volume by id
	/// @param theSongId
	/// @param theVolume
	virtual void SetSongVolume(int theSongId, double theVolume);
	/// @brief sets song maximum volume by id
	/// @param theSongId
	/// @param theMaxVolume
	virtual void SetSongMaxVolume(int theSongId, double theMaxVolume);
	/// @brief is song by id playing?
	/// @param theSongId
	/// @return true if yes
	virtual bool IsPlaying(int theSongId);

	/// @brief sets global volume
	/// @param theVolume
	virtual void SetVolume(double theVolume);
	/// @brief sets music amplify, default is 0.50
	/// @param theSongId
	/// @param theAmp
	virtual void SetMusicAmplify(int theSongId, double theAmp);
	/// @brief music update
	virtual void Update();
};

} // namespace PopLib

#endif