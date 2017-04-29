/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

// the maximum number of concurrent downloads
#define MAX_CONCURRENT_DOWNLOADS 6

// the maximum number of downloads allowed to be running before queueing a huge file
#define MAX_CONCURRENT_DOWNLOADS_HUGE 2

// what to consider a huge file
#define HUGE_SIZE (5 * 1024 * 1024)

// what to consider a mega-huge file (should be the sole file)
#define MEGA_HUGE_SIZE (25 * 1024 * 1024)

// rockstar: meta-protocol base URL
#define ROCKSTAR_BASE_URL "http://gtavp.citizen.re/file?name=%s"

#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>

#include <math.h>
#include <queue>

#define restrict
#define LZMA_API_STATIC
#include <lzma.h>
#undef restrict

typedef struct download_s
{
	std::string opath;
	std::string tmpPath;

	char url[512];
	char file[512];
	int64_t size;
	CURL* curlHandles[10];
	FILE* fp[10];
	bool compressed;
	bool conditionalDL;

	int segments;

	uint8_t conditionalHash[20];

	lzma_stream strm;
	uint8_t strmOut[65535];
} download_t;

struct dlState
{
	char downloadList[8192];
	int numDownloads;
	int completedDownloads;
	int64_t totalSize;
	int64_t completedSize;

	uint32_t lastTime;
	int64_t lastBytes;
	int bytesPerSecond;

	uint32_t lastPoll;

	// these are global?!
	int64_t totalBytes;
	int64_t doneTotalBytes;

	bool downloadInitialized;
	CURLM* curl;
	
	bool isDownloading;

	std::queue<std::shared_ptr<download_t>> downloadQueue;
	std::vector<std::shared_ptr<download_t>> currentDownloads;
} dls;

void CL_InitDownloadQueue()
{
	dls.completedSize = 0;
	dls.totalSize = 0;
	dls.completedDownloads = 0;
	dls.numDownloads = 0;
	dls.isDownloading = false;
	dls.lastTime = 0;
	dls.lastBytes = 0;
	dls.bytesPerSecond = 0;
	dls.totalBytes = 0;
	dls.doneTotalBytes = 0;
	
	std::queue<std::shared_ptr<download_t>> empty;
	std::swap(dls.downloadQueue, empty);
}

//void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed, const uint8_t* hash, uint32_t hashLen);

void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed)
{
	CL_QueueDownload(url, file, size, compressed, 1);
}

void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed, int segments)
{
	auto downloadPtr = std::make_shared<download_t>();
	auto& download = *downloadPtr.get();

	sprintf_s(download.url, sizeof(download.url), "%s", url);
	strcpy_s(download.file, sizeof(download.file), file);
	download.size = size;
	download.compressed = compressed;
	download.segments = segments;

	for (int i = 0; i < segments; i++)
	{
		download.curlHandles[i] = NULL;
		download.fp[i] = NULL;
	}


	dls.downloadQueue.push(downloadPtr);

	dls.numDownloads++;
	dls.totalSize += size;
	dls.totalBytes = dls.totalSize;

	dls.isDownloading = true;
}

void DL_Initialize()
{
	dls.curl = curl_multi_init();
	dls.downloadInitialized = true;

	curl_multi_setopt(dls.curl, CURLMOPT_PIPELINING, CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX);
}

void DL_Shutdown()
{
	curl_multi_cleanup(dls.curl);
	dls.downloadInitialized = false;
}

static download_t thisDownload;

void DL_DequeueDownload()
{
	auto download = dls.downloadQueue.front();

	// limit concurrent downloads
	if (dls.currentDownloads.size() >= MAX_CONCURRENT_DOWNLOADS)
	{
		return;
	}

	// is this download 'huge'? then skip until we're the MAX_CONCURRENT_DOWNLOADS_HUGEth file
	if (dls.currentDownloads.size() >= MAX_CONCURRENT_DOWNLOADS_HUGE)
	{
		if (download->size > HUGE_SIZE)
		{
			return;
		}
	}

	// is this a mega-huge file? then only allow this file
	if (dls.currentDownloads.size() >= 1)
	{
		if (download->size > MEGA_HUGE_SIZE)
		{
			return;
		}
	}

	dls.currentDownloads.push_back(download);

	// process Rockstar URLs
	if (_strnicmp(download->url, "rockstar:", 9) == 0)
	{
		std::string newUrl = va(ROCKSTAR_BASE_URL, &download->url[9]);

		strcpy_s(download->url, sizeof(download->url), newUrl.c_str());
	}

	dls.downloadQueue.pop();
}

void DL_UpdateGlobalProgress(size_t thisSize)
{
	dls.doneTotalBytes += thisSize;

	double percentage = ((double)(dls.doneTotalBytes / 1000) / (dls.totalBytes / 1000)) * 100.0;

	UI_UpdateText(0, L"Updating " PRODUCT_NAME L"...");

	UI_UpdateText(1, va(L"Downloaded %.2f/%.2f MB (%.0f%%, %.1f MB/s)", (dls.doneTotalBytes / 1000) / 1000.f, ((dls.totalBytes / 1000) / 1000.f), percentage, dls.bytesPerSecond / (double)1000000));

	UI_UpdateProgress(percentage);
}

size_t DL_WriteToFile(void *ptr, size_t size, size_t nmemb, download_t* download)
{
	size_t written = 0;

	if (!download->compressed)
	{
		written = fwrite(ptr, size, nmemb, download->fp[0]);
	}
	else
	{
		download->strm.next_in = (uint8_t*)ptr;
		download->strm.avail_in = size * nmemb;

		while (download->strm.avail_in)
		{
			download->strm.next_out = download->strmOut;
			download->strm.avail_out = sizeof(download->strmOut);

			lzma_ret ret = lzma_code(&download->strm, LZMA_RUN);

			if (ret != LZMA_OK && ret != LZMA_STREAM_END)
			{
				MessageBoxA(NULL, va("LZMA decoding error %i in %s.", ret, download->file), "Error", MB_OK | MB_ICONSTOP);
				ExitProcess(1);
			}

			fwrite(download->strmOut, 1, (sizeof(download->strmOut) - download->strm.avail_out), download->fp[0]);
		}

		written = size * nmemb;
	}

	// do size calculations
	dls.completedSize += (size * nmemb);
	if ((dls.lastTime + 1000) < GetTickCount())
	{
		dls.bytesPerSecond = (int)(dls.completedSize - dls.lastBytes);
		dls.lastTime = GetTickCount();
		dls.lastBytes = dls.completedSize;
	}

	DL_UpdateGlobalProgress(size * nmemb);

	if ((GetTickCount() - dls.lastPoll) > 50)
	{
		// poll message loop in case of file:// transfers or similar
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		dls.lastPoll = GetTickCount();
	}

	return written;
}

static int DL_CurlDebug(CURL *handle,
	curl_infotype type,
	char *data,
	size_t size,
	void *userptr)
{
	if (type == CURLINFO_TEXT)
	{
		if (strstr(data, "schannel") == nullptr)
		{
			trace("CURL: %s", std::string(data, size));
		}
	}

	return 0;
}

void DL_ProcessDownload()
{
	if (!dls.currentDownloads.size())
	{
		return;
	}

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	// create new handles if we don't have one yet
	for (auto& download : dls.currentDownloads)
	{
		if (!download->curlHandles[0])
		{
			// build path stuff
			char opath[256];
			char tmpPath[256];
			char tmpDir[256];
			opath[0] = '\0';
			strcat_s(opath, sizeof(opath), download->file);
			strcpy_s(tmpPath, sizeof(tmpPath), opath);
			strcat_s(tmpPath, sizeof(tmpPath), ".tmp");

			for (char* p = tmpPath; *p; p++)
			{
				if (*p == '\\')
				{
					*p = '/';
				}
			}

			download->opath = opath;
			download->tmpPath = tmpPath;

			// create the parent directory too
			strncpy(tmpDir, tmpPath, sizeof(tmpDir));

			char* slashPos = strrchr(tmpDir, '/');

			if (slashPos)
			{
				slashPos[0] = '\0';
				CreateDirectoryAnyDepth(tmpDir);
			}

			memset(&download->strm, 0, sizeof(download->strm));

			FILE* fp = nullptr;

			fp = _wfopen(converter.from_bytes(tmpPath).c_str(), L"wb");

			if (!fp)
			{
				dls.isDownloading = false;
				MessageBox(NULL, va(L"Unable to open %s for writing.", converter.from_bytes(opath).c_str()), L"Error", MB_OK | MB_ICONSTOP);

				ExitProcess(1);
				return;
			}

			download->fp[0] = fp;

			curl_slist* headers = nullptr;
			headers = curl_slist_append(headers, va("X-Cfx-Client: 1"));

			auto curlHandle = curl_easy_init();
			download->curlHandles[0] = curlHandle;

			curl_easy_setopt(curlHandle, CURLOPT_URL, download->url);
			curl_easy_setopt(curlHandle, CURLOPT_PRIVATE, download.get());
			curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, download.get());
			curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, DL_WriteToFile);
			curl_easy_setopt(curlHandle, CURLOPT_FAILONERROR, true);
			curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, true);
			curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);

			if (getenv("CFX_CURL_DEBUG"))
			{
				curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1);
				curl_easy_setopt(curlHandle, CURLOPT_DEBUGFUNCTION, DL_CurlDebug);
			}

			lzma_stream_decoder(&download->strm, UINT32_MAX, 0);
			download->strm.avail_out = sizeof(download->strmOut);
			download->strm.next_out = download->strmOut;

			curl_multi_add_handle(dls.curl, curlHandle);
		}
	}

	// perform the downloading loop bit
	int stillRunning;
	curl_multi_perform(dls.curl, &stillRunning);

	CURLMsg* info = NULL;

	do
	{
		// check for success
		info = curl_multi_info_read(dls.curl, &stillRunning);

		if (info != NULL)
		{
			if (info->msg == CURLMSG_DONE)
			{
				CURLcode code = info->data.result;
				CURL* handle = info->easy_handle;

				char* outPtr;
				curl_easy_getinfo(handle, CURLINFO_PRIVATE, &outPtr);

				auto it = std::find_if(dls.currentDownloads.begin(), dls.currentDownloads.end(), [&](auto& d)
				{
					return ((char*)d.get() == outPtr);
				});

				if (it == dls.currentDownloads.end())
				{
					continue;
				}

				auto download = *it;

				bool allOK = true;

				if (download->fp[0])
				{
					fclose(download->fp[0]);
				}

				std::wstring opathWide = converter.from_bytes(download->opath);
				std::wstring tmpPathWide = converter.from_bytes(download->tmpPath);

				curl_multi_remove_handle(dls.curl, handle);
				curl_easy_cleanup(handle);
				lzma_end(&download->strm);

				if (code == CURLE_OK)
				{
					if (DeleteFile(opathWide.c_str()) == 0)
					{
						if (GetLastError() != ERROR_FILE_NOT_FOUND)
						{
							MessageBoxA(NULL, va("Deleting old %s failed (err = %d)", download->url, GetLastError()), "Error", MB_OK | MB_ICONSTOP);

							ExitProcess(1);
						}
					}

					if (MoveFile(tmpPathWide.c_str(), opathWide.c_str()) == 0)//rename(tmpPath, opath) != 0)
					{
						MessageBoxA(NULL, va("Moving of %s failed (err = %d)", download->url, GetLastError()), "Error", MB_OK | MB_ICONSTOP);
						DeleteFile(tmpPathWide.c_str());

						ExitProcess(1);
					}

					dls.completedDownloads++;

					dls.currentDownloads.erase(it);
				}
				else
				{
					_wunlink(tmpPathWide.c_str());
					MessageBoxA(NULL, va("Downloading of %s failed with CURLcode 0x%x", download->url, (int)code), "Error", MB_OK | MB_ICONSTOP);

					ExitProcess(1);
				}
			}
		}

	} while (info != NULL);
}

bool DL_Process()
{
	if (!dls.isDownloading)
	{
		if (dls.downloadInitialized)
		{
			DL_Shutdown();
		}

		return true;
	}

	if (!dls.downloadInitialized)
	{
		DL_Initialize();
	}

	if (!dls.currentDownloads.empty())
	{
		DL_ProcessDownload();
	}
	
	if (dls.downloadQueue.size() > 0)
	{
		DL_DequeueDownload();
	}
	
	if (dls.downloadQueue.empty() && dls.currentDownloads.empty())
	{
		return true;
	}

	return false;
}

struct BufferData
{
	char* buffer;
	size_t curSize;
	size_t maxSize;
};

static size_t RequestDataReceived(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t rsize = (size * nmemb);
	char* text = (char*)ptr;

	BufferData* d = (BufferData*)data;

	if ((d->curSize + rsize) >= d->maxSize)
	{
		rsize = (d->maxSize - d->curSize) - 1;
	}

	memcpy(&d->buffer[d->curSize], ptr, rsize);
	d->curSize += rsize;

	return (size * nmemb);
}

int DL_RequestURL(const char* url, char* buffer, size_t bufSize)
{
	curl_global_init(CURL_GLOBAL_ALL);

	CURL* curl = curl_easy_init();

	if (curl)
	{
		BufferData bufferData;
		bufferData.buffer = buffer;
		bufferData.curSize = 0;
		bufferData.maxSize = bufSize;

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RequestDataReceived);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bufferData);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "CitizenIV");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

		CURLcode code = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		curl_global_cleanup();

		buffer[bufferData.curSize] = '\0';

		if (code == CURLE_OK)
		{
			return 0;
		}
		else
		{
			return (int)code;
		}
	}

	curl_global_cleanup();

	return 0;
}

bool DL_RunLoop()
{
	while (!DL_Process())
	{
		HANDLE pHandles[1];
		pHandles[0] = GetCurrentThread();

		DWORD waitResult = MsgWaitForMultipleObjects(1, pHandles, FALSE, 20, QS_ALLINPUT);

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

	return true;
}