#include "StdInc.h"

#include <nvapi.h>
#include <NvApiDriverSettings.h>

#include <strsafe.h>

static auto StatusToString(NvAPI_Status status) -> std::string
{
	NvAPI_ShortString s;
	if (NvAPI_GetErrorMessage(status, s) == NVAPI_OK)
	{
		return s;
	}

	return "";
}

struct SessionDtor
{
	explicit SessionDtor(NvDRSSessionHandle session)
		: session(session)
	{

	}

	~SessionDtor()
	{
		if (session)
		{
			NvAPI_DRS_DestroySession(session);
			session = nullptr;
		}
	}

private:
	NvDRSSessionHandle session;
};

struct MutexDtor
{
	explicit MutexDtor(HANDLE handle)
		: handle(handle)
	{
	}

	~MutexDtor()
	{
		if (handle)
		{
			ReleaseMutex(handle);
			CloseHandle(handle);
			handle = NULL;
		}
	}

private:
	HANDLE handle;
};

void DisableNvCache()
{
	if (NvAPI_Initialize() != NVAPI_OK)
	{
		return;
	}

	HANDLE hMutex = CreateMutexW(NULL, FALSE, L"CfxNvCacheMutex");
	WaitForSingleObject(hMutex, INFINITE);

	MutexDtor mutexDtor(hMutex);

	NvAPI_Status status = NVAPI_OK;
	NvDRSSessionHandle session = { 0 };

	status = NvAPI_DRS_CreateSession(&session);
	if (status != NVAPI_OK)
	{
		trace("%s: NvAPI_DRS_CreateSession returned %s\n", __func__, StatusToString(status));
		return;
	}

	SessionDtor dtor(session);

	status = NvAPI_DRS_LoadSettings(session);
	if (status != NVAPI_OK)
	{
		trace("%s: NvAPI_DRS_LoadSettings returned %s\n", __func__, StatusToString(status));
		return;
	}

	// try finding a profile, if existent
	wchar_t appName1[1024] = { 0 };
	GetModuleFileNameW(GetModuleHandleW(NULL), appName1, std::size(appName1));

	wchar_t appName[1024] = { 0 };
	GetFullPathNameW(appName1, std::size(appName), appName, NULL);

	for (wchar_t& ch : appName)
	{
		if (ch == '\\')
		{
			ch = '/';
		}
	}

	NVDRS_APPLICATION application = {};
	application.version = NVDRS_APPLICATION_VER_V4;

	NvAPI_UnicodeString name = { 0 };
	memcpy(&name, appName, std::min(sizeof(appName), NVAPI_UNICODE_STRING_MAX * sizeof(NvU16)));

	NvDRSProfileHandle profile = { 0 };
	status = NvAPI_DRS_FindApplicationByName(session, name, &profile, &application);

	if (status == NVAPI_OK)
	{
		// delete any old-named profile
		if (wcsstr((wchar_t*)application.userFriendlyName, L"Cfx.re EXE: 0x"))
		{
			NvAPI_DRS_DeleteProfile(session, profile);
			status = NVAPI_ERROR;
		}
	}

	if (status != NVAPI_OK)
	{
		// make a new profile, okay
		NVDRS_PROFILE profileInfo = {0};
		profileInfo.version = NVDRS_PROFILE_VER;
		profileInfo.isPredefined = 0;
		StringCchCopyW((wchar_t*)profileInfo.profileName, NVAPI_UNICODE_STRING_MAX, va(L"Cfx.re: %s", appName));

		status = NvAPI_DRS_CreateProfile(session, &profileInfo, &profile);

		if (status != NVAPI_OK)
		{
			trace("%s: NvAPI_DRS_CreateProfile returned %s\n", __func__, StatusToString(status));
			return;
		}

		application.version = NVDRS_APPLICATION_VER_V4;
		application.isPredefined = 0;
		memcpy(application.appName, name, sizeof(application.appName));
		StringCchCopyW((wchar_t*)application.userFriendlyName, NVAPI_UNICODE_STRING_MAX, va(L"Cfx.re EXE: %s", appName));

		status = NvAPI_DRS_CreateApplication(session, profile, &application);

		if (status != NVAPI_OK)
		{
			trace("%s: NvAPI_DRS_CreateApplication returned %s\n", __func__, StatusToString(status));
			return;
		}
	}

	auto settings = {
		// important (nvidia drivers have a race condition that leads to these files corrupting): disable shader disk cache
		std::make_tuple(PS_SHADERDISKCACHE_ID, (DWORD)PS_SHADERDISKCACHE_OFF), // 'Enables/Disables strategy' -> Off

		// mobile GPUs that use profile overrides need this manually replaced
		std::make_tuple(SHIM_MCCOMPAT_ID, (DWORD)SHIM_MCCOMPAT_ENABLE), // 'White list for shim layer' -> 'Run on DGPU'
		std::make_tuple(SHIM_RENDERING_MODE_ID, (DWORD)SHIM_RENDERING_MODE_ENABLE), // 'White list for shim layer per application' -> 'Run on DGPU'
		std::make_tuple(SHIM_RENDERING_OPTIONS_ID, (DWORD)SHIM_RENDERING_OPTIONS_DEFAULT_RENDERING_MODE), // 'Rendering Mode Options for shim layer per application' -> 'No overrides for Shim rendering methods'
	};

	for (const auto& [id, value] : settings)
	{
		NVDRS_SETTING setting = { 0 };
		setting.version = NVDRS_SETTING_VER;
		setting.settingId = id;
		setting.settingType = NVDRS_DWORD_TYPE;
		setting.settingLocation = NVDRS_CURRENT_PROFILE_LOCATION;
		setting.isCurrentPredefined = 0;
		setting.isPredefinedValid = 0;
		setting.u32CurrentValue = value;
		setting.u32PredefinedValue = value;

		status = NvAPI_DRS_SetSetting(session, profile, &setting);
		if (status != NVAPI_OK)
		{
			trace("%s: NvAPI_DRS_SetSetting returned %s\n", __func__, StatusToString(status));
			continue;
		}
	}

	status = NvAPI_DRS_SaveSettings(session);

	if (status != NVAPI_OK)
	{
		trace("%s: NvAPI_DRS_SaveSettings returned %s\n", __func__, StatusToString(status));
	}
}
