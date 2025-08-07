#ifndef __FLAGS_HPP__
#define __FLAGS_HPP__

#pragma once

namespace PopLib
{

class FlagsMod
{
  public:
	int mAddFlags;
	int mRemoveFlags;

  public:
	FlagsMod()
	{
		mAddFlags = 0;
		mRemoveFlags = 0;
	}
};

inline void ModFlags(int &theFlags, const FlagsMod &theFlagMod)
{
	theFlags = (theFlags | theFlagMod.mAddFlags) & ~theFlagMod.mRemoveFlags;
}

inline int GetModFlags(int theFlags, const FlagsMod &theFlagMod)
{
	return (theFlags | theFlagMod.mAddFlags) & ~theFlagMod.mRemoveFlags;
}

class ModalFlags
{
  public:
	int mOverFlags;
	int mUnderFlags;
	bool mIsOver;

  public:
	void ModFlags(const FlagsMod &theFlagsMod)
	{
		PopLib::ModFlags(mOverFlags, theFlagsMod);
		PopLib::ModFlags(mUnderFlags, theFlagsMod);
	}

	int GetFlags()
	{
		return mIsOver ? mOverFlags : mUnderFlags;
	}
};

class AutoModalFlags
{
  public:
	ModalFlags *mModalFlags;
	int mOldOverFlags;
	int mOldUnderFlags;

  public:
	AutoModalFlags(ModalFlags *theModalFlags, const FlagsMod &theFlagMod)
	{
		mModalFlags = theModalFlags;
		mOldOverFlags = theModalFlags->mOverFlags;
		mOldUnderFlags = theModalFlags->mUnderFlags;
		theModalFlags->ModFlags(theFlagMod);
	}

	~AutoModalFlags()
	{
		mModalFlags->mOverFlags = mOldOverFlags;
		mModalFlags->mUnderFlags = mOldUnderFlags;
	}
};

} // namespace PopLib

#endif // __FLAGS_HPP__