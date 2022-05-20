#include "StdInc.h"

#include <nvapi.h>
#include <NvApiDriverSettings.h>

void DisableNvCache()
{
	if (NvAPI_Initialize() != NVAPI_OK)
	{
		return;
	}

	NvAPI_Status status = NVAPI_OK;
	NvDRSSessionHandle session = { 0 };

	status = NvAPI_DRS_CreateSession(&session);
	if (status != NVAPI_OK)
	{
		return;
	}

	status = NvAPI_DRS_LoadSettings(session);
	if (status != NVAPI_OK)
	{
		return;
	}

	// try finding a profile, if existent
	wchar_t appName[1024] = { 0 };
	GetModuleFileNameW(GetModuleHandleW(NULL), appName, std::size(appName));

	for (wchar_t& ch : appName)
	{
		if (ch == '\\')
		{
			ch = '/';
		}
	}

	uint32_t appHash = HashString(ToNarrow(appName).c_str());

	NVDRS_APPLICATION application = {};
	application.version = NVDRS_APPLICATION_VER_V4;

	NvAPI_UnicodeString name = { 0 };
	memcpy(&name, appName, std::min(sizeof(appName), NVAPI_UNICODE_STRING_MAX * sizeof(NvU16)));

	NvDRSProfileHandle profile = { 0 };
	status = NvAPI_DRS_FindApplicationByName(session, name, &profile, &application);

	if (status != NVAPI_OK)
	{
		// make a new profile, okay
		NVDRS_PROFILE profileInfo = {0};
		profileInfo.version = NVDRS_PROFILE_VER;
		profileInfo.isPredefined = 0;
		wcscpy((wchar_t*)profileInfo.profileName, va(L"Cfx.re: 0x%08x", appHash));

		status = NvAPI_DRS_CreateProfile(session, &profileInfo, &profile);

		if (status != NVAPI_OK)
		{
			return;
		}

		application.version = NVDRS_APPLICATION_VER_V4;
		application.isPredefined = 0;
		memcpy(application.appName, name, sizeof(application.appName));
		wcscpy((wchar_t*)application.userFriendlyName, va(L"Cfx.re EXE: 0x%08x", appHash));

		status = NvAPI_DRS_CreateApplication(session, profile, &application);

		if (status != NVAPI_OK)
		{
			return;
		}
	}

	NVDRS_SETTING setting = { 0 };
	setting.version = NVDRS_SETTING_VER;
	setting.settingId = PS_SHADERDISKCACHE_ID;
	setting.settingType = NVDRS_DWORD_TYPE;
	setting.settingLocation = NVDRS_CURRENT_PROFILE_LOCATION;
	setting.isCurrentPredefined = 0;
	setting.isPredefinedValid = 0;
	setting.u32CurrentValue = PS_SHADERDISKCACHE_OFF;
	setting.u32PredefinedValue = PS_SHADERDISKCACHE_OFF;

	status = NvAPI_DRS_SetSetting(session, profile, &setting);
	if (status != NVAPI_OK)
	{
		return;
	}

	NvAPI_DRS_SaveSettings(session);
}
