#include "pakinterface.hpp"
#include "gpak.hpp"
#include "common.hpp"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <zlib.h>
extern "C"
{
#include <aes.h>
}

using namespace std;

PakInterface *gPakInterface = new PakInterface();

std::string gDecryptPassword = "PopCapPopLibFramework";

//////////////////
std::vector<uint8_t> Decompress(const std::vector<uint8_t> &input, size_t originalSize)
{
	std::vector<uint8_t> output(originalSize);
	uLongf destLen = originalSize;

	int res = ::uncompress(output.data(), &destLen, input.data(), input.size());
	if (res != Z_OK)
		throw std::runtime_error("zlib decompression failed with code: " + std::to_string(res));

	return output;
}

//////////////////
std::vector<uint8_t> AESDecrypt(const std::vector<uint8_t> &data, const std::string &password)
{
	AES_ctx ctx;
	uint8_t key[32] = {};
	std::memcpy(key, password.data(), std::min(password.size(), sizeof(key)));
	AES_init_ctx(&ctx, key);

	std::vector<uint8_t> decrypted = data;
	for (size_t i = 0; i < decrypted.size(); i += 16)
	{
		AES_ECB_decrypt(&ctx, decrypted.data() + i);
	}

	if (!decrypted.empty())
	{
		uint8_t pad = decrypted.back();
		if (pad <= 16)
			decrypted.resize(decrypted.size() - pad);
	}
	return decrypted;
}

//////////////////
static bool starts_with(const std::string &str, const std::string &prefix)
{
	return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

static inline std::string toupper(const std::string &theString)
{
	std::string aString;
	for (unsigned i = 0; i < theString.length(); i++)
		aString += toupper(theString[i]);
	return aString;
}
//////////////////

// Helper to convert wildcard patterns to simple matching
static bool matchPattern(const string &pattern, const string &name)
{
	size_t pos = pattern.find('*');
	if (pos == string::npos)
		return _stricmp(pattern.c_str(), name.c_str()) == 0;
	string prefix = pattern.substr(0, pos);
	string suffix = pattern.substr(pos + 1);
	if (!starts_with(name, prefix))
		return false;
	if (suffix.empty())
		return true;
	if (name.length() < prefix.length() + suffix.length())
		return false;
	return _stricmp(name.substr(name.size() - suffix.size()).c_str(), suffix.c_str()) == 0;
}

PakInterface::PakInterface()
{
	// nothing to initialize beyond base
}

PakInterface::~PakInterface()
{
}

bool PakInterface::AddPakFile(const string &fileName)
{
	FILE *fp = fopen(fileName.c_str(), "rb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	size_t fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	mPakCollectionList.emplace_back(fileSize);
	PakCollection &collection = mPakCollectionList.back();
	fread(collection.data(), 1, fileSize, fp);
	fclose(fp);

	// check for gpak header
	GPAKHeader *gpakHeader = reinterpret_cast<GPAKHeader *>(collection.data());
	if (memcmp(gpakHeader->magic, "GPAK", 4) != 0 || gpakHeader->version != 1)
		return false;

	GPAKFileEntry *entriesPtr =
		reinterpret_cast<GPAKFileEntry *>(reinterpret_cast<uint8_t *>(collection.data()) + gpakHeader->fileTableOffset);
	std::vector<GPAKFileEntry> entries(entriesPtr, entriesPtr + gpakHeader->fileCount);

	// the decompressed buffer to fill up
	std::vector<uint8_t> finalBuffer;
	finalBuffer.reserve(fileSize);

	// Size of the header for recalculating the start pos
	size_t headerSize = gpakHeader->fileTableOffset + gpakHeader->fileCount * sizeof(GPAKFileEntry);

	for (const GPAKFileEntry &entry : entries)
	{
		std::string upperName = toupper(std::string(entry.path));
		PakRecord &rec = mPakRecordMap[upperName];
		rec.mCollection = &collection;
		rec.mFileName = entry.path;
		rec.mSize = entry.originalSize;
		rec.mFileTime = filesystem::file_time_type::min(); // GPAK doesn't store this yet

		// Decompress and Decrypt the GPAK data.
		const uint8_t *compressedData = collection.data() + entry.dataOffset;
		std::vector<uint8_t> compressed(compressedData, compressedData + entry.compressedSize);
		if (!gDecryptPassword.empty())
			compressed = AESDecrypt(compressed, gDecryptPassword);
		std::vector<uint8_t> decompressed = Decompress(compressed, entry.originalSize);

		rec.mStartPos = finalBuffer.size();

		finalBuffer.insert(finalBuffer.end(), decompressed.begin(), decompressed.end());
	}

	// move the readable data into the collection for fread to use
	collection.vector() = std::move(finalBuffer);

	return true;
}

PFILE *PakInterface::FOpen(const char *fn, const char *mode)
{
	string name(fn);
	auto it = mPakRecordMap.find(toupper(name));
	if (it != mPakRecordMap.end())
	{
		PFILE *pf = new PFILE;
		pf->mRecord = &it->second;
		pf->mPos = 0;
		pf->mFP = nullptr;
		return pf;
	}
	FILE *real = fopen(fn, mode);
	if (!real)
		return nullptr;
	PFILE *pf = new PFILE;
	pf->mRecord = nullptr;
	pf->mFP = real;
	pf->mPos = 0;
	return pf;
}

int PakInterface::FClose(PFILE *pf)
{
	if (pf->mFP)
		fclose(pf->mFP);
	delete pf;
	return 0;
}

int PakInterface::FSeek(PFILE *pf, long offset, int whence)
{
	if (pf->mRecord)
	{
		auto &r = *pf->mRecord;
		switch (whence)
		{
		case SEEK_SET:
			pf->mPos = offset;
			break;
		case SEEK_CUR:
			pf->mPos += offset;
			break;
		case SEEK_END:
			pf->mPos = r.mSize + offset;
			break;
		}
		pf->mPos = std::min(std::max(pf->mPos, 0L), static_cast<long>(r.mSize));
		return 0;
	}
	return fseek(pf->mFP, offset, whence);
}

int PakInterface::FTell(PFILE *pf)
{
	return pf->mRecord ? static_cast<int>(pf->mPos) : ftell(pf->mFP);
}

size_t PakInterface::FRead(void *buf, int size, int count, PFILE *pf)
{
	if (pf->mRecord)
	{
		PakRecord *rec = pf->mRecord;

		int aSizeBytes = std::min(size * count, static_cast<int>(pf->mRecord->mSize - pf->mPos));

		std::memcpy(buf, rec->mCollection->data() + rec->mStartPos + pf->mPos, aSizeBytes);

		pf->mPos += aSizeBytes;

		return aSizeBytes / size;
	}
	return fread(buf, size, count, pf->mFP);
}

int PakInterface::FGetC(PFILE *pf)
{
	unsigned char c;
	return FRead(&c, 1, 1, pf) == 1 ? c : EOF;
}

int PakInterface::UnGetC(int c, PFILE *pf)
{
	if (pf->mRecord)
	{
		pf->mPos = std::max(0L, pf->mPos - 1);
		return c;
	}
	return ungetc(c, pf->mFP);
}

char *PakInterface::FGetS(char *str, int size, PFILE *pf)
{
	int i = 0;
	while (i + 1 < size)
	{
		int c = FGetC(pf);
		if (c == EOF || c == '\n')
			break;
		str[i++] = c;
	}
	str[i] = '\0';
	return i > 0 ? str : nullptr;
}

int PakInterface::FEof(PFILE *pf)
{
	return pf->mRecord ? pf->mPos >= pf->mRecord->mSize : feof(pf->mFP);
}

#ifdef _WIN32
#undef FindFirstFile
#undef FindNextFile
#endif

PFindData PakInterface::FindFirstFile(const string &pattern)
{
	PFindData fd;
	fd.it = filesystem::directory_iterator(".");
	fd.end = filesystem::directory_iterator();
	fd.pattern = pattern;
	return fd;
}

bool PakInterface::FindNextFile(PFindData &fd, string &outName)
{
	while (fd.it != fd.end)
	{
		auto entry = *fd.it++;
		string name = entry.path().filename().string();
		if (matchPattern(fd.pattern, name))
		{
			outName = name;
			return true;
		}
	}
	return false;
}

void PakInterface::FindClose(PFindData &fd)
{
	// nothing to clean
}
