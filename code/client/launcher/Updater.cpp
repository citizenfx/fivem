/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <tinyxml2.h>

#include <stdint.h>

#include <list>
#include <unordered_set>
#include <string>

#include "sha1.h"

struct cache_t
{
	std::string name;
	int version;
};

struct cacheFile_t
{
private:
	std::list<cache_t> caches;

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

				caches.push_back(cache);

				cacheElement = cacheElement->NextSiblingElement("Cache");
			}
		}
	}

	std::list<cache_t>& GetCaches()
	{
		return caches;
	}

	cache_t GetCache(std::string name)
	{
		for (cache_t& cache : caches)
		{
			if (cache.name == name)
			{
				return cache;
			}
		}

		return cache_t();
	}
};

struct manifestFile_t
{
	std::string name;
	size_t downloadSize;
	bool compressed;
	uint8_t hash[20];
};

struct manifest_t
{
private:
	std::list<manifestFile_t> files;
	cache_t& parentCache;

public:
	manifest_t(cache_t& parent)
		: parentCache(parent)
	{

	}

	void Parse(const char* str)
	{
		tinyxml2::XMLDocument doc;
		doc.Parse(str);

		auto rootElement = doc.RootElement();
		auto fileElement = rootElement->FirstChildElement("ContentFile");

		while (fileElement)
		{
			manifestFile_t file;
			file.name = fileElement->Attribute("Name");

			int size, compressedSize;
			size = atoi(fileElement->Attribute("Size"));
			compressedSize = atoi(fileElement->Attribute("CompressedSize"));

			file.compressed = (size != compressedSize);
			file.downloadSize = compressedSize;

			ParseHash(fileElement->Attribute("SHA1Hash"), file.hash);

			files.push_back(file);

			fileElement = fileElement->NextSiblingElement("ContentFile");
		}
	}

	std::list<manifestFile_t>& GetFiles()
	{
		return files;
	}

	cache_t& GetParentCache()
	{
		return parentCache;
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

bool Updater_RunUpdate(int numCaches, ...)
{
	static char cachesFile[800000];

	int result = DL_RequestURL(va(CONTENT_URL "/%s/content/caches.xml", GetUpdateChannel()), cachesFile, sizeof(cachesFile));

	if (result != 0)
	{
		MessageBox(NULL, va(L"An error (%i) occurred while checking the game version. Check if " CONTENT_URL_WIDE L" is available in your web browser.", result), L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
		return false;
	}

	// parse wanted caches
	std::unordered_set<std::string> wantedCaches;
	va_list ap;
	va_start(ap, numCaches);

	for (int i = 0; i < numCaches; i++)
	{
		wantedCaches.insert(va_arg(ap, const char*));
	}

	va_end(ap);

	// get the caches we want to update
	cacheFile_t cacheFile;
	cacheFile.Parse(cachesFile);

	// error out if the remote caches file is empty
	if (cacheFile.GetCaches().empty())
	{
		MessageBox(NULL, L"Remote caches file could not be parsed. Check if " CONTENT_URL_WIDE L" is available in your web browser.", L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
		return false;
	}

	std::list<cache_t> needsUpdate;

	FILE* cachesReader = _wfopen(MakeRelativeCitPath(L"caches.xml").c_str(), L"r");

	if (!cachesReader)
	{
		for (cache_t& cache : cacheFile.GetCaches())
		{
			if (wantedCaches.find(cache.name) != wantedCaches.end())
			{
				needsUpdate.push_back(cache);
			}
		}
	}
	else
	{
		int length = fread(cachesFile, 1, sizeof(cachesFile), cachesReader);
		fclose(cachesReader);

		cachesFile[length] = '\0';

		cacheFile_t localCacheFile;
		localCacheFile.Parse(cachesFile);

		for (cache_t& cache : cacheFile.GetCaches())
		{
			if (wantedCaches.find(cache.name) != wantedCaches.end())
			{
				cache_t& localCache = localCacheFile.GetCache(cache.name);

				if (localCache.version != cache.version)
				{
					needsUpdate.push_back(cache);
				}
			}
		}
	}

	// if we don't need to update, return true
	if (needsUpdate.size() == 0)
	{
		return true;
	}

	// fetch cache manifests and enqueue downloads
	std::list<std::pair<cache_t, manifestFile_t>> queuedFiles;

	for (cache_t& cache : needsUpdate)
	{
		result = DL_RequestURL(va(CONTENT_URL "/%s/content/%s/info.xml", GetUpdateChannel(), cache.name.c_str()), cachesFile, sizeof(cachesFile));

		manifest_t manifest(cache);
		manifest.Parse(cachesFile);

		for (manifestFile_t& file : manifest.GetFiles())
		{
			queuedFiles.push_back(std::make_pair(cache, file));
		}
	}

	UI_DoCreation();

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	for (auto& filePair : queuedFiles)
	{
		cache_t& cache = filePair.first;
		manifestFile_t& file = filePair.second;

		// check file hash first
		bool fileOutdated = CheckFileOutdatedWithUI(MakeRelativeCitPath(converter.from_bytes(file.name)).c_str(), file.hash);

		if (fileOutdated)
		{
			const char* url = va(CONTENT_URL "/%s/content/%s/%s%s", GetUpdateChannel(), cache.name.c_str(), file.name.c_str(), (file.compressed) ? ".xz" : "");

			std::string outPath = converter.to_bytes(MakeRelativeCitPath(converter.from_bytes(file.name)));

			CL_QueueDownload(url, outPath.c_str(), file.downloadSize, file.compressed);
		}
	}

	bool retval = DL_RunLoop();

	UI_DoDestruction();

	if (retval)
	{
		// there used to be an XML writing library here to write the file properly
		// but people trusting VirusTotal caused me to have to remove it
		// (both TinyXML1/2/3 and RapidXML cause 'Gen:Variant.Kazy.454890' detections
		//  by around 7 different antivirus applications all using the same codebase,
		//  and therefore writing XML like this is the only way antivirus scampanies
		//  allow me to write XML from a 'suspicious application' like CitiLaunch...)
		//
		// TinyXML2 (3.0) does not seem to cause this detection, but only if its
		// XMLWriter class isn't used - TinyXML1 and boost::property_tree w/ RapidXML
		// both still cause this detection.

		FILE* outCachesFile = _wfopen(MakeRelativeCitPath(L"caches.xml").c_str(), L"w");
		
		if (outCachesFile)
		{
			fprintf(outCachesFile, "<Caches>\n");
			
			for (cache_t& cache : cacheFile.GetCaches())
			{
				fprintf(outCachesFile, "\t<Cache ID=\"%s\" Version=\"%d\" />\n", cache.name.c_str(), cache.version);
			}

			fprintf(outCachesFile, "</Caches>");
			fclose(outCachesFile);
		}
	}

	return retval;
}

bool CheckFileOutdatedWithUI(const wchar_t* fileName, const uint8_t hash[20])
{
	bool fileOutdated = true;

	HANDLE hFile = CreateFile(fileName, GENERIC_READ | SYNCHRONIZE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

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

		UI_UpdateText(1, va(L"Checking %s", &fileName[fileNameOffset]));
		UI_UpdateProgress(0.0);

		LARGE_INTEGER fileSize;
		GetFileSizeEx(hFile, &fileSize);

		OVERLAPPED overlapped;

		SHA1Context ctx;
		SHA1Reset(&ctx);

		bool doneReading = false;
		DWORD fileOffset = 0;

		while (!doneReading)
		{
			memset(&overlapped, 0, sizeof(overlapped));
			overlapped.OffsetHigh = 0;
			overlapped.Offset = fileOffset;

			char buffer[131072];
			if (ReadFile(hFile, buffer, sizeof(buffer), NULL, &overlapped) == FALSE)
			{
				if (GetLastError() != ERROR_IO_PENDING && GetLastError() != ERROR_HANDLE_EOF)
				{
					MessageBox(NULL, va(L"Reading of %s failed with error %i.", fileName, GetLastError()), L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
					return false;
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

			SHA1Input(&ctx, (uint8_t*)buffer, bytesRead);

			if (bytesRead < sizeof(buffer) || (!olResult && err == ERROR_HANDLE_EOF))
			{
				doneReading = true;
			}

			fileOffset += bytesRead;

			UI_UpdateProgress((fileOffset / (double)fileSize.QuadPart) * 100.0);
		}

		uint8_t outHash[20];
		SHA1Result(&ctx, outHash);

		if (!memcmp(hash, outHash, 20))
		{
			fileOutdated = false;
		}
	}

	CloseHandle(hFile);
	CloseHandle(hEvent);

	return fileOutdated;
}

const char* GetUpdateChannel()
{
	static std::string updateChannel;

	if (!updateChannel.size())
	{
		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

		if (GetFileAttributes(fpath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			updateChannel = "prod";
			return updateChannel.c_str();
		}

		wchar_t channel[512];
		GetPrivateProfileString(L"Game", L"UpdateChannel", L"prod", channel, _countof(channel), fpath.c_str());

		char channelS[512];
		wcstombs(channelS, channel, sizeof(channelS));

		updateChannel = channelS;
	}

	return updateChannel.c_str();
}