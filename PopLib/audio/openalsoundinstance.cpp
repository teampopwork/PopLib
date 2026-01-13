#include "openalsoundinstance.hpp"
#include "openalsoundmanager.hpp"
#include <complex.h>

namespace PopLib
{

OpenALSoundInstance::OpenALSoundInstance(OpenALSoundManager *theSoundManager, ALuint theSourceSound)
{
	mSoundManagerP = theSoundManager;
	mReleased = false;
	mAutoRelease = false;
	mHasPlayed = false;
	mSourceSoundBuffer = theSourceSound;
	mSoundSource = 0;

	mBaseVolume = 1.0;
	mBasePan = 0;

	mVolume = 1.0;
	mPan = 0;

	mDefaultFrequency = 44100;

	// Generate the OpenAL source.
	if (mSourceSoundBuffer != 0)
	{
		alGenSources(1, &mSoundSource);
		alSourcei(mSoundSource, AL_BUFFER, mSourceSoundBuffer);
	}

	RehupVolume();
}

OpenALSoundInstance::~OpenALSoundInstance()
{
	alSourceStop(mSoundSource);			   // Stop the source first
	alSourcei(mSoundSource, AL_BUFFER, 0); // Detach buffer
	alDeleteSources(1, &mSoundSource);	   /// Delete the source
	mSoundSource = 0;
}

void OpenALSoundInstance::RehupVolume()
{
	if (mSoundSource != 0)
	{
		alSourcef(mSoundSource, AL_GAIN, mVolume * mBaseVolume * mSoundManagerP->mMasterVolume);
	}
}

void OpenALSoundInstance::RehupPan()
{
	if (mSoundSource)
	{
		float converted_panning = (mBasePan + mPan) / 10000.0f;
		if (converted_panning < -1.0f)
			converted_panning = -1.0f;
		if (converted_panning > 1.0f)
			converted_panning = 1.0f;

		alSource3f(mSoundSource, AL_POSITION, converted_panning, 0.0f, 0.0f);
	}
}

void OpenALSoundInstance::Release()
{
	Stop();
	mReleased = true;
}

void OpenALSoundInstance::SetBaseVolume(double theBaseVolume)
{
	mBaseVolume = theBaseVolume;
	RehupVolume();
}

void OpenALSoundInstance::SetBasePan(int theBasePan)
{
	mBasePan = theBasePan;
	RehupPan();
}

void OpenALSoundInstance::SetVolume(double theVolume)
{
	mVolume = theVolume;
	RehupVolume();
}

void OpenALSoundInstance::SetPan(int thePosition)
{
	mPan = thePosition;
	RehupPan();
}

bool OpenALSoundInstance::Play(bool looping, bool autoRelease)
{
	if (!mSoundSource)
		return false;

	if (!mSoundManagerP->mALDeviceD) // hacky hack
		return false;

	Stop();

	mHasPlayed = true;
	mAutoRelease = autoRelease;

	alSourcei(mSoundSource, AL_LOOPING, looping);
	alSourcePlay(mSoundSource);
	return true;
}

void OpenALSoundInstance::Stop()
{
	if (!mSoundSource)
		return;

	if (!mSoundManagerP->mALDeviceD) // hacky hack
		return;

	alSourceStop(mSoundSource);
	alSourcei(mSoundSource, AL_SEC_OFFSET, 0);
	mAutoRelease = false;
}

void OpenALSoundInstance::AdjustPitch(double theNumSteps)
{
	if (mSoundSource)
	{
		// 1.059463..... is the twelfth root of 2, which is the how many semitones per steps.
		double aFrequencyMult = std::pow(1.0594630943592952645618252949463, theNumSteps);
		alSourcef(mSoundSource, AL_PITCH, aFrequencyMult);
	}
}

bool OpenALSoundInstance::IsPlaying()
{
	if (!mHasPlayed)
		return false;

	if (!mSoundSource)
		return false;

	ALint aStatus;
	alGetSourcei(mSoundSource, AL_SOURCE_STATE, &aStatus);
	if (aStatus == AL_PLAYING)
		return true;
	else
		return false;
}

bool OpenALSoundInstance::IsReleased()
{
	if ((!mReleased) && (mAutoRelease) && (mHasPlayed) && (!IsPlaying()))
		Release();

	return mReleased;
}

double OpenALSoundInstance::GetVolume()
{
	return mVolume;
}

}; // namespace PopLib