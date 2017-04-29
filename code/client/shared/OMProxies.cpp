#include <StdInc.h>

#define FXOM_NO_HELPERS
#include <../citicore/om/core.h> // HACKHACK

extern "C" intptr_t fxFindFirstImpl(const guid_t& iid, guid_t* clsid)
{
    static intptr_t(*func)(const guid_t&, guid_t*);

    if (!func)
    {
        func = (decltype(func))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreFxFindFirstImpl");
    }

    return func(iid, clsid);
}

extern "C" int32_t fxFindNextImpl(intptr_t findHandle, guid_t* clsid)
{
    static int32_t(*func)(intptr_t, guid_t*);

    if (!func)
    {
        func = (decltype(func))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreFxFindNextImpl");
    }

    return func(findHandle, clsid);
}

extern "C" void fxFindImplClose(intptr_t findHandle)
{
    static void(*func)(intptr_t);

    if (!func)
    {
        func = (decltype(func))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreFxFindImplClose");
    }

    return func(findHandle);
}

extern "C" result_t fxCreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef)
{
    static result_t(*func)(const guid_t&, const guid_t&, void**);

    if (!func)
    {
        func = (decltype(func))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreFxCreateObjectInstance");
    }

    return func(guid, iid, objectRef);
}