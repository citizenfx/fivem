/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

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
	char url[512];
	char file[512];
	int64_t size;
	CURL* curlHandles[10];
	FILE* fp[10];
	bool compressed;
	bool conditionalDL;

	int segments;

	uint8_t conditionalHash[20];
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

	// these are global?!
	int64_t totalBytes;
	int64_t doneTotalBytes;

	bool downloadInitialized;
	CURLM* curl;

	lzma_stream strm;
	uint8_t strmOut[65535];

	bool isDownloading;
	download_t* currentDownload;
	std::queue<download_t> downloadQueue;
} dls;

void CL_InitDownloadQueue()
{
	dls.completedSize = 0;
	dls.totalSize = 0;
	dls.completedDownloads = 0;
	dls.numDownloads = 0;
	dls.currentDownload = NULL;
	dls.isDownloading = false;
	dls.lastTime = 0;
	dls.lastBytes = 0;
	dls.bytesPerSecond = 0;
	dls.totalBytes = 0;
	dls.doneTotalBytes = 0;
	
	std::queue<download_t> empty;
	std::swap(dls.downloadQueue, empty);
}

//void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed, const uint8_t* hash, uint32_t hashLen);

void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed)
{
	CL_QueueDownload(url, file, size, compressed, 1);
}

void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed, int segments)
{
	download_t download;
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


	dls.downloadQueue.push(download);

	dls.numDownloads++;
	dls.totalSize += size;
	dls.totalBytes = dls.totalSize;

	dls.isDownloading = true;
}

void DL_Initialize()
{
	dls.curl = curl_multi_init();
	dls.downloadInitialized = true;
}

void DL_Shutdown()
{
	curl_multi_cleanup(dls.curl);
	dls.downloadInitialized = false;
}

static download_t thisDownload;

void DL_DequeueDownload()
{
	download_t download = dls.downloadQueue.front();

	memcpy(&thisDownload, &download, sizeof(download_t));
	dls.currentDownload = &thisDownload;

	// process Rockstar URLs
	if (_strnicmp(dls.currentDownload->url, "rockstar:", 9) == 0)
	{
		std::string newUrl = va(ROCKSTAR_BASE_URL, &dls.currentDownload->url[9]);

		strcpy_s(dls.currentDownload->url, sizeof(dls.currentDownload->url), newUrl.c_str());
	}

	dls.downloadQueue.pop();
}

void DL_UpdateGlobalProgress(int thisSize)
{
	dls.doneTotalBytes += thisSize;

	double percentage = ((double)(dls.doneTotalBytes / 1000) / (dls.totalBytes / 1000)) * 100.0;

	UI_UpdateText(0, L"Updating " PRODUCT_NAME L"...");

	UI_UpdateText(1, va(L"Downloaded %.2f/%.2f MB (%.0f%%, %.1f MB/s)", (dls.doneTotalBytes / 1000) / 1000.f, ((dls.totalBytes / 1000) / 1000.f), percentage, dls.bytesPerSecond / (double)1000000));

	UI_UpdateProgress(percentage);
}

size_t DL_WriteToFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = 0;

	if (!dls.currentDownload->compressed)
	{
		written = fwrite(ptr, size, nmemb, stream);
	}
	else
	{
		dls.strm.next_in = (uint8_t*)ptr;
		dls.strm.avail_in = size * nmemb;

		while (dls.strm.avail_in)
		{
			dls.strm.next_out = dls.strmOut;
			dls.strm.avail_out = sizeof(dls.strmOut);

			lzma_ret ret = lzma_code(&dls.strm, LZMA_RUN);

			if (ret != LZMA_OK && ret != LZMA_STREAM_END)
			{
				MessageBoxA(NULL, va("LZMA decoding error %i in %s.", ret, dls.currentDownload->file), "Error", MB_OK | MB_ICONSTOP);
				ExitProcess(1);
			}

			fwrite(dls.strmOut, 1, (sizeof(dls.strmOut) - dls.strm.avail_out), stream);
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

	// poll message loop in case of file:// transfers or similar
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return written;
}

void DL_ProcessDownload()
{
	if (!dls.currentDownload)
	{
		return;
	}

	// build path stuff, TODO: should be saved somewhere intermediate
	char opath[256];
	char tmpPath[256];
	char tmpDir[256];
	opath[0] = '\0';
	//strcat_s(opath, sizeof(opath), ".");
	//strcat_s(opath, sizeof(opath), "/");
	strcat_s(opath, sizeof(opath), dls.currentDownload->file);
	strcpy_s(tmpPath, sizeof(tmpPath), opath);
	strcat_s(tmpPath, sizeof(tmpPath), ".tmp");

	for (char* p = tmpPath; *p; p++)
	{
		if (*p == '\\')
		{
			*p = '/';
		}
	}

	// wsc
	bool alreadyDone = false;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

	// create a new handle if we don't have one yet
	if (!dls.currentDownload->curlHandles[0])
	{
		// create the parent directory too
		strncpy(tmpDir, tmpPath, sizeof(tmpDir));

		char* slashPos = strrchr(tmpDir, '/');

		if (slashPos)
		{
			slashPos[0] = '\0';
			CreateDirectoryAnyDepth(tmpDir);
		}

		int64_t segSize = (dls.currentDownload->size / dls.currentDownload->segments);
		int64_t segRemainder = (dls.currentDownload->size % dls.currentDownload->segments);

		memset(&dls.strm, 0, sizeof(dls.strm));

		for (int i = 0; i < dls.currentDownload->segments; i++)
		{
			FILE* fp = nullptr;
			int64_t segStart = (i * segSize);
			int64_t segEnd = segStart + segSize;

			if (i > 0)
			{
				fp = _wfopen(converter.from_bytes(va("%s.%i", tmpPath, i)).c_str(), L"ab");
				_fseeki64(fp, 0, SEEK_END);
				segStart += _ftelli64(fp);

				dls.doneTotalBytes += _ftelli64(fp);
				dls.completedSize += _ftelli64(fp);

				if (i == (dls.currentDownload->segments - 1))
				{
					segEnd += segRemainder;
				}
			}
			else
			{
				fp = _wfopen(converter.from_bytes(tmpPath).c_str(), L"ab");

				_fseeki64(fp, 0, SEEK_END);
				segStart += _ftelli64(fp);

				dls.doneTotalBytes += _ftelli64(fp);
				dls.completedSize += _ftelli64(fp);
			}

			if (!fp)
			{
				dls.isDownloading = false;
				MessageBox(NULL, va(L"Unable to open %s for writing.", converter.from_bytes(opath).c_str()), L"Error", MB_OK | MB_ICONSTOP);

				ExitProcess(1);
				return;
			}

			dls.currentDownload->fp[i] = fp;

			if (segStart != segEnd)
			{
				curl_slist* headers = nullptr;
				headers = curl_slist_append(headers, va("Range: bytes=%llu-%llu", segStart, segEnd - 1));

				dls.currentDownload->curlHandles[i] = curl_easy_init();
				curl_easy_setopt(dls.currentDownload->curlHandles[i], CURLOPT_URL, dls.currentDownload->url);
				curl_easy_setopt(dls.currentDownload->curlHandles[i], CURLOPT_WRITEDATA, fp);
				curl_easy_setopt(dls.currentDownload->curlHandles[i], CURLOPT_WRITEFUNCTION, DL_WriteToFile);
				curl_easy_setopt(dls.currentDownload->curlHandles[i], CURLOPT_FAILONERROR, true);
				curl_easy_setopt(dls.currentDownload->curlHandles[i], CURLOPT_HTTPHEADER, headers);
				curl_easy_setopt(dls.currentDownload->curlHandles[i], CURLOPT_FOLLOWLOCATION, true);

				lzma_stream_decoder(&dls.strm, UINT32_MAX, 0);
				dls.strm.avail_out = sizeof(dls.strmOut);
				dls.strm.next_out = dls.strmOut;

				curl_multi_add_handle(dls.curl, dls.currentDownload->curlHandles[i]);
			}
			else
			{
				alreadyDone = true;
			}
		}
	}

	// perform the downloading loop bit
	int stillRunning;
	curl_multi_perform(dls.curl, &stillRunning);

	if (stillRunning == 0)
	{
		bool allOK = true;

		if (dls.currentDownload->fp[0])
		{
			for (int i = 0; i < dls.currentDownload->segments; i++)
			{
				fclose(dls.currentDownload->fp[i]);
			}
		}

		// check for success
		CURLMsg* info = NULL;

		std::wstring opathWide = converter.from_bytes(opath);
		std::wstring tmpPathWide = converter.from_bytes(tmpPath);

		do 
		{
			info = curl_multi_info_read(dls.curl, &stillRunning);

			if (info != NULL || alreadyDone)
			{
				if (alreadyDone || info->msg == CURLMSG_DONE)
				{
					CURLcode code = CURLE_OK;

					if (!alreadyDone)
					{
						code = info->data.result;
						CURL* handle = info->easy_handle;

						curl_multi_remove_handle(dls.curl, /*dls.currentDownload->curlHandle*/handle);
						curl_easy_cleanup(/*dls.currentDownload->curlHandle*/handle);
						lzma_end(&dls.strm);
					}

					if (code == CURLE_OK)
					{
						if (dls.currentDownload->segments == 1)
						{
							if (DeleteFile(opathWide.c_str()) == 0)
							{
								if (GetLastError() != ERROR_FILE_NOT_FOUND)
								{
									MessageBoxA(NULL, va("Deleting old %s failed (err = %d)", dls.currentDownload->url, GetLastError()), "Error", MB_OK | MB_ICONSTOP);
									
									ExitProcess(1);
								}
							}

							if (MoveFile(tmpPathWide.c_str(), opathWide.c_str()) == 0)//rename(tmpPath, opath) != 0)
							{
								MessageBoxA(NULL, va("Moving of %s failed (err = %d)", dls.currentDownload->url, GetLastError()), "Error", MB_OK | MB_ICONSTOP);
								DeleteFile(tmpPathWide.c_str());

								ExitProcess(1);
							}

							dls.completedDownloads++;
						}
					}
					else
					{
						_wunlink(tmpPathWide.c_str());
						MessageBoxA(NULL, va("Downloading of %s failed with CURLcode 0x%x", dls.currentDownload->url, code), "Error", MB_OK | MB_ICONSTOP);

						ExitProcess(1);
					}
				}
			}
		} while (info != NULL);

		if (dls.currentDownload->segments > 1)
		{
			_unlink(opath);

			FILE* newFile = _wfopen(opathWide.c_str(), L"wb");

			for (int i = 0; i < dls.currentDownload->segments; i++)
			{
				FILE* fp;

				if (i > 0)
				{
					fp = _wfopen(va(L"%s.%i", tmpPathWide.c_str(), i), L"rb");
				}
				else
				{
					fp = _wfopen(tmpPathWide.c_str(), L"rb");
				}

				char buffer[4096];
				int len;

				while ((len = fread(buffer, 1, sizeof(buffer), fp)) > 0)
				{
					if (fwrite(buffer, 1, len, newFile) != len)
					{
						fclose(newFile);
						_unlink(opath);

						MessageBoxA(NULL, va("Writing to %s failed.", dls.currentDownload->url), "Error", MB_OK | MB_ICONSTOP);

						ExitProcess(1);
					}
				}

				fclose(fp);
				
				if (i > 0)
				{
					_wunlink(va(L"%s.%i", tmpPathWide.c_str(), i));
				}
				else
				{
					_wunlink(tmpPathWide.c_str());
				}
			}

			fclose(newFile);

			//rename(tmpPath, opath);
			dls.completedDownloads++;
		}

		dls.currentDownload = NULL;
	}
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

	if (dls.currentDownload)
	{
		DL_ProcessDownload();
	}
	else
	{
		if (dls.downloadQueue.size() > 0)
		{
			DL_DequeueDownload();
		}
		else
		{
			return true;
		}
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