/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#if defined(LAUNCHER_PERSONALITY_MAIN) || defined(COMPILING_GLUE)
#include <CfxLocale.h>
#include <tinyxml2.h>

#include <stdint.h>

#include <optional>
#include <list>
#include <unordered_set>
#include <string>

#include <sstream>

#include <openssl/sha.h>
#include <boost/algorithm/string.hpp>

#include "launcher_version.h"

struct cache_t
{
	std::string name;
	int version = 0;
	std::string manifest;
	std::string manifestUrl;
};

std::string GetObjectURL(std::string_view objectHash, std::string_view suffix = "")
{
	auto url = fmt::sprintf("%s/%s/%s/%s%s", CONTENT_URL, objectHash.substr(0, 2), objectHash.substr(2, 2), objectHash, suffix);
	boost::algorithm::to_lower(url);

	return url;
}

using cache_ptr = std::shared_ptr<cache_t>;

struct cacheFile_t
{
private:
	std::list<cache_ptr> caches;

public:
	void Parse(const char* str)
	{
		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError error = doc.Parse(str);

		if (error == tinyxml2::XML_SUCCESS)
		{
			auto rootElement = doc.RootElement();
			auto cacheElement = rootElement->FirstChildElement("Cache");

			while (cacheElement)
			{
				cache_t cache;
				cache.name = cacheElement->Attribute("ID");
				cache.version = atoi(cacheElement->Attribute("Version"));
				
				if (auto child = cacheElement->FirstChild(); child)
				{
					if (auto text = child->ToText(); text)
					{
						cache.manifest = text->Value();
					}
				}

				caches.push_back(std::make_shared<cache_t>(cache));

				cacheElement = cacheElement->NextSiblingElement("Cache");
			}
		}
	}

	std::list<cache_ptr>& GetCaches()
	{
		return caches;
	}

	cache_ptr GetCache(std::string name)
	{
		for (cache_ptr& cache : caches)
		{
			if (cache->name == name)
			{
				return cache;
			}
		}

		return std::make_shared<cache_t>();
	}
};

struct manifestFile_t
{
	using TTuple = std::tuple<std::string, int64_t, std::array<uint8_t, 20>>;

	std::string name;
	int64_t downloadSize = 0;
	int64_t localSize = 0;
	bool compressed = false;
	bool hash256Valid = false;
	compressionAlgo_e algorithm = compressionAlgo_e::XZ;
	std::array<uint8_t, 20> hash = { 0 };
	std::array<uint8_t, 32> hash256 = { 0 };

	TTuple ToTuple() const
	{
		return { name, localSize, hash };
	}

	std::string_view AlgoSuffix() const
	{
		switch (algorithm)
		{
			case compressionAlgo_e::XZ:
				return ".xz";
			case compressionAlgo_e::Zstd:
				return ".zst";
			case compressionAlgo_e::None:
			default:
				return "";
		}
	}
};

struct manifest_t
{
	using TSet = std::set<typename manifestFile_t::TTuple>;

private:
	std::list<manifestFile_t> files;
	cache_ptr parentCache;

public:
	manifest_t(cache_ptr parent)
		: parentCache(parent)
	{

	}

	bool Parse(const char* str)
	{
		tinyxml2::XMLDocument doc;
		if (doc.Parse(str) != tinyxml2::XML_SUCCESS)
		{
			return false;
		}

		auto rootElement = doc.RootElement();
		auto fileElement = rootElement->FirstChildElement("ContentFile");

		while (fileElement)
		{
			manifestFile_t file;
			file.name = fileElement->Attribute("Name");

			int64_t size, compressedSize, compressedSizeZstd;
			size = strtoll(fileElement->Attribute("Size"), nullptr, 10);
			compressedSize = strtoll(fileElement->Attribute("CompressedSize"), nullptr, 10);
			compressedSizeZstd = 0;

			if (auto zstdSize = fileElement->Attribute("CompressedSizeZstd"))
			{
				compressedSizeZstd = strtoll(zstdSize, nullptr, 10);
				file.algorithm = compressionAlgo_e::Zstd;
			}

			file.compressed = (size != compressedSize);
			file.downloadSize = compressedSize;
			file.localSize = size;

			if (!file.compressed)
			{
				file.algorithm = compressionAlgo_e::None;
			}

			ParseHash(fileElement->Attribute("SHA1Hash"), file.hash.data());

			if (fileElement->Attribute("SHA256Hash"))
			{
				ParseHash(fileElement->Attribute("SHA256Hash"), file.hash256.data());
				file.hash256Valid = true;
			}

			files.push_back(file);

			fileElement = fileElement->NextSiblingElement("ContentFile");
		}

		return true;
	}

	const manifestFile_t* GetFile(std::string_view name) const
	{
		for (const auto& file : files)
		{
			if (file.name == name)
			{
				return &file;
			}
		}

		return nullptr;
	}

	std::list<manifestFile_t>& GetFiles()
	{
		return files;
	}

	cache_ptr GetParentCache()
	{
		return parentCache;
	}

	TSet ToSet() const
	{
		TSet rv;

		for (const auto& file : files)
		{
			rv.insert(file.ToTuple());
		}

		return rv;
	}

private:
	void ParseHash(const char* hashString, uint8_t* out)
	{
		int i = 0;

		while (*hashString)
		{
			char hexDigit[3];
			hexDigit[0] = *hashString;
			hexDigit[1] = *(hashString + 1);
			hexDigit[2] = '\0';

			out[i] = (uint8_t)strtoul(hexDigit, nullptr, 16);

			hashString += 2;
			i++;
		}
	}
};

bool Updater_RunUpdate(std::initializer_list<std::string> wantedCachesList)
{
	std::unordered_set<std::string> wantedCaches{
		wantedCachesList
	};

	// fetch remote caches
	cacheFile_t cacheFile;

	static std::vector<char> cachesFile(131072);

	bool success = false;
	for (auto& cacheName : wantedCaches)
	{
		char bootstrapVersion[256];

		auto contentHeaders = std::make_shared<HttpHeaderList>();
		auto testURL = va(CONTENT_URL "/heads/%s/%s?time=%lld", cacheName, GetUpdateChannel(), _time64(NULL));
		int result = DL_RequestURL(va(CONTENT_URL "/heads/%s/%s?time=%lld", cacheName, GetUpdateChannel(), _time64(NULL)), bootstrapVersion, sizeof(bootstrapVersion), contentHeaders);

		if (result != 0 && !success)
		{
			UI_DisplayError(va(L"An error (%i, %s) occurred while checking the game version. Check if " CONTENT_URL_WIDE L" is available in your web browser.", result, ToWide(DL_RequestURLError())));
			return false;
		}

		success = true;

		auto v = contentHeaders->find("x-amz-meta-branch-version");
		auto m = contentHeaders->find("x-amz-meta-branch-manifest");

		if (v == contentHeaders->end() || m == contentHeaders->end())
		{
			continue;
		}

		// get the caches we want to update
		cache_ptr cache = std::make_shared<cache_t>();
		cache->name = cacheName;
		cache->version = std::stoi(v->second);
		cache->manifestUrl = GetObjectURL(m->second);

		cacheFile.GetCaches().push_back(cache);
	}

	// error out if the remote caches file is empty
	if (cacheFile.GetCaches().empty())
	{
		MessageBox(NULL, L"Remote caches file could not be parsed. Check if " CONTENT_URL_WIDE L" is available in your web browser.", L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
		return false;
	}

	// ---------------------
	// read local cache file
	// ---------------------
	cacheFile_t localCacheFile;
	std::list<std::tuple<std::optional<cache_ptr>, cache_ptr>> needsUpdate;

	// workaround: if the user removed citizen/ (or its contents), make sure we re-verify, as that's just silly
	bool shouldVerify = false;

	if (GetFileAttributesW(MakeRelativeCitPath(L"citizen/").c_str()) == INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesW(MakeRelativeCitPath(L"citizen/version.txt").c_str()) == INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesW(MakeRelativeCitPath(L"citizen/release.txt").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		shouldVerify = true;
	}

	// if citizen/ got overwritten with a 'weird' incompatible/way too old version, also verify
#if defined(EXE_VERSION) && EXE_VERSION > 1
	{
		FILE* f = _wfopen(MakeRelativeCitPath(L"citizen/release.txt").c_str(), L"r");

		if (f)
		{
			char ver[128];

			fgets(ver, sizeof(ver), f);
			fclose(f);

			int version = atoi(ver);
			if (version < EXE_VERSION)
			{
				shouldVerify = true;
			}
		}
	}
#endif

	// additional check for Five/RDR
#if defined(GTA_FIVE) || defined(IS_RDR3)
	if (GetFileAttributesW(MakeRelativeCitPath(L"citizen/ros/ros.crt").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		shouldVerify = true;
	}
#endif

	FILE* cachesReader = NULL;
	
	if (!shouldVerify)
	{
		cachesReader = _wfopen(MakeRelativeCitPath(L"content_index.xml").c_str(), L"rb");

		if (!cachesReader)
		{
			// old?
			cachesReader = _wfopen(MakeRelativeCitPath(L"caches.xml").c_str(), L"rb");
		}
	}

	// ------------------------------------
	// if local cache file does *not* exist
	// ------------------------------------
	if (!cachesReader)
	{
		for (const cache_ptr& cache : cacheFile.GetCaches())
		{
			if (wantedCaches.find(cache->name) != wantedCaches.end())
			{
				needsUpdate.emplace_back(std::optional<cache_ptr>{}, cache);
			}
		}
	}
	else
	{
		// --------------------------------
		// if local cache file *does* exist
		// --------------------------------
		fseek(cachesReader, 0, SEEK_END);
		size_t len = ftell(cachesReader);
		fseek(cachesReader, 0, SEEK_SET);

		if (cachesFile.size() < (len + 1))
		{
			cachesFile.resize(len + 1);
		}

		int length = fread(cachesFile.data(), 1, cachesFile.size(), cachesReader);
		fclose(cachesReader);

		cachesFile[length] = '\0';

		localCacheFile.Parse(cachesFile.data());

		for (cache_ptr& cache : cacheFile.GetCaches())
		{
			if (wantedCaches.find(cache->name) != wantedCaches.end())
			{
				cache_ptr& localCache = localCacheFile.GetCache(cache->name);

				if (localCache->version != cache->version)
				{
					needsUpdate.emplace_back(localCache, cache);
				}
				else
				{
					wantedCaches.erase(cache->name);
				}
			}
		}
	}

	// if we don't need to update, return true
	if (needsUpdate.size() == 0)
	{
		return true;
	}

	// -------------------------------------------
	// fetch cache manifests and enqueue downloads
	// -------------------------------------------
	std::list<std::pair<cache_ptr, manifestFile_t>> queuedFiles;
	std::list<std::tuple<std::string, size_t>> deleteFiles;
	std::set<std::string> queuedNames;

	for (auto& [localCache, cache] : needsUpdate)
	{
		int result = DL_RequestURL(cache->manifestUrl.c_str(), cachesFile.data(), cachesFile.size());

		manifest_t manifest(cache);
		manifest.Parse(cachesFile.data());
		cache->manifest = cachesFile.data();

		// check if we have a valid manifest
		bool localDiff = false;
		manifest_t localManifest(cache);

		if (localCache && !(*localCache)->manifest.empty())
		{
			localDiff = localManifest.Parse((*localCache)->manifest.c_str());
		}

		// if we *don't* want to diff
		if (!localDiff)
		{
			for (manifestFile_t& file : manifest.GetFiles())
			{
				queuedFiles.emplace_back(cache, file);
			}
		}
		else
		{
			// or if we do...
			auto localData = localManifest.ToSet();
			auto remoteData = manifest.ToSet();
			manifest_t::TSet diffData;

			std::set_symmetric_difference(remoteData.begin(), remoteData.end(), localData.begin(), localData.end(), std::inserter(diffData, diffData.begin()));

			for (const auto& [name, size, _] : diffData)
			{
				auto file = manifest.GetFile(name);

				if (!file)
				{
					deleteFiles.emplace_back(name, size);
				}
				else if (queuedNames.find(name) == queuedNames.end())
				{
					queuedFiles.emplace_back(cache, *file);
					queuedNames.insert(name);
				}
			}
		}
	}

	UI_DoCreation();
	CL_InitDownloadQueue();

	uint64_t fileStart = 0;
	uint64_t fileTotal = 0;

	for (auto& filePair : queuedFiles)
	{
		struct _stat64 stat;
		if (_wstat64(MakeRelativeCitPath(ToWide(filePair.second.name)).c_str(), &stat) >= 0)
		{
			// if the size is wrong.. why verify? -> we don't count this file so don't add it to verifying
			if (stat.st_size == filePair.second.localSize)
			{
				fileTotal += stat.st_size;
			}
		}
	}

	UI_UpdateText(0, gettext(L"Verifying content...").c_str());

	for (auto& filePair : queuedFiles)
	{
		cache_ptr cache = filePair.first;
		manifestFile_t& file = filePair.second;

		// check file hash first
		std::array<uint8_t, 20> hashEntry;
		memcpy(hashEntry.data(), file.hash.data(), hashEntry.size());

		std::stringstream formattedHash;
		for (uint8_t b : hashEntry)
		{
			formattedHash << fmt::sprintf("%02X", (uint32_t)b);
		}

		bool fileOutdated = CheckFileOutdatedWithUI(MakeRelativeCitPath(ToWide(file.name)).c_str(), { hashEntry }, &fileStart, fileTotal, nullptr, filePair.second.localSize);

		if (fileOutdated)
		{
			std::stringstream hashString;
			for (uint8_t b : file.hash256)
			{
				hashString << fmt::sprintf("%02x", b);
			}

			std::string url = GetObjectURL(hashString.str(), file.AlgoSuffix());
			std::string outPath = ToNarrow(MakeRelativeCitPath(ToWide(file.name)));

			CL_QueueDownload(url.c_str(), outPath.c_str(), file.downloadSize, file.algorithm);
		}
	}

	UI_UpdateText(0, va(gettext(L"Updating %s..."), PRODUCT_NAME));

	bool retval = DL_RunLoop();

	// delete obsolete files
	// *after* downloading, so if downloading fails we won't break
	for (const auto& [name, size] : deleteFiles)
	{
		struct _stat64 stat;
		auto fn = MakeRelativeCitPath(ToWide(name));
		if (_wstat64(fn.c_str(), &stat) >= 0)
		{
			if (stat.st_size == size)
			{
				// delete the file
				DeleteFileW(fn.c_str());
			}
		}
	}

	UI_DoDestruction();

	if (retval)
	{
		tinyxml2::XMLDocument cachesDoc;
		auto root = cachesDoc.NewElement("Caches");
		
		for (const cache_ptr& cache : cacheFile.GetCaches())
		{
			auto cacheElement = cachesDoc.NewElement("Cache");
			cacheElement->SetAttribute("ID", cache->name.c_str());
			tinyxml2::XMLText* text = nullptr;
			
			if (wantedCaches.find(cache->name) != wantedCaches.end())
			{
				cacheElement->SetAttribute("Version", cache->version);

				if (!cache->manifest.empty())
				{
					text = cachesDoc.NewText(cache->manifest.c_str());
				}
			}
			else
			{
				cache_ptr localCache = localCacheFile.GetCache(cache->name);
				cacheElement->SetAttribute("Version", localCache->version);

				if (!localCache->manifest.empty())
				{
					text = cachesDoc.NewText(localCache->manifest.c_str());
				}
			}

			if (text)
			{
				text->SetCData(true);

				cacheElement->InsertFirstChild(text);
			}

			root->InsertEndChild(cacheElement);
		}

		cachesDoc.InsertEndChild(root);
		
		_wunlink(MakeRelativeCitPath(L"caches.xml").c_str());
		FILE* outCachesFile = _wfopen(MakeRelativeCitPath(L"content_index.xml").c_str(), L"wb");

		if (outCachesFile)
		{
			cachesDoc.SaveFile(outCachesFile);
			fclose(outCachesFile);
		}
	}

	return retval;
}

bool CheckFileOutdatedWithUI(const wchar_t* fileName, const std::vector<std::array<uint8_t, 20>>& validHashes, uint64_t* fileStart, uint64_t fileTotal, std::array<uint8_t, 20>* foundHash, size_t checkSize)
{
	// if default changes, fix shouldCheck below
	bool fileOutdated = true;

	HANDLE hFile = CreateFile(fileName, GENERIC_READ | SYNCHRONIZE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		// skip the game root path if needed
		int fileNameOffset = 0;
		std::wstring gameRoot = MakeRelativeGamePath(L"");

		if (_wcsnicmp(fileName, gameRoot.c_str(), gameRoot.size()) == 0)
		{
			fileNameOffset = gameRoot.size();
		}

		std::wstring citizenRoot = MakeRelativeCitPath(L"");

		if (_wcsnicmp(fileName, citizenRoot.c_str(), citizenRoot.size()) == 0)
		{
			fileNameOffset = citizenRoot.size();
		}

		bool shouldCheck = true;

		// should we assume the file is outdated if the size is different?
		if (checkSize != -1 && !foundHash)
		{
			LARGE_INTEGER size = { 0 };
			GetFileSizeEx(hFile, &size);

			// we can just assume it's outdated already
			if (size.QuadPart != checkSize)
			{
				shouldCheck = false;
			}
		}

		// fileOutdated defaults to true, so we can safely skip the check
		if (shouldCheck)
		{
			UI_UpdateText(1, va(gettext(L"Checking %s"), &fileName[fileNameOffset]));

			LARGE_INTEGER fileSize;
			GetFileSizeEx(hFile, &fileSize);

			OVERLAPPED overlapped;

			SHA_CTX ctx;
			SHA1_Init(&ctx);

			bool doneReading = false;
			uint64_t fileOffset = 0;

			double lastProgress = 0.0;

			while (!doneReading)
			{
				memset(&overlapped, 0, sizeof(overlapped));
				overlapped.OffsetHigh = fileOffset >> 32;
				overlapped.Offset = fileOffset;

				std::vector<char> buffer(131072);
				if (ReadFile(hFile, buffer.data(), buffer.size(), NULL, &overlapped) == FALSE)
				{
					if (GetLastError() != ERROR_IO_PENDING && GetLastError() != ERROR_HANDLE_EOF)
					{
						UI_DisplayError(va(L"Reading of %s failed with error %i.", fileName, GetLastError()));
						return true;
					}

					if (GetLastError() == ERROR_HANDLE_EOF)
					{
						break;
					}

					while (true)
					{
						HANDLE pHandles[1];
						pHandles[0] = hFile;

						DWORD waitResult = MsgWaitForMultipleObjects(1, pHandles, FALSE, INFINITE, QS_ALLINPUT);

						if (waitResult == WAIT_OBJECT_0)
						{
							break;
						}

						MSG msg;
						while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}

						if (UI_IsCanceled())
						{
							return false;
						}
					}
				}

				DWORD bytesRead;
				BOOL olResult = GetOverlappedResult(hFile, &overlapped, &bytesRead, FALSE);
				DWORD err = GetLastError();

				SHA1_Update(&ctx, (uint8_t*)buffer.data(), bytesRead);

				if (bytesRead < buffer.size() || (!olResult && err == ERROR_HANDLE_EOF))
				{
					doneReading = true;
				}

				fileOffset += bytesRead;

				if (fileSize.QuadPart == 0)
				{
					UI_UpdateProgress(100.0);
				}
				else
				{
					double progress = ((*fileStart + fileOffset) / (double)fileTotal) * 100.0;

					if (abs(progress - lastProgress) > 0.5)
					{
						UI_UpdateProgress(progress);
						lastProgress = progress;
					}
				}
			}

			*fileStart += fileOffset;

			std::array<uint8_t, 20> outHash;
			SHA1_Final(outHash.data(), &ctx);

			if (foundHash)
			{
				*foundHash = outHash;
			}

			for (auto& hash : validHashes)
			{
				if (hash == outHash)
				{
					fileOutdated = false;
				}
			}
		}

		CloseHandle(hFile);
	}

	return fileOutdated;
}

static std::string updateChannel;

void ResetUpdateChannel()
{
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		WritePrivateProfileString(L"Game", L"UpdateChannel", L"production", fpath.c_str());
	}

	updateChannel = "";
}

const char* GetUpdateChannel()
{
	if (!updateChannel.size())
	{
		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

		if (GetFileAttributes(fpath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			updateChannel = "production";
			return updateChannel.c_str();
		}

		wchar_t channel[512];
		GetPrivateProfileString(L"Game", L"UpdateChannel", L"production", channel, _countof(channel), fpath.c_str());

		char channelS[512];
		wcstombs(channelS, channel, sizeof(channelS));

		updateChannel = channelS;

		// map prod -> production
		if (updateChannel == "prod")
		{
			updateChannel = "production";
		}

		// check file age, and revert to 'beta' if it's old
		if (updateChannel == "canary")
		{
			struct _stati64 st;
			if (_wstati64(fpath.c_str(), &st) >= 0)
			{
				// Mon Jan 03 2022 12:00:00 GMT+0000
				if (st.st_mtime < 1641211200)
				{
					updateChannel = "beta";
					WritePrivateProfileString(L"Game", L"UpdateChannel", L"beta", fpath.c_str());
				}
			}
		}
	}

	return updateChannel.c_str();
}
#endif
