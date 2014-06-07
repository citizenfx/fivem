#include "StdInc.h"
#include <tinyxml.h>

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
		TiXmlDocument doc;
		doc.Parse(str);

		TiXmlElement* rootElement = doc.RootElement();
		TiXmlElement* cacheElement = rootElement->FirstChildElement("Cache");

		while (cacheElement)
		{
			cache_t cache;
			cache.name = cacheElement->Attribute("ID");
			cacheElement->Attribute("Version", &cache.version);

			caches.push_back(cache);

			cacheElement = cacheElement->NextSiblingElement("Cache");
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
		TiXmlDocument doc;
		doc.Parse(str);

		TiXmlElement* rootElement = doc.RootElement();
		TiXmlElement* fileElement = rootElement->FirstChildElement("ContentFile");

		while (fileElement)
		{
			manifestFile_t file;
			file.name = fileElement->Attribute("Name");

			int size, compressedSize;
			fileElement->Attribute("Size", &size);
			fileElement->Attribute("CompressedSize", &compressedSize);

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

	std::list<cache_t> needsUpdate;

	FILE* cachesReader = fopen("caches.xml", "r");

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

	for (auto& filePair : queuedFiles)
	{
		cache_t& cache = filePair.first;
		manifestFile_t& file = filePair.second;

		// check file hash first
		bool fileOutdated = true;

		HANDLE hFile = CreateFileA(file.name.c_str(), GENERIC_READ | SYNCHRONIZE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		static wchar_t fnameWide[512];
		MultiByteToWideChar(CP_ACP, 0, file.name.c_str(), -1, fnameWide, sizeof(fnameWide) / 2);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			UI_UpdateText(1, va(L"Checking %s", fnameWide));

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

				char buffer[4096];
				if (ReadFile(hFile, buffer, sizeof(buffer), NULL, &overlapped) == FALSE)
				{
					if (GetLastError() != ERROR_IO_PENDING && GetLastError() != ERROR_HANDLE_EOF)
					{
						MessageBox(NULL, va(L"Reading of %s failed with error %i.", fnameWide, GetLastError()), L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
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
			}

			uint8_t outHash[20];
			SHA1Result(&ctx, outHash);

			if (!memcmp(file.hash, outHash, 20))
			{
				fileOutdated = false;
			}
		}

		CloseHandle(hFile);
		CloseHandle(hEvent);

		if (fileOutdated)
		{
			const char* url = va(CONTENT_URL "/%s/content/%s/%s%s", GetUpdateChannel(), cache.name.c_str(), file.name.c_str(), (file.compressed) ? ".xz" : "");

			CL_QueueDownload(url, file.name.c_str(), file.downloadSize, file.compressed);
		}
	}

	bool retval = DL_RunLoop();

	UI_DoDestruction();

	if (retval)
	{
		TiXmlDocument doc;
		TiXmlElement* rootElement = new TiXmlElement("Caches");

		for (cache_t& cache : cacheFile.GetCaches())
		{
			TiXmlElement* element = new TiXmlElement("Cache");
			element->SetAttribute("ID", cache.name.c_str());
			element->SetAttribute("Version", cache.version);

			rootElement->LinkEndChild(element);
		}

		doc.LinkEndChild(rootElement);

		TiXmlPrinter printer;
		doc.Accept(&printer);

		const char* data = printer.CStr();
		
		FILE* f = fopen("caches.xml", "w");
		if (f)
		{
			fwrite(data, 1, strlen(data), f);
			fclose(f);
		}
	}

	return retval;
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