#include <StdInc.h>

#define FXOM_NO_HELPERS
#include <../citicore/om/core.h> // HACKHACK

extern "C" intptr_t CoreFxFindFirstImpl(const guid_t& iid, guid_t* clsid);
extern "C" int32_t CoreFxFindNextImpl(intptr_t findHandle, guid_t* clsid);
extern "C" void CoreFxFindImplClose(intptr_t findHandle);
extern "C" result_t CoreFxCreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef);

extern "C" intptr_t fxFindFirstImpl(const guid_t& iid, guid_t* clsid)
{
	return CoreFxFindFirstImpl(iid, clsid);
}

extern "C" int32_t fxFindNextImpl(intptr_t findHandle, guid_t* clsid)
{
	return CoreFxFindNextImpl(findHandle, clsid);
}

extern "C" void fxFindImplClose(intptr_t findHandle)
{
	return CoreFxFindImplClose(findHandle);
}

extern "C" result_t fxCreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef)
{
	return CoreFxCreateObjectInstance(guid, iid, objectRef);
}
