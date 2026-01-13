#ifndef __AUREADER_HPP__
#define __AUREADER_HPP__

#pragma once

#include <vector>

struct AuFile
{
	uint32_t mSampleRate;
	uint32_t mChannels;
	std::vector<int16_t> mSamples;
};

// Big-endian 32-bit reader
inline uint32_t read_be32(const uint8_t *p)
{
	return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | (uint32_t(p[3]));
}

// u-law decoder (G.711 u-law, 8-bit -> 16-bit PCM)
inline int16_t ulawDecode(uint8_t u)
{
	u = ~u;
	int t = ((u & 0x0F) << 3) + 0x84;
	t <<= (u & 0x70) >> 4;
	return (u & 0x80) ? (0x84 - t) : (t - 0x84);
}

inline bool LoadAU(const uint8_t *data, size_t size, AuFile &out)
{
	if (size < 28)
		return false; // must have header+annotation
	uint32_t magic = read_be32(data + 0);
	uint32_t offset = read_be32(data + 4);
	uint32_t dataSize = read_be32(data + 8);
	uint32_t encoding = read_be32(data + 12);
	uint32_t sampleRate = read_be32(data + 16);
	uint32_t channels = read_be32(data + 20);

	if (magic != 0x2e736e64u) // ".snd"
		return false;
	if (offset + dataSize > size) // bound check
		return false;

	const uint8_t *audio = data + offset;
	out.mSampleRate = sampleRate;
	out.mChannels = channels;
	out.mSamples.clear();
	out.mSamples.reserve(dataSize);

	// Decode based on encoding
	if (encoding == 1)
	{ // u-law
		for (uint32_t i = 0; i < dataSize; ++i)
			out.mSamples.push_back(ulawDecode(audio[i]));
	}
	else if (encoding >= 2 && encoding <= 7)
	{ // linear PCM 8/16/24/32/float/double
		size_t depth = (encoding == 2 ? 1 : encoding == 3 ? 2 : encoding == 4 ? 3 : encoding == 5 ? 4 : 0);
		if (depth == 0)
			return false;
		for (uint32_t i = 0; i + depth <= dataSize; i += depth)
		{
			int32_t v = 0;
			// big-endian to signed
			for (size_t b = 0; b < depth; ++b)
				v = (v << 8) | audio[i + b];
			// sign-extend
			int32_t sign = 1 << (depth * 8 - 1);
			if (v & sign)
				v |= ~((1 << (depth * 8)) - 1);
			out.mSamples.push_back(int16_t(v >> (depth * 8 - 16)));
		}
	}
	else
	{
		return false; // unsupported encoding
	}
	return true;
}

#endif