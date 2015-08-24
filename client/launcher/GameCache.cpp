/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <array>

#ifdef GTA_FIVE
// entry for a cached-intent file
struct GameCacheEntry
{
	// local filename to map from
	const wchar_t* filename;

	// checksum (SHA1, typically) to validate as
	const char* checksum;

	// remote path on ROS service to use
	const char* remotePath;

	// file to extract from any potential archive
	const char* archivedFile;

	// local size of the file
	size_t localSize;

	// remote size of the archive file
	size_t remoteSize;

	// constructor
	GameCacheEntry(const wchar_t* filename, const char* checksum, const char* remotePath, size_t localSize)
		: filename(filename), checksum(checksum), remotePath(remotePath), localSize(localSize), remoteSize(localSize), archivedFile(nullptr)
	{

	}

	GameCacheEntry(const wchar_t* filename, const char* checksum, const char* remotePath, const char* archivedFile, size_t localSize, size_t remoteSize)
		: filename(filename), checksum(checksum), remotePath(remotePath), localSize(localSize), remoteSize(remoteSize), archivedFile(archivedFile)
	{

	}

	// methods
	std::wstring GetCacheFileName() const
	{
		std::wstring filenameBase = filename;
		std::replace(filenameBase.begin(), filenameBase.end(), '/', '+');

		wchar_t checksumWide[42];
		mbstowcs(checksumWide, checksum, _countof(checksumWide));

		return MakeRelativeCitPath(va(L"cache\\game\\%s_%s", filenameBase.c_str(), checksumWide));
	}

	std::wstring GetRemoteBaseName() const
	{
		std::string remoteNameBase = remotePath;
		std::wstring remoteNameBaseWide(remoteNameBase.begin(), remoteNameBase.end());

		int slashIndex = remoteNameBaseWide.find_last_of(L'/') + 1;

		return MakeRelativeCitPath(va(L"cache\\game\\%s", remoteNameBaseWide.substr(slashIndex).c_str()));
	}

	std::wstring GetLocalFileName() const
	{
		return MakeRelativeGamePath(filename);
	}
};

struct GameCacheStorageEntry
{
	// sha1-sized file checksum
	uint8_t checksum[20];

	// file modification time
	time_t fileTime;
};

// global cache mapping of ROS files to disk files
static GameCacheEntry g_requiredEntries[] =
{
	// TODO: replace paths with NSIS variable-based paths
	{ L"GTA5.exe", "883d05ce147ec01f94f862453bb69fe96cc15539", "Game_EFIGS/GTA_V_Patch_1_0_393_4.exe", "$/GTA5.exe", 55839112, 422755424 },
	{ L"update/update.rpf", "d9f84cd5b8b5bafaeee92dd43568887172849d01", "Game_EFIGS/GTA_V_Patch_1_0_393_4.exe", "$/update/update.rpf", 374835200, 422755424 },

	{ L"update/x64/dlcpacks/patchday4ng/dlc.rpf", "124c908d82724258a5721535c87f1b8e5c6d8e57", "DigitalData/DLCPacks2/update/x64/dlcpacks/patchday4ng/dlc.rpf", 312438784 },
	{ L"update/x64/dlcpacks/mpluxe/dlc.rpf", "78f7777b49f4b4d77e3da6db728cb3f7ec51e2fc", "DigitalData/DLCPacks2/update/x64/dlcpacks/mpluxe/dlc.rpf", 226260992 },

	{ L"update/x64/dlcpacks/patchday5ng/dlc.rpf", "af3b2a59b4e1e5fd220c308d85753bdbffd8063c", "DigitalData/DLCPacks3/update/x64/dlcpacks/patchday5ng/dlc.rpf", 7827456 },
	{ L"update/x64/dlcpacks/mpluxe2/dlc.rpf", "1e59e1f05be5dba5650a1166eadfcb5aeaf7737b", "DigitalData/DLCPacks3/update/x64/dlcpacks/mpluxe2/dlc.rpf", 105105408 },
};

static bool ParseCacheFileName(const char* inString, std::string& fileNameOut, std::string& hashOut)
{
	// check if the file name meets the minimum length for there to be a hash
	int length = strlen(inString);

	if (length < 44)
	{
		return false;
	}

	// find the file extension
	const char* dotPos = strchr(inString, '.');

	if (!dotPos)
	{
		return false;
	}

	// find the first underscore following the file extension
	const char* underscorePos = strchr(inString, '_');

	if (!underscorePos)
	{
		return false;
	}

	// store the file name
	fileNameOut = fwString(inString, underscorePos - inString);

	// check if we have a hash
	const char* hashStart = &inString[length - 41];

	if (*hashStart != '_')
	{
		return false;
	}

	hashOut = hashStart + 1;

	return true;
}

template<int Size>
static std::array<uint8_t, Size> ParseHexString(const char* string)
{
	std::array<uint8_t, Size> retval;

	assert(strlen(string) == Size * 2);

	for (int i = 0; i < Size; i++)
	{
		const char* startIndex = &string[i * 2];
		char byte[3] = { startIndex[0], startIndex[1], 0 };

		retval[i] = strtoul(byte, nullptr, 16);
	}

	return retval;
}

static std::vector<GameCacheStorageEntry> LoadCacheStorage()
{
	// create the cache directory if needed
	CreateDirectory(MakeRelativeCitPath(L"cache").c_str(), nullptr);
	CreateDirectory(MakeRelativeCitPath(L"cache\\game").c_str(), nullptr);

	// output buffer
	std::vector<GameCacheStorageEntry> cacheStorage;

	// iterate over files in cache
	WIN32_FIND_DATA findData;

	HANDLE hFind = FindFirstFile(MakeRelativeCitPath(L"cache\\game\\*.*").c_str(), &findData);

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do 
		{
			// try parsing the file name
			std::string fileName;
			std::string fileHash;

			if (ParseCacheFileName(converter.to_bytes(findData.cFileName).c_str(), fileName, fileHash))
			{
				// add the entry, if so
				LARGE_INTEGER quadTime;
				quadTime.HighPart = findData.ftLastWriteTime.dwHighDateTime;
				quadTime.LowPart = findData.ftLastWriteTime.dwLowDateTime;

				GameCacheStorageEntry entry;
				entry.fileTime = quadTime.QuadPart / 10000000ULL - 11644473600ULL;
				
				auto checksum = ParseHexString<20>(fileHash.c_str());
				memcpy(entry.checksum, checksum.data(), checksum.size());

				cacheStorage.push_back(entry);
			}
		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}

	// return the obtained data
	return cacheStorage;
}

static std::vector<GameCacheEntry> CompareCacheDifferences()
{
	// load the cache storage from disk
	auto storageEntries = LoadCacheStorage();

	// return value
	std::vector<GameCacheEntry> retval;

	// go through each entry and check for validity
	for (auto& entry : g_requiredEntries)
	{
		// find the storage entry associated with the file and check it for validity
		auto requiredHash = ParseHexString<20>(entry.checksum);
		bool found = false;

		for (auto& storageEntry : storageEntries)
		{
			if (std::equal(requiredHash.begin(), requiredHash.end(), storageEntry.checksum))
			{
				// check if the file exists
				std::wstring cacheFileName = entry.GetCacheFileName();

				if (GetFileAttributes(cacheFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
				{
					// as it doesn't add to the list
					retval.push_back(entry);
				}

				found = true;

				break;
			}
		}

		// if no entry was found, add to the list as well
		if (!found)
		{
			retval.push_back(entry);
		}
	}

	return retval;
}

bool ExtractInstallerFile(const std::wstring& installerFile, const std::string& entryName, const std::wstring& outFile);

static void PerformUpdate(const std::vector<GameCacheEntry>& entries)
{
	// create UI
	UI_DoCreation();

	// hash local files for those that *do* exist, add those that don't match to the download queue and add those that do match to be copied locally
	std::set<std::string> referencedFiles; // remote URLs that we already requested
	std::vector<GameCacheEntry> extractedEntries; // entries to extract from an archive

	for (auto& entry : entries)
	{
		// check if the file is outdated
		auto hash = ParseHexString<20>(entry.checksum);

		bool fileOutdated = CheckFileOutdatedWithUI(entry.GetLocalFileName().c_str(), hash.data());

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

		// if not, copy it from the local filesystem (we're abusing the download code here a lot)
		if (!fileOutdated)
		{
			CL_QueueDownload(va("file:///%s", converter.to_bytes(entry.GetLocalFileName()).c_str()), converter.to_bytes(entry.GetCacheFileName()).c_str(), entry.localSize, false);
		}
		else
		{
			// else, if it's not already referenced by a queued download...
			if (referencedFiles.find(entry.remotePath) == referencedFiles.end())
			{
				// download it from the rockstar service
				std::string localFileName = (entry.archivedFile) ? converter.to_bytes(entry.GetRemoteBaseName()) : converter.to_bytes(entry.GetCacheFileName());

				CL_QueueDownload(va("rockstar:%s", entry.remotePath), localFileName.c_str(), entry.remoteSize, false);

				referencedFiles.insert(entry.remotePath);
			}

			// if we want an archived file from here, we should *likely* note its existence
			extractedEntries.push_back(entry);
		}
	}

	bool retval = DL_RunLoop();

	// if succeeded, try extracting any entries
	if (retval)
	{
		for (auto& entry : extractedEntries)
		{
			retval = retval && ExtractInstallerFile(entry.GetRemoteBaseName(), entry.archivedFile, entry.GetCacheFileName());
		}
	}

	// destroy UI
	UI_DoDestruction();

	// failed?
	if (!retval)
	{
		FatalError("are you still the one i knoooooooooow?");
	}
}

std::map<std::string, std::string> UpdateGameCache()
{
	// perform a game update
	auto differences = CompareCacheDifferences();

	if (!differences.empty())
	{
		PerformUpdate(differences);
	}

	// get a list of cache files that should be mapped given an updated cache
	std::map<std::string, std::string> retval;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	for (auto& entry : g_requiredEntries)
	{
		retval.insert({ converter.to_bytes(entry.filename), converter.to_bytes(entry.GetCacheFileName()) });
	}

	return retval;
}
#endif