#include <StdInc.h>
#include <minhook.h>

static LONG(WINAPI* g_origRegOpenKeyExA)(HKEY key, const char* subKey, DWORD options, REGSAM samDesired, PHKEY outKey);

LONG WINAPI ProcessLSPRegOpenKeyExA(HKEY key, const char* subKey, DWORD options, REGSAM samDesired, PHKEY outKey)
{
    static thread_local HKEY lastLSPKey = (HKEY)-1;

    if (subKey)
    {
        if (!_stricmp(subKey, "AppId_Catalog"))
        {
            auto setValue = [&](const wchar_t* name, const wchar_t* keyString)
            {
                RegSetKeyValue(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\AppID_Catalog", name, REG_SZ, keyString, (wcslen(keyString) * 2) + 2);
            };

            wchar_t modulePath[512];
            GetModuleFileName(GetModuleHandle(nullptr), modulePath, sizeof(modulePath) / sizeof(wchar_t));

            setValue(L"AppFullPath", modulePath);
            
            DWORD permittedCategories = 0x80000000;
            RegSetKeyValue(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\AppID_Catalog", L"PermittedLspCategories", REG_DWORD, &permittedCategories, sizeof(permittedCategories));

            LONG status = g_origRegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\CitizenFX\\AppID_Catalog", options, samDesired, outKey);
            lastLSPKey = *outKey;

            return status;
        }
    }

    if (key == lastLSPKey)
    {
        if (!strchr(subKey, L'-'))
        {
            LONG status = g_origRegOpenKeyExA(key, "", options, samDesired, outKey);

            lastLSPKey = (HKEY)-1;

            return status;
        }
    }

    return g_origRegOpenKeyExA(key, subKey, options, samDesired, outKey);
}

void LSP_InitializeHooks()
{
    MH_CreateHookApi(L"kernelbase.dll", "RegOpenKeyExA", ProcessLSPRegOpenKeyExA, (void**)&g_origRegOpenKeyExA);
    MH_EnableHook(MH_ALL_HOOKS);
}