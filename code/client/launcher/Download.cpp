/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#if defined(LAUNCHER_PERSONALITY_MAIN) || defined(COMPILING_GLUE)
#include <CfxLocale.h>

// the maximum number of concurrent downloads
#define MAX_CONCURRENT_DOWNLOADS 6

// the maximum number of downloads allowed to be running before queueing a huge file
#define MAX_CONCURRENT_DOWNLOADS_HUGE 2

// what to consider a huge file
#define HUGE_SIZE (10 * 1024 * 1024)

// what to consider a mega-huge file (should be the sole file)
#define MEGA_HUGE_SIZE (40 * 1024 * 1024)

#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>

#ifdef CURL_MBEDTLS
#include <mbedtls/ssl.h>
#include <mbedtls/x509.h>

#include "SSLRoots.h"
#endif

#include <math.h>
#include <queue>
#include <sstream>

static bool fallbackPoll = true;

static std::string_view GetBaseName(std::string_view str)
{
	return str.substr(str.find_last_of('/') + 1);
}

#ifdef CURL_MBEDTLS
static CURLcode ssl_ctx_callback(CURL* curl, void* ssl_ctx, void* userptr)
{
	auto config = (mbedtls_ssl_config*)ssl_ctx;

	mbedtls_ssl_conf_ca_chain(config,
		(mbedtls_x509_crt*)userptr,
		nullptr);

	return CURLE_OK;
}
#endif

static CURL* curl_easy_init_cfx()
{
	auto curlHandle = curl_easy_init();

	if (curlHandle)
	{
		curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
		curl_easy_setopt(curlHandle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2 | CURL_SSLVERSION_MAX_TLSv1_2);

		char* curlVer = curl_version();

		if (strstr(curlVer, "mbedTLS/") != nullptr)
		{
#ifdef CURL_MBEDTLS
			static mbedtls_x509_crt cacert;
			mbedtls_x509_crt_init(&cacert);
			mbedtls_x509_crt_parse(&cacert, sslRoots, sizeof(sslRoots));

			curl_easy_setopt(curlHandle, CURLOPT_SSL_CTX_DATA, &cacert);
			curl_easy_setopt(curlHandle, CURLOPT_SSL_CTX_FUNCTION, ssl_ctx_callback);
#else
			assert(false);
#endif
		}
	}

	return curlHandle;
}

#define restrict
#define LZMA_API_STATIC
#include <lzma.h>
#undef restrict

#include <zstd.h>

typedef struct download_s
{
	std::string opath;
	std::string tmpPath;

	char url[512];
	char file[512];
	int64_t size;
	CURL* curlHandles[10];
	FILE* fp[10];
	compressionAlgo_e algorithm;
	bool conditionalDL;
	bool doneExternal;
	bool successExternal;

	bool writeToMemory;
	std::stringstream memoryStream;

	int segments;

	uint8_t conditionalHash[20];

	lzma_stream strm;
	uint8_t strmOut[65535];

	ZSTD_DStream* zstrm;
	ZSTD_inBuffer zin;
	ZSTD_outBuffer zout;

	int64_t progress = 0;
	int numRetries = 10;

	char curlError[CURL_ERROR_SIZE * 4];
} download_t;

struct dlState
{
	char downloadList[8192];
	int numDownloads;
	int completedDownloads;
	int64_t totalSize;
	int64_t completedSize;

	uint64_t lastTime;
	int64_t lastBytes;
	int bytesPerSecond;

	uint64_t lastPoll;
	uint64_t lastPump;

	// these are global?!
	int64_t totalBytes;
	int64_t doneTotalBytes;

	bool downloadInitialized;
	CURLM* curl;
	
	bool isDownloading;
	bool error;

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
	dls.error = false;
	
	dls.downloadQueue = {};
	dls.currentDownloads = {};
}

void CL_QueueDownload(const char* url, const char* file, int64_t size, compressionAlgo_e algo)
{
	CL_QueueDownload(url, file, size, algo, 1);
}

void CL_QueueDownload(const char* url, const char* file, int64_t size, compressionAlgo_e algo, int segments)
{
	if (strcmp(url, "https://runtime.fivem.net/patches/GTA_V_Patch_1_0_1604_0.exe") == 0)
	{
		for (int i = 0; i <= 9; i++)
		{
			CL_QueueDownload(va("https://content.cfx.re/mirrors/emergency_mirror/GTAV1604.exe%02d", i), va("%s.%d", file, i), i == 9 ? 87584200 : 104857600, compressionAlgo_e::None, 1);
		}

		return;
	}

	auto downloadPtr = std::make_shared<download_t>();
	auto& download = *downloadPtr.get();

	sprintf_s(download.url, sizeof(download.url), "%s", url);
	strcpy_s(download.file, sizeof(download.file), file);
	download.size = size;
	download.algorithm = algo;
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
	for (const auto& download : dls.currentDownloads)
	{
		if (download->fp[0])
		{
			fclose(download->fp[0]);
		}
	}

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

	dls.downloadQueue.pop();
}

static std::mutex g_globalProgressMutex;

void DL_UpdateGlobalProgress(size_t thisSize, uint64_t now = 0)
{
	std::unique_lock _(g_globalProgressMutex);

	if (!now)
	{
		now = GetTickCount64();
	}

	dls.doneTotalBytes += thisSize;

	if ((now - dls.lastTime) > 1000)
	{
		dls.bytesPerSecond = (int)(dls.completedSize - dls.lastBytes);
		dls.lastTime = now;
		dls.lastBytes = dls.completedSize;
	}

	double percentage = ((double)(dls.doneTotalBytes / 1000) / (dls.totalBytes / 1000)) * 100.0;

	UI_UpdateText(1, va(gettext(L"Downloaded %.2f/%.2f MB (%.0f%%, %.1f MB/s)"), (dls.doneTotalBytes / 1000) / 1000.f, ((dls.totalBytes / 1000) / 1000.f), percentage, dls.bytesPerSecond / (double)1000000));

	UI_UpdateProgress(percentage);
}

size_t DL_WriteToFile(void *ptr, size_t size, size_t nmemb, download_t* download)
{
	size_t written = 0;

	if (download->algorithm == compressionAlgo_e::None)
	{
		if (download->writeToMemory)
		{
			download->memoryStream << std::string(reinterpret_cast<char*>(ptr), size * nmemb);

			written = nmemb;
		}
		else
		{
			written = fwrite(ptr, size, nmemb, download->fp[0]);
		}
	}
	else if (download->algorithm == compressionAlgo_e::XZ)
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
				UI_DisplayError(va(L"LZMA decoding error %i in %s.", ret, ToWide(download->file)));
				return 0;
			}

			if (download->writeToMemory)
			{
				download->memoryStream << std::string(reinterpret_cast<char*>(download->strmOut), sizeof(download->strmOut) - download->strm.avail_out);
			}
			else
			{
				fwrite(download->strmOut, 1, (sizeof(download->strmOut) - download->strm.avail_out), download->fp[0]);
			}
		}

		written = nmemb;
	}
	else if (download->algorithm == compressionAlgo_e::Zstd)
	{
		download->zin = { (uint8_t*)ptr, size * nmemb, 0 };

		while (download->zin.pos < download->zin.size)
		{
			download->zout = { download->strmOut, sizeof(download->strmOut), 0 };

			size_t ret = ZSTD_decompressStream(download->zstrm, &download->zout, &download->zin);

			if (ZSTD_isError(ret))
			{
				UI_DisplayError(va(L"ZSTD decoding error %i (%s) in %s.", ret, ToWide(ZSTD_getErrorName(ret)), ToWide(download->file)));
				return 0;
			}

			if (download->writeToMemory)
			{
				download->memoryStream << std::string(reinterpret_cast<char*>(download->strmOut), download->zout.pos);
			}
			else
			{
				fwrite(download->strmOut, 1, download->zout.pos, download->fp[0]);
			}
		}

		written = nmemb;
	}

	// do size calculations
	auto now = GetTickCount64();
	download->progress += (size * nmemb);
	dls.completedSize += (size * nmemb);

	if ((now - dls.lastPoll) > 50)
	{
		DL_UpdateGlobalProgress(size * nmemb, now);

		dls.lastPoll = now;
	}
	else
	{
		// this is usually done in DL_UpdateGlobalProgress
		dls.doneTotalBytes += size * nmemb;
	}

	if ((now - dls.lastPump) > 100)
	{
		// poll message loop in case of file:// transfers or similar
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		dls.lastPump = now;
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

struct IpfsLibrary
{
public:
	IpfsLibrary()
		: success(false)
	{
		self = this;

		assert(LoadLibrary(MakeRelativeCitPath(L"CoreRT.dll").c_str()));
		hMod = LoadLibrary(MakeRelativeCitPath(L"ipfsdl.dll").c_str());

		if (hMod)
		{
			ipfsdlInit = (decltype(ipfsdlInit))GetProcAddress(hMod, "ipfsdlInit");
			ipfsdlExit = (decltype(ipfsdlExit))GetProcAddress(hMod, "ipfsdlExit");
			ipfsdlPoll = (decltype(ipfsdlPoll))GetProcAddress(hMod, "ipfsdlPoll");
			ipfsdlDownloadFile = (decltype(ipfsdlDownloadFile))GetProcAddress(hMod, "ipfsdlDownloadFile");

			if (ipfsdlInit)
			{
				ipfsdlInit(&OnInitStatic);
				success = true;
			}
		}
	}

	~IpfsLibrary()
	{
		ipfsdlExit();
	}

private:
	static inline IpfsLibrary* self;

	static void OnInitStatic(bool succeeded)
	{
		self->OnInit(succeeded);
	}

	void OnInit(bool succeeded)
	{
		if (!succeeded)
		{
			success = false;
		}

		std::unique_lock _(m_initCbMutex);

		if (m_initCb)
		{
			m_initCb();
			m_initCb = {};
		}

		m_inited = true;
	}

public:
	bool DownloadFile(const char* url, const std::function<bool(const void*, size_t)>& cb, const std::function<void(const char*)>& done)
	{
		struct Cxt
		{
			std::function<bool(const void*, size_t)> cb;
			std::function<void(const char*)> done;
		};

		auto cxt = new Cxt();
		cxt->cb = cb;
		cxt->done = done;

		if (!ipfsdlDownloadFile(cxt, url, [](void* cxt, const void* data, size_t size)
		{
			return ((Cxt*)cxt)->cb(data, size);
		}, [](void* cxt, const char* error)
		{
			auto cx = (Cxt*)cxt;
			cx->done(error);

			delete cx;
		}))
		{
			delete cxt;
			return false;
		}

		return true;
	}

	void Poll()
	{
		return ipfsdlPoll();
	}

	auto EnsureInit(const std::function<bool()>& func)
	{
		if (m_inited)
		{
			return func();
		}
		else
		{
			UI_UpdateText(1, gettext(L"Initializing download library...").c_str());

			std::unique_lock _(m_initCbMutex);
			m_initCb = func;

			return true;
		}
	}

	operator bool()
	{
		return hMod && ipfsdlInit && ipfsdlExit && ipfsdlDownloadFile && ipfsdlPoll && success;
	}

private:
	using DownloadCb = bool(*)(void* cxt, const void* data, size_t size);
	using FinishCb = void(*)(void* cxt, const char* error);

	HMODULE hMod;
	void(*ipfsdlInit)(void(*)(bool)) = nullptr;
	bool(*ipfsdlExit)() = nullptr;
	void(*ipfsdlPoll)() = nullptr;
	bool(*ipfsdlDownloadFile)(void* cxt, const char* url, DownloadCb cb, FinishCb done) = nullptr;

	bool success;

	bool m_inited = false;
	std::function<void()> m_initCb;
	std::mutex m_initCbMutex;
};

static IpfsLibrary* GetIpfsLib()
{
	static IpfsLibrary ipfsLib;

	return &ipfsLib;
}

extern void UI_SetSnailState(bool snail);

static bool StartIPFSDownload(download_t* download)
{
	auto& ipfsLib = *GetIpfsLib();

	if (!ipfsLib)
	{
		return false;
	}

	download->doneExternal = false;
	download->successExternal = false;

	UI_SetSnailState(true);

	fallbackPoll = false;

	auto continueDownload = [download, &ipfsLib]()
	{
		UI_UpdateText(1, gettext(L"Starting IPFS discovery...").c_str());

		return ipfsLib.DownloadFile(
		download->url, [download](const void* data, size_t size)
		{
			fallbackPoll = true;

			if (DL_WriteToFile(const_cast<void*>(data), 1, size, download) != size)
			{
				return false;
			}

			return true;
		},
		[download](const char* error)
		{
			download->doneExternal = true;

			if (!error)
			{
				download->successExternal = true;
			}
			else
			{
				strncpy(download->curlError, error, std::size(download->curlError));
				download->curlError[std::size(download->curlError) - 1] = '\0';
			}

			UI_SetSnailState(false);
		});
	};

	return ipfsLib.EnsureInit(continueDownload);
}

static bool PollIPFS()
{
	auto& ipfsLib = *GetIpfsLib();

	if (!ipfsLib)
	{
		return false;
	}

	ipfsLib.Poll();

	return true;
}

#include <shellapi.h>
#include <shobjidl.h>
#include <wrl.h>
namespace WRL = Microsoft::WRL;

static bool ReallyMoveFile(const std::wstring& from, const std::wstring& to)
{
	CoInitialize(NULL);

	WRL::ComPtr<IFileOperation> ifo;
	HRESULT hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOperation, (void**)&ifo);

	if (FAILED(hr))
	{
		return false;
	}

	ifo->SetOperationFlags(FOF_NOCONFIRMATION);
	ifo->SetOwnerWindow(UI_GetWindowHandle());

	WRL::ComPtr<IShellItem> shitem;
	if (FAILED(SHCreateItemFromParsingName(from.c_str(), NULL, IID_IShellItem, (void**)&shitem)))
	{
		return false;
	}

	ifo->RenameItem(shitem.Get(), to.c_str(), NULL);

	hr = ifo->PerformOperations();

	if (FAILED(hr))
	{
		return false;
	}

	BOOL aborted = FALSE;
	ifo->GetAnyOperationsAborted(&aborted);

	if (aborted)
	{
		return false;
	}

	return true;
}

bool DL_ProcessDownload()
{
	if (!dls.currentDownloads.size())
	{
		return true;
	}

	auto initDownload = [](download_t* download)
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

		if (download->progress == 0)
		{
			memset(&download->strm, 0, sizeof(download->strm));
		}

		download->writeToMemory = false;

		// download adhesive.dll to memory to prevent partial write issues
		if (strstr(opath, "adhesive.dll") != nullptr)
		{
			download->writeToMemory = true;
		}

		if (!download->writeToMemory)
		{
			FILE* fp = nullptr;

			fp = _wfopen(ToWide(tmpPath).c_str(), ((download->progress > 0) ? L"ab" : L"wb"));

			if (!fp)
			{
				dls.isDownloading = false;
				UI_DisplayError(va(L"Unable to open %s for writing.", ToWide(opath).c_str()));

				return false;
			}

			download->fp[0] = fp;
		}

		return true;
	};

	auto initCurlDownload = [&initDownload](download_t* download)
	{
		if (!initDownload(download))
		{
			return false;
		}

		curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, va("X-Cfx-Client: 1"));

		auto curlHandle = curl_easy_init_cfx();
		download->curlHandles[0] = curlHandle;

		curl_easy_setopt(curlHandle, CURLOPT_URL, download->url);
		curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, download->curlError);
		curl_easy_setopt(curlHandle, CURLOPT_PRIVATE, download);
		curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, download);
		curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, DL_WriteToFile);
		curl_easy_setopt(curlHandle, CURLOPT_FAILONERROR, true);
		curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, true);

		if (getenv("CFX_CURL_DEBUG"))
		{
			curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1);
			curl_easy_setopt(curlHandle, CURLOPT_DEBUGFUNCTION, DL_CurlDebug);
		}

		if (download->progress)
		{
			curl_easy_setopt(curlHandle, CURLOPT_RANGE, va("%d-", download->progress));
		}

		if (download->progress == 0)
		{
			if (download->algorithm == compressionAlgo_e::XZ)
			{
				lzma_stream_decoder(&download->strm, UINT64_MAX, 0);
				download->strm.avail_out = sizeof(download->strmOut);
				download->strm.next_out = download->strmOut;
			}
			else if (download->algorithm == compressionAlgo_e::Zstd)
			{
				download->zstrm = ZSTD_createDStream();
				download->zout = { download->strmOut, sizeof(download->strmOut), 0 };
			}
		}

		curl_multi_add_handle(dls.curl, curlHandle);

		return true;
	};

	auto onSuccess = [](decltype(dls.currentDownloads.begin()) it)
	{
		auto download = *it;

		std::wstring tmpPathWide = ToWide(download->tmpPath);
		std::wstring opathWide = ToWide(download->opath);

		if (DeleteFile(opathWide.c_str()) == 0)
		{
			if (GetLastError() != ERROR_FILE_NOT_FOUND)
			{
				auto toDeleteName = MakeRelativeCitPath(fmt::sprintf(".updater-remove-%08x-%d", HashString(download->tmpPath.c_str()), GetTickCount64()));

				if (MoveFile(opathWide.c_str(), toDeleteName.c_str()) == 0)
				{
					// let's try asking the shell
					if (!ReallyMoveFile(opathWide, toDeleteName))
					{
						UI_DisplayError(va(L"Moving of %s failed (err = %d). Check your system for any conflicting software.", ToWide(download->file), GetLastError()));
						return false;
					}
				}
			}
		}

		if (MoveFile(tmpPathWide.c_str(), opathWide.c_str()) == 0)
		{
			UI_DisplayError(va(L"Moving of %s failed (err = %d) - make sure you don't have any existing FiveM processes running", ToWide(download->file), GetLastError()));
			DeleteFile(tmpPathWide.c_str());

			return false;
		}

		dls.completedDownloads++;

		dls.currentDownloads.erase(it);

		return true;
	};

	auto processIPFSDownload = [&initDownload, &onSuccess](download_t* download)
	{
		if (!download->curlHandles[0])
		{
			if (!initDownload(download))
			{
				return false;
			}

			download->curlHandles[0] = (CURL*)1;

			if (!StartIPFSDownload(download))
			{
				return false;
			}
		}

		PollIPFS();

		if (download->doneExternal)
		{
			if (download->successExternal)
			{
				auto it = std::find_if(dls.currentDownloads.begin(), dls.currentDownloads.end(), [&](auto& d)
				{
					return (d.get() == download);
				});

				if (download->fp[0])
				{
					fclose(download->fp[0]);
				}

				if (!onSuccess(it))
				{
					return false;
				}
			}
			else
			{
				std::wstring tmpPathWide = ToWide(download->tmpPath);
				_wunlink(tmpPathWide.c_str());

				UI_DisplayError(va(L"Downloading of %s failed - %s", ToWide(download->url), ToWide(download->curlError)));

				return false;
			}
		}

		return true;
	};

	// create new handles if we don't have one yet
	for (auto& download : dls.currentDownloads)
	{
		if (strncmp(download->url, "ipfs://", 7) == 0)
		{
			return processIPFSDownload(download.get());
		}

		if (!download->curlHandles[0])
		{
			initCurlDownload(download.get());
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

				std::wstring tmpPathWide = ToWide(download->tmpPath);

				if (!download->writeToMemory)
				{
					if (download->fp[0])
					{
						fclose(download->fp[0]);
						download->fp[0] = nullptr;
					}
				}
				else
				{
					HANDLE hFile = CreateFileW(tmpPathWide.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

					if (hFile == INVALID_HANDLE_VALUE)
					{
						DWORD errNo = GetLastError();

						dls.isDownloading = false;
						UI_DisplayError(va(L"Unable to open %s for writing. Windows error code %d was returned.", tmpPathWide, errNo));

						return false;
					}

					std::string str = download->memoryStream.str();

					DWORD bytesWritten = 0;
					BOOL success = WriteFile(hFile, str.c_str(), DWORD(str.size()), &bytesWritten, nullptr);

					if (!success || bytesWritten != str.size())
					{
						DWORD errNo = GetLastError();

						UI_DisplayError(
							va(
								L"Unable to write to %s. Windows error code %d was returned.%s",
								tmpPathWide,
								errNo,
								(errNo == ERROR_VIRUS_INFECTED) ? L"\nThis is usually caused by anti-malware software. Please report this issue to your anti-malware software vendor." : L""
							));

						return false;
					}

					CloseHandle(hFile);
				}

				curl_multi_remove_handle(dls.curl, handle);
				curl_easy_cleanup(handle);

				if (code == CURLE_OK)
				{
					if (download->algorithm == compressionAlgo_e::XZ)
					{
						lzma_end(&download->strm);
					}
					else if (download->algorithm == compressionAlgo_e::Zstd)
					{
						ZSTD_freeDStream(download->zstrm);
					}

					if (!onSuccess(it))
					{
						return false;
					}
				}
				else
				{
					bool shouldRetry = false;

					if (code != CURLE_HTTP_RETURNED_ERROR)
					{
						if (download->numRetries > 0)
						{
							download->numRetries--;

							shouldRetry = true;
						}
					}

					if (!shouldRetry)
					{
						lzma_end(&download->strm);

						_wunlink(tmpPathWide.c_str());
						UI_DisplayError(va(L"Downloading %s failed with CURLcode %d - %s%s",
							ToWide(std::string{ GetBaseName(download->url) }),
							(int)code,
							ToWide(download->curlError),
							(code == CURLE_WRITE_ERROR) ? L"\nAre you sure you have enough disk space on all drives?" : L""));

						return false;
					}

					download->curlHandles[0] = NULL;
					initCurlDownload(download.get());
				}
			}
		}

	} while (info != NULL);

	return true;
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

	if (fallbackPoll)
	{
		uint64_t now = GetTickCount64();

		if ((now - dls.lastPoll) > 250)
		{
			DL_UpdateGlobalProgress(0, now);

			dls.lastPoll = now;
		}
	}

	if (!dls.currentDownloads.empty())
	{
		if (!DL_ProcessDownload())
		{
			dls.isDownloading = false;
			dls.error = true;

			return true;
		}
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

static char g_curlError[CURL_ERROR_SIZE];

const char* DL_RequestURLError()
{
	return g_curlError;
}

static struct CurlInit
{
	CurlInit()
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}
} curlInit;

static size_t CurlHeaderInfo(char* buffer, size_t size, size_t nitems, void* userdata)
{
	auto cdPtr = reinterpret_cast<HttpHeaderList*>(userdata);

	if (cdPtr)
	{
		std::string str(buffer, size * nitems);

		// reset HTTP headers if we followed a Location and got a new HTTP response
		if (str.find("HTTP/") == 0)
		{
			cdPtr->clear();
		}

		auto colonPos = str.find(": ");

		if (colonPos != std::string::npos)
		{
			cdPtr->emplace(str.substr(0, colonPos), str.substr(colonPos + 2, str.length() - 2 - colonPos - 2));
		}
	}

	return size * nitems;
}

int DL_RequestURL(const char* url, char* buffer, size_t bufSize, HttpHeaderListPtr responseHeaders)
{
	CURL* curl = curl_easy_init_cfx();

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
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, g_curlError);

		if (getenv("CFX_CURL_DEBUG"))
		{
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
			curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, DL_CurlDebug);
		}

		if (responseHeaders)
		{
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlHeaderInfo);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, responseHeaders.get());
		}

		CURLcode code = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

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
			DL_Shutdown();
			return false;
		}
	}

	if (dls.error)
	{
		return false;
	}

	return true;
}

void StartIPFS()
{
	GetIpfsLib();
}
#endif
