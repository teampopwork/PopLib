#ifndef __GPAK_HPP__
#define __GPAK_HPP__

#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief GPAK header
 */
struct GPAKHeader
{
	char magic[5];			  // should be {'G','P','A','K','\0'}
	uint32_t version;		  // format version (start at 1)
	uint32_t fileCount;		  // how many entries in the file table
	uint64_t fileTableOffset; // byte offset in this .pak where the table begins
};

/**
 * @brief GPAK file entry
 */
struct GPAKFileEntry
{
	char path[256];			 // null-terminated relative path
	uint64_t dataOffset;	 // byte offset in .pak where compressed data lives
	uint32_t compressedSize; // size in bytes of the compressed blob
	uint32_t originalSize;	 // uncompressed size in bytes
};

#endif // __GPAK_HPP__