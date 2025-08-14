#define MINIAUDIO_IMPLEMENTATION
#include "openmptmusicinterface.hpp"
#include "debug/log.hpp"

#include <libopenmpt/libopenmpt.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <SDL3/SDL.h>

namespace PopLib
{

struct ScratchBuffers
{
	std::vector<float> left;
	std::vector<float> right;
	void ensure_capacity(size_t frames)
	{
		if (left.size() < frames)
			left.resize(frames);
		if (right.size() < frames)
			right.resize(frames);
	}
};

static std::mutex g_globalMutex;
static std::unordered_map<OpenMPTMusicInfo *, ScratchBuffers> g_scratchMap;
static double g_masterVolume = 1.0;

OpenMPTMusicInfo::OpenMPTMusicInfo()
{
	mVolume = 0.0; // start at 0 like BassMusicInterface
	mVolumeAdd = 0.0;
	mVolumeCap = 1.0;
	mStopOnFade = false;
	mHMusic = nullptr;
	mIsModule = false;
	mLoop = false;
	mStream = {nullptr, nullptr};
	memset(&mDecoder, 0, sizeof(mDecoder));
}

OpenMPTMusicInterface::OpenMPTMusicInterface()
{
	mMaxMusicVolume = 40;
	mMusicLoadFlags = 0;
	mDeviceInitialized = false;

	ma_device_config config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_f32;
	config.playback.channels = 2;
	config.sampleRate = 44100; // match BassMusicInterface sample rate
	config.dataCallback = [](ma_device *pDevice, void *pOutput, const void * /*pInput*/, ma_uint32 frameCount) {
		OpenMPTMusicInterface *self = (OpenMPTMusicInterface *)pDevice->pUserData;
		if (self)
		{
			self->dataCallback((float *)pOutput, frameCount);
		}
	};
	config.pUserData = this;

	if (ma_device_init(NULL, &config, &mDevice) != MA_SUCCESS)
	{
		LOG_ERROR("OpenMPTMusicInterface: ma_device_init failed\n");
		mDeviceInitialized = false;
	}
	else
	{
		mDeviceInitialized = true;
		StartDevice();
	}
}

OpenMPTMusicInterface::~OpenMPTMusicInterface()
{
	if (mDeviceInitialized)
	{
		ma_device_stop(&mDevice);
		ma_device_uninit(&mDevice);
	}

	std::lock_guard<std::mutex> lock(mMutex);
	for (auto &kv : mMusicMap)
	{
		OpenMPTMusicInfo *info = kv.second.get();
		if (!info)
			continue;

		if (info->mIsModule && info->mHMusic)
		{
			openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
			openmpt_module_destroy(mod);
		}
		else if (!info->mIsModule)
		{
			std::lock_guard<std::mutex> lk(info->mDecoderMutex);
			ma_decoder_uninit(&info->mDecoder);
		}

		if (info->mStream.mStreamData)
		{
			delete[] static_cast<unsigned char *>(info->mStream.mStreamData);
		}
	}
	mMusicMap.clear();

	std::lock_guard<std::mutex> g(g_globalMutex);
	g_scratchMap.clear();
}

void OpenMPTMusicInterface::StartDevice()
{
	if (mDeviceInitialized)
		ma_device_start(&mDevice);
}

void OpenMPTMusicInterface::StopDevice()
{
	if (mDeviceInitialized)
		ma_device_stop(&mDevice);
}

bool OpenMPTMusicInterface::LoadMusic(int theSongId, const std::string &theFileName)
{
	std::string ext;
	size_t dotPos = theFileName.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		ext = theFileName.substr(dotPos + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	}

	LOG_INFO("OpenMPTMusicInterface: Loading file with extension: %s\n", ext.c_str());

	std::ifstream file(theFileName, std::ios::binary | std::ios::ate);
	if (!file)
		return false;

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	if (size <= 0)
		return false;

	unsigned char *data = new unsigned char[size];
	if (!file.read(reinterpret_cast<char *>(data), size))
	{
		delete[] data;
		return false;
	}

	auto musicInfo = std::make_unique<OpenMPTMusicInfo>();
	musicInfo->mStream.mStreamData = data; // store data pointer for cleanup

	if (ext == "wav" || ext == "ogg" || ext == "mp3" || ext == "flac")
	{
		LOG_INFO("OpenMPTMusicInterface: Using miniaudio decoder\n");

		ma_decoder_config cfg = ma_decoder_config_init_default();
		ma_result result = ma_decoder_init_memory(data, size, &cfg, &musicInfo->mDecoder);

		if (result != MA_SUCCESS)
		{
			LOG_ERROR("OpenMPTMusicInterface: Default decoder init failed: %d, trying with specific format\n", result);

			ma_uint32 sampleRate = mDeviceInitialized ? mDevice.sampleRate : 44100;
			cfg = ma_decoder_config_init(ma_format_f32, 2, sampleRate);
			result = ma_decoder_init_memory(data, size, &cfg, &musicInfo->mDecoder);

			if (result != MA_SUCCESS)
			{
				LOG_ERROR("OpenMPTMusicInterface: Both decoder configs failed: %d for file: %s\n", result,
						theFileName.c_str());
				delete[] data;
				return false;
			}
		}

		LOG_INFO("OpenMPTMusicInterface: Decoder initialized successfully\n");
		musicInfo->mIsModule = false;
	}
	else
	{
		LOG_INFO("OpenMPTMusicInterface: Using OpenMPT\n");

		int err = OPENMPT_ERROR_OK;
		openmpt_module *mod =
			openmpt_module_create_from_memory2(data, size, nullptr, nullptr, nullptr, nullptr, &err, nullptr, nullptr);

		if (!mod || err != OPENMPT_ERROR_OK)
		{
			LOG_ERROR("OpenMPTMusicInterface: OpenMPT failed to load module: error %d\n", err);
			delete[] data;
			return false;
		}

		musicInfo->mHMusic = reinterpret_cast<void *>(mod);
		musicInfo->mIsModule = true;

		std::lock_guard<std::mutex> g(g_globalMutex);
		ScratchBuffers sb;
		sb.left.resize(4096);
		sb.right.resize(4096);
		g_scratchMap[musicInfo.get()] = std::move(sb);
	}

	std::lock_guard<std::mutex> lock(mMutex);
	mMusicMap[theSongId] = std::move(musicInfo);
	LOG_INFO("OpenMPTMusicInterface: Music loaded successfully with ID: %d\n", theSongId);
	return true;
}

void OpenMPTMusicInterface::PlayMusic(int theSongId, int theOffset, bool noLoop)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info)
		return;

	info->mVolume = info->mVolumeCap; // set to full volume immediately
	info->mVolumeAdd = 0.0;
	info->mStopOnFade = noLoop;

	if (info->mIsModule)
	{
		openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
		if (mod)
		{
			openmpt_module_set_repeat_count(mod, noLoop ? 0 : -1);
			if (theOffset > 0)
			{
				double seconds = (double)theOffset / 1000.0;
				openmpt_module_set_position_seconds(mod, seconds);
			}
			else
			{
				openmpt_module_set_position_seconds(mod, 0.0);
			}
		}
	}
	else
	{
		info->mLoop = !noLoop;
		std::lock_guard<std::mutex> lk(info->mDecoderMutex);
		if (theOffset > 0)
		{
			ma_uint32 sampleRate = mDeviceInitialized ? mDevice.sampleRate : 44100;
			double seconds = (double)theOffset / 1000.0;
			ma_uint64 frame = (ma_uint64)(seconds * (double)sampleRate);
			ma_result result = ma_decoder_seek_to_pcm_frame(&info->mDecoder, frame);
			if (result != MA_SUCCESS)
			{
				LOG_ERROR("OpenMPTMusicInterface: Failed to seek to frame %llu\n", frame);
				return;
			}
		} // no seek if theOffset == 0; decoder is already at start
	}
}

void OpenMPTMusicInterface::StopMusic(int theSongId)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info)
		return;

	info->mVolume = 0.0;
	info->mVolumeAdd = 0.0;

	if (info->mIsModule)
	{
		openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
		if (mod)
			openmpt_module_set_position_seconds(mod, 0.0);
	}
	else
	{
		std::lock_guard<std::mutex> lk(info->mDecoderMutex);
		ma_decoder_seek_to_pcm_frame(&info->mDecoder, 0);
	}
}

void OpenMPTMusicInterface::StopAllMusic()
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (auto &kv : mMusicMap)
	{
		OpenMPTMusicInfo *info = kv.second.get();
		if (!info)
			continue;

		info->mVolume = 0.0;
		info->mVolumeAdd = 0.0;

		if (info->mIsModule)
		{
			openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
			if (mod)
				openmpt_module_set_position_seconds(mod, 0.0);
		}
		else
		{
			std::lock_guard<std::mutex> lk(info->mDecoderMutex);
			ma_decoder_seek_to_pcm_frame(&info->mDecoder, 0);
		}
	}
}

void OpenMPTMusicInterface::UnloadMusic(int theSongId)
{
	StopMusic(theSongId);

	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it != mMusicMap.end())
	{
		OpenMPTMusicInfo *info = it->second.get();
		if (info)
		{
			if (info->mIsModule)
			{
				if (info->mHMusic)
				{
					openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
					openmpt_module_destroy(mod);
				}
				std::lock_guard<std::mutex> g(g_globalMutex);
				g_scratchMap.erase(info);
			}
			else
			{
				std::lock_guard<std::mutex> lk(info->mDecoderMutex);
				ma_decoder_uninit(&info->mDecoder);
			}

			if (info->mStream.mStreamData)
			{
				delete[] static_cast<unsigned char *>(info->mStream.mStreamData);
			}
		}
		mMusicMap.erase(it);
	}
}

void OpenMPTMusicInterface::UnloadAllMusic()
{
	StopAllMusic();

	std::lock_guard<std::mutex> lock(mMutex);
	for (auto &kv : mMusicMap)
	{
		OpenMPTMusicInfo *info = kv.second.get();
		if (!info)
			continue;

		if (info->mIsModule)
		{
			if (info->mHMusic)
			{
				openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
				openmpt_module_destroy(mod);
			}
		}
		else
		{
			std::lock_guard<std::mutex> lk(info->mDecoderMutex);
			ma_decoder_uninit(&info->mDecoder);
		}

		if (info->mStream.mStreamData)
		{
			delete[] static_cast<unsigned char *>(info->mStream.mStreamData);
		}
	}
	mMusicMap.clear();

	std::lock_guard<std::mutex> g(g_globalMutex);
	g_scratchMap.clear();
}

void OpenMPTMusicInterface::PauseMusic(int theSongId)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it != mMusicMap.end() && it->second)
	{
		it->second->mVolume = 0.0;
		it->second->mVolumeAdd = 0.0;
	}
}

void OpenMPTMusicInterface::ResumeMusic(int theSongId)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it != mMusicMap.end() && it->second)
	{
		it->second->mVolume = it->second->mVolumeCap;
	}
}

void OpenMPTMusicInterface::PauseAllMusic()
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (auto &kv : mMusicMap)
	{
		if (kv.second)
		{
			kv.second->mVolume = 0.0;
			kv.second->mVolumeAdd = 0.0;
		}
	}
}

void OpenMPTMusicInterface::ResumeAllMusic()
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (auto &kv : mMusicMap)
	{
		if (kv.second)
		{
			kv.second->mVolume = kv.second->mVolumeCap;
		}
	}
}

void OpenMPTMusicInterface::FadeIn(int theSongId, int theOffset, double theSpeed, bool noLoop)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info)
		return;

	info->mVolume = 0.0;					  // start from 0 for fade in
	info->mVolumeAdd = fabs(theSpeed) * 3.14; // positive for fade in
	info->mStopOnFade = noLoop;

	if (info->mIsModule)
	{
		openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
		if (mod)
		{
			openmpt_module_set_repeat_count(mod, noLoop ? 0 : -1);
			if (theOffset > 0)
			{
				double seconds = (double)theOffset / 1000.0;
				openmpt_module_set_position_seconds(mod, seconds);
			}
			else
			{
				openmpt_module_set_position_seconds(mod, 0.0);
			}
		}
	}
	else
	{
		info->mLoop = !noLoop;
		std::lock_guard<std::mutex> lk(info->mDecoderMutex);
		if (theOffset > 0)
		{
			ma_uint32 sampleRate = mDeviceInitialized ? mDevice.sampleRate : 44100;
			double seconds = (double)theOffset / 1000.0;
			ma_uint64 frame = (ma_uint64)(seconds * (double)sampleRate);
			ma_result result = ma_decoder_seek_to_pcm_frame(&info->mDecoder, frame);
			if (result != MA_SUCCESS)
			{
				LOG_ERROR("OpenMPTMusicInterface: Failed to seek to frame %llu\n", frame);
				return;
			}
		} // no seek if theOffset == 0; decoder is already at start
	}
}

void OpenMPTMusicInterface::FadeOut(int theSongId, bool stopSong, double theSpeed)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info)
		return;

	if (info->mVolume > 0.0)
	{
		info->mVolumeAdd = -fabs(theSpeed); // negative for fade out
	}
	info->mStopOnFade = stopSong;
}

void OpenMPTMusicInterface::FadeOutAll(bool stopSong, double theSpeed)
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (auto &kv : mMusicMap)
	{
		OpenMPTMusicInfo *info = kv.second.get();
		if (!info)
			continue;

		if (info->mVolume > 0.0)
		{
			info->mVolumeAdd = -fabs(theSpeed);
		}
		info->mStopOnFade = stopSong;
	}
}

void OpenMPTMusicInterface::SetSongVolume(int theSongId, double theVolume)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info)
		return;

	info->mVolume = std::max(0.0, std::min(theVolume, info->mVolumeCap));
}

void OpenMPTMusicInterface::SetSongMaxVolume(int theSongId, double theMaxVolume)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info)
		return;

	info->mVolumeCap = theMaxVolume;
	if (info->mVolume > info->mVolumeCap)
	{
		info->mVolume = info->mVolumeCap;
	}
}

bool OpenMPTMusicInterface::IsPlaying(int theSongId)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return false;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info)
		return false;

	return info->mVolume > 0.0;
}

void OpenMPTMusicInterface::SetVolume(double theVolume)
{
	g_masterVolume = std::max(0.0, theVolume);
}

void OpenMPTMusicInterface::SetMusicAmplify(int theSongId, double theAmp)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info || !info->mIsModule)
		return;

	openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
	if (!mod)
		return;

	int32_t millibel = 0;
	if (theAmp <= 0.0)
	{
		millibel = -12000; // -120 dB (silence)
	}
	else
	{
		double dB = 20.0 * log10(theAmp);
		millibel = (int32_t)round(dB * 100.0);
	}
	openmpt_module_set_render_param(mod, OPENMPT_MODULE_RENDER_MASTERGAIN_MILLIBEL, millibel);
}

void OpenMPTMusicInterface::Update()
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (auto &kv : mMusicMap)
	{
		OpenMPTMusicInfo *info = kv.second.get();
		if (!info)
			continue;

		if (info->mVolumeAdd != 0.0)
		{
			info->mVolume += info->mVolumeAdd;

			if (info->mVolume >= info->mVolumeCap)
			{
				info->mVolume = info->mVolumeCap;
				info->mVolumeAdd = 0.0;
			}
			else if (info->mVolume <= 0.0)
			{
				info->mVolume = 0.0;
				info->mVolumeAdd = 0.0;

				if (info->mStopOnFade)
				{
					if (info->mIsModule)
					{
						openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
						if (mod)
							openmpt_module_set_position_seconds(mod, 0.0);
					}
					else
					{
						std::lock_guard<std::mutex> lk(info->mDecoderMutex);
						ma_decoder_seek_to_pcm_frame(&info->mDecoder, 0);
					}
				}
			}
		}
	}
}

int OpenMPTMusicInterface::GetMusicOrder(int theSongId)
{
	std::lock_guard<std::mutex> lock(mMutex);
	auto it = mMusicMap.find(theSongId);
	if (it == mMusicMap.end())
		return -1;

	OpenMPTMusicInfo *info = it->second.get();
	if (!info || !info->mIsModule)
		return -1;

	openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
	if (!mod)
		return -1;

	return (int)openmpt_module_get_current_order(mod);
}

void OpenMPTMusicInterface::dataCallback(float *outF32, ma_uint32 frameCount)
{
	const ma_uint32 outSamples = frameCount * 2;
	memset(outF32, 0, outSamples * sizeof(float));

	// get active tracks
	std::vector<OpenMPTMusicInfo *> active;
	{
		std::lock_guard<std::mutex> lock(mMutex);
		for (auto &kv : mMusicMap)
		{
			OpenMPTMusicInfo *info = kv.second.get();
			if (info && info->mVolume > 0.0)
			{
				active.push_back(info);
			}
		}
	}

	if (active.empty())
		return;

	ma_uint32 sampleRate = mDevice.sampleRate;

	for (OpenMPTMusicInfo *info : active)
	{
		double vol = info->mVolume * g_masterVolume;
		float fvol = (float)vol;

		if (info->mIsModule)
		{
			openmpt_module *mod = reinterpret_cast<openmpt_module *>(info->mHMusic);
			if (!mod)
				continue;

			ScratchBuffers *sb = nullptr;
			{
				std::lock_guard<std::mutex> g(g_globalMutex);
				auto it = g_scratchMap.find(info);
				if (it != g_scratchMap.end())
				{
					sb = &it->second;
					sb->ensure_capacity(frameCount);
				}
			}
			if (!sb)
				continue;

			size_t frames =
				openmpt_module_read_float_stereo(mod, sampleRate, frameCount, sb->left.data(), sb->right.data());

			for (size_t i = 0; i < frames; ++i)
			{
				size_t idx = i * 2;
				outF32[idx] += sb->left[i] * fvol;
				outF32[idx + 1] += sb->right[i] * fvol;
			}
		}
		else
		{
			std::vector<float> buffer(frameCount * 2);
			ma_uint64 framesRead = 0;

			{
				std::lock_guard<std::mutex> lk(info->mDecoderMutex);
				ma_result result = ma_decoder_read_pcm_frames(&info->mDecoder, buffer.data(), frameCount, &framesRead);

				if (result != MA_SUCCESS || framesRead == 0)
				{
					if (info->mLoop)
					{
						ma_decoder_seek_to_pcm_frame(&info->mDecoder, 0);
						ma_decoder_read_pcm_frames(&info->mDecoder, buffer.data(), frameCount, &framesRead);
					}
				}
			}

			for (ma_uint64 i = 0; i < framesRead; ++i)
			{
				size_t idx = i * 2;
				outF32[idx] += buffer[idx] * fvol;
				outF32[idx + 1] += buffer[idx + 1] * fvol;
			}
		}
	}

	for (ma_uint32 i = 0; i < outSamples; ++i)
	{
		outF32[i] = std::max(-1.0f, std::min(1.0f, outF32[i]));
	}
}

} // namespace PopLib