/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "NUISchemeHandlerFactory.h"
#include "NUIWindowManager.h"

#include <DrawCommands.h>
#include <fiDevice.h>

#include <delayimp.h>

#include <include/cef_origin_whitelist.h>

#include "ResumeComponent.h"
#include "HookFunction.h"

#include <CfxSubProcess.h>

#include <Error.h>

#include "memdbgon.h"

void FinalizeInitNUI();

void Component_RunPreInit()
{
#ifdef _M_AMD64
	// again, a Win7 SP1 check (Chromium x64 isn't supported below this operating level)
	if (!IsWindows7SP1OrGreater())
	{
		FatalError("CitizenFX requires Windows 7 SP1 or higher. Please upgrade to this operating system version to run CitizenFX.");
	}
#endif

	// CEF keeps loading/unloading this - load it ourselves to make the refcount always 1
	LoadLibrary(L"bluetoothapis.dll");

	// load the CEF library
	HMODULE libcef = LoadLibraryW(MakeRelativeCitPath(L"bin/libcef.dll").c_str());

	if (!libcef)
	{
		MessageBoxW(NULL, L"Could not load bin/libcef.dll.", L"CitizenFX", MB_ICONSTOP | MB_OK);

		ExitProcess(0);
	}

	__HrLoadAllImportsForDll("libcef.dll");

	Instance<NUIApp>::Set(new NUIApp());

	// instantiate a NUIApp
	auto selfApp = Instance<NUIApp>::Get();

	CefMainArgs args(GetModuleHandle(NULL));
	static CefRefPtr<CefApp> app(selfApp);

    auto schemeHandlerFactory = new NUISchemeHandlerFactory();
    schemeHandlerFactory->AddRef();
    Instance<NUISchemeHandlerFactory>::Set(schemeHandlerFactory);

    InitFunctionBase::RunAll();

    OnResumeGame.Connect([] ()
    {
        FinalizeInitNUI();
    });

	// try to execute as a CEF process
	int exitCode = CefExecuteProcess(args, app, nullptr);

	// and exit if we did
	if (exitCode >= 0)
	{
		ExitProcess(0);
	}
}

#include <fiCustomDevice.h>

class ObfuscatedDevice : public rage::fiCustomDevice
{
private:
	rage::fiDevice* m_device;
	std::string m_fileName;

public:
	ObfuscatedDevice(rage::fiDevice* parent, const std::string& fileName)
		: m_device(parent), m_fileName(fileName)
	{
	}

	virtual uint64_t Open(const char* fileName, bool readOnly) override
	{
		return m_device->Open(m_fileName.c_str(), readOnly);
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) override
	{
		return m_device->OpenBulk(m_fileName.c_str(), ptr);
	}

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*) override
	{
		return OpenBulk(fileName, ptr);
	}

	virtual uint64_t Create(const char* fileName) override
	{
		return -1;
	}

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) override
	{
		return m_device->Read(handle, buffer, toRead);
	}

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) override
	{
		return m_device->ReadBulk(handle, ptr, buffer, toRead);
	}

	virtual uint32_t Write(uint64_t, void*, int) override
	{
		return -1;
	}

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) override
	{
		return m_device->Seek(handle, distance, method);
	}

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) override
	{
		return m_device->SeekLong(handle, distance, method);
	}

	virtual int32_t Close(uint64_t handle) override
	{
		return m_device->Close(handle);
	}

	virtual int32_t CloseBulk(uint64_t handle) override
	{
		return m_device->CloseBulk(handle);
	}

	virtual int GetFileLength(uint64_t handle) override
	{
		return m_device->GetFileLength(handle);
	}

	virtual uint64_t GetFileLengthLong(const char* fileName) override
	{
		return m_device->GetFileLengthLong(m_fileName.c_str());
	}

	virtual uint64_t GetFileLengthUInt64(uint64_t handle) override
	{
		return m_device->GetFileLengthUInt64(handle);
	}

	virtual bool RemoveFile(const char* file) override
	{
		return false;
	}

	virtual int RenameFile(const char* from, const char* to) override
	{
		return false;
	}

	virtual int CreateDirectory(const char* dir) override
	{
		return false;
	}

	virtual int RemoveDirectory(const char* dir) override
	{
		return false;
	}

	virtual uint32_t GetFileTime(const char* file) override
	{
		return m_device->GetFileTime(m_fileName.c_str());
	}

	virtual bool SetFileTime(const char* file, FILETIME fileTime) override
	{
		return false;
	}

	virtual uint32_t GetFileAttributes(const char* path) override
	{
		return m_device->GetFileAttributes(m_fileName.c_str());
	}

	virtual int m_yx() override
	{
		return m_device->m_yx();
	}

	virtual bool IsBulkDevice() override
	{
		return m_device->IsBulkDevice();
	}

	virtual const char* GetName() override
	{
		return "RageVFSDeviceAdapter";
	}
};

void FinalizeInitNUI()
{
    if (getenv("CitizenFX_ToolMode"))
    {
        return;
    }

	std::wstring cachePath = MakeRelativeCitPath(L"cache\\browser\\");
	CreateDirectory(cachePath.c_str(), nullptr);

	// delete any old CEF logs
	DeleteFile(MakeRelativeCitPath(L"debug.log").c_str());

	auto selfApp = Instance<NUIApp>::Get();

	CefMainArgs args(GetModuleHandle(NULL));
	CefRefPtr<CefApp> app(selfApp);

	CefSettings cSettings;
		
	cSettings.multi_threaded_message_loop = true;
	cSettings.remote_debugging_port = 13172;
	cSettings.pack_loading_disabled = false; // true;
	cSettings.windowless_rendering_enabled = false; // true;
	cSettings.log_severity = LOGSEVERITY_DISABLE;
	
	CefString(&cSettings.browser_subprocess_path).FromWString(MakeCfxSubProcess(L"ChromeBrowser"));

	CefString(&cSettings.locale).FromASCII("en-US");

	std::wstring resPath = MakeRelativeCitPath(L"bin/cef/");

	CefString(&cSettings.resources_dir_path).FromWString(resPath);
	CefString(&cSettings.locales_dir_path).FromWString(resPath);
	CefString(&cSettings.cache_path).FromWString(cachePath);

	// 2014-06-30: sandbox disabled as it breaks scheme handler factories (results in blank page being loaded)
	CefInitialize(args, cSettings, app.get(), /*cefSandbox*/ nullptr);
	CefRegisterSchemeHandlerFactory("nui", "", Instance<NUISchemeHandlerFactory>::Get());
	CefAddCrossOriginWhitelistEntry("nui://game", "http", "", true);
	CefAddCrossOriginWhitelistEntry("nui://game", "nui", "", true);

    HookFunctionBase::RunAll();

#if defined(GTA_NY)
	OnGrcBeginScene.Connect([] ()
	{
		Instance<NUIWindowManager>::Get()->ForAllWindows([] (fwRefContainer<NUIWindow> window)
		{
			window->UpdateFrame();
		});
	});
#else

#endif

	//g_hooksDLL->SetHookCallback(StringHash("d3dCreate"), [] (void*)
	OnGrcCreateDevice.Connect([]()
	{
		//int resX = *(int*)0xFDCEAC;
		//int resY = *(int*)0xFDCEB0;
		int resX, resY;
		GetGameResolution(resX, resY);

		auto rootWindow = NUIWindow::Create(true, resX, resY, "nui://game/ui/root.html");
		rootWindow->SetPaintType(NUIPaintTypePostRender);

		Instance<NUIWindowManager>::Get()->SetRootWindow(rootWindow);
	});

	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		std::wstring uiPack = MakeRelativeCitPath(L"citizen/ui.rpf");

		if (GetFileAttributes(uiPack.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			std::string uiPath = converter.to_bytes(uiPack);

			ObfuscatedDevice* obfuscatedDevice = new ObfuscatedDevice(rage::fiDevice::GetDevice(uiPath.c_str(), true), uiPath);
			rage::fiDevice::MountGlobal("obf:/", obfuscatedDevice, true);

			rage::fiPackfile* packFile = new rage::fiPackfile();
			packFile->OpenPackfile("obf:/fi.rpf", true, false, 0);
			packFile->Mount("citizen:/ui/");
		}
	}, 100);

	return;
}