#include "StdInc.h"
#include "Live.h"

// #5349: XLiveProtectedVerifyFile
extern "C" DWORD __stdcall XLiveProtectedVerifyFile(HANDLE hContentAccess, VOID * pvReserved, PCWSTR pszFilePath)
{
	return 0;
}

// #5350: XLiveContentCreateAccessHandle
extern "C" DWORD __stdcall XLiveContentCreateAccessHandle(DWORD dwTitleId, void * pContentInfo,
														  DWORD dwLicenseInfoVersion, void * xebBuffer, DWORD dwOffset, HANDLE * phAccess, void * pOverlapped)
{
	if (phAccess)
		*phAccess = INVALID_HANDLE_VALUE;
	return E_OUTOFMEMORY;
}

// #5352: XLiveContentUninstall
extern "C" DWORD __stdcall XLiveContentUninstall(void * pContentInfo, void * pxuidFor, void * pInstallCallbackParams)
{
	return 0;
}

// #5355: XLiveContentGetPath
extern "C" DWORD __stdcall XLiveContentGetPath(DWORD dwUserIndex, void * pContentInfo, wchar_t * pszPath, DWORD * pcchPath)
{
	if (pcchPath)
		*pcchPath = 0;
	if (pszPath)
		*pszPath = 0;
	return 0;
}

// #5360: XLiveContentCreateEnumerator
extern "C" DWORD __stdcall XLiveContentCreateEnumerator(DWORD, void *, DWORD *pchBuffer, HANDLE * phContent)
{
	if (phContent)
		*phContent = INVALID_HANDLE_VALUE;
	return 0;
}

// #5361: XLiveContentRetrieveOffersByDate
extern "C" DWORD __stdcall XLiveContentRetrieveOffersByDate(DWORD dwUserIndex, DWORD dwOffserInfoVersion,
															SYSTEMTIME * pstStartDate, void * pOffserInfoArray, DWORD * pcOfferInfo, void * pOverlapped)
{
	if (pcOfferInfo)
		*pcOfferInfo = 0;
	return 0;
}

// #5365: XShowMarketplaceUI
extern "C" DWORD __stdcall XShowMarketplaceUI(DWORD dwUserIndex, DWORD dwEntryPoint, ULONGLONG dwOfferId, DWORD dwContentCategories)
{
	return 1;
}

// === replacements ===
struct FakeProtectedBuffer
{
	DWORD	dwMagic;
	DWORD	dwSize;
	BYTE	bData[4];
};

// #5016: XLivePBufferAllocate
DWORD __stdcall XLivePBufferAllocate(int size, FakeProtectedBuffer ** pBuffer)
{
	//	trace ("xlive_5016: XLivePBufferAllocate (%d)\n", size);
	*pBuffer = (FakeProtectedBuffer *)malloc(size + 8);
	if (!*pBuffer)
	{
		return E_OUTOFMEMORY;
	}

	(*pBuffer)->dwMagic = 0xDEADDEAD;	// some arbitrary number
	(*pBuffer)->dwSize = size;
	return 0;
}

// #5017: XLivePBufferFree
DWORD __stdcall XLivePBufferFree(FakeProtectedBuffer * pBuffer)
{
	// trace ("xlive_5017: XLivePBufferFree\n");
	if (pBuffer && pBuffer->dwMagic == 0xDEADDEAD)
		free(pBuffer);
	return 0;
}

// #5295: XLivePBufferSetByteArray
DWORD __stdcall XLivePBufferSetByteArray(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * source, DWORD size)
{
	if (!pBuffer || pBuffer->dwMagic != 0xDEADDEAD || !source || offset < 0 || offset + size > pBuffer->dwSize)
		return 0;
	memcpy(pBuffer->bData + offset, source, size);
	return 0;
}

// #5294: XLivePBufferGetByteArray
DWORD __stdcall XLivePBufferGetByteArray(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * destination, DWORD size)
{
	if (!pBuffer || pBuffer->dwMagic != 0xDEADDEAD || !destination || offset < 0 || offset + size > pBuffer->dwSize)
		return 0;
	memcpy(destination, pBuffer->bData + offset, size);
	return 0;
}

// #5019: XLivePBufferSetByte
DWORD __stdcall XLivePBufferSetByte(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE value)
{
	if (!pBuffer || pBuffer->dwMagic != 0xDEADDEAD || offset < 0 || offset > pBuffer->dwSize)
		return 0;
	pBuffer->bData[offset] = value;
	return 0;
}

// #5018: XLivePBufferGetByte
DWORD __stdcall XLivePBufferGetByte(FakeProtectedBuffer * pBuffer, DWORD offset, BYTE * value)
{
	if (!pBuffer || pBuffer->dwMagic != 0xDEADDEAD || !value || offset < 0 || offset > pBuffer->dwSize)
		return 0;
	*value = pBuffer->bData[offset];
	return 0;
}

// #5020: XLivePBufferGetDWORD
extern "C" DWORD __stdcall XLivePBufferGetDWORD(FakeProtectedBuffer * pBuffer, DWORD dwOffset, DWORD * pdwValue)
{
	if (!pBuffer || pBuffer->dwMagic != 0xDEADDEAD || dwOffset < 0 || dwOffset > pBuffer->dwSize - 4 || !pdwValue)
		return 0;
	*pdwValue = *(DWORD *)(pBuffer->bData + dwOffset);
	return 0;
}

// #5021: XLivePBufferSetDWORD
extern "C" DWORD __stdcall XLivePBufferSetDWORD(FakeProtectedBuffer * pBuffer, DWORD dwOffset, DWORD dwValue)
{
	if (!pBuffer || pBuffer->dwMagic != 0xDEADDEAD || dwOffset < 0 || dwOffset > pBuffer->dwSize - 4)
		return 0;
	*(DWORD *)(pBuffer->bData + dwOffset) = dwValue;
	return 0;
}

// #5026: XLiveSetSponsorToken
extern "C" DWORD __stdcall XLiveSetSponsorToken(LPCWSTR pwszToken, DWORD dwTitleId)
{
	return S_OK;
}


// #5036: XLiveCreateProtectedDataContext
DWORD __stdcall XLiveCreateProtectedDataContext(DWORD * dwType, PHANDLE pHandle)
{
	if (pHandle)
		*pHandle = (HANDLE)1;
	return 0;
}

// #5037: XLiveQueryProtectedDataInformation
DWORD __stdcall XLiveQueryProtectedDataInformation(HANDLE h, DWORD * p)
{
	return 0;
}

// #5038: XLiveCloseProtectedDataContext
DWORD __stdcall XLiveCloseProtectedDataContext(HANDLE h)
{
	return 0;
}

// #5035: XLiveUnprotectData
DWORD __stdcall XLiveUnprotectData(BYTE * pInBuffer, DWORD dwInDataSize, BYTE * pOutBuffer, DWORD * pDataSize, HANDLE * ph)
{
	if (!pDataSize || !ph)
		return E_FAIL;

	*ph = (HANDLE)1;

	if (!pOutBuffer || *pDataSize < dwInDataSize)
	{
		*pDataSize = dwInDataSize;
		return ERROR_INSUFFICIENT_BUFFER;
	}
	*pDataSize = dwInDataSize;
	memcpy(pOutBuffer, pInBuffer, dwInDataSize);
	return 0;
}

// #5034: XLiveProtectData
DWORD __stdcall XLiveProtectData(BYTE * pInBuffer, DWORD dwInDataSize, BYTE * pOutBuffer, DWORD * pDataSize, HANDLE h)
{
	*pDataSize = dwInDataSize;
	if (*pDataSize >= dwInDataSize && pOutBuffer)
		memcpy(pOutBuffer, pInBuffer, dwInDataSize);
	return 0;
}

// #5367
extern "C" DWORD __stdcall xlive_5367(HANDLE, DWORD, DWORD, XOVERLAPPED * pOverlapped, DWORD)
{
	pOverlapped->InternalLow = ERROR_SUCCESS;
	pOverlapped->InternalHigh = 0;

	if (pOverlapped->hEvent)
	{
		SetEvent(pOverlapped->hEvent);
	}

	return ERROR_IO_PENDING;
}

// #5372
extern "C" DWORD __stdcall xlive_5372(HANDLE, DWORD, DWORD, DWORD, BYTE *, HANDLE)
{
	return 0;
}

// #5000: XLiveInitialize
int __stdcall XLiveInitialize(DWORD)
{
	return 0;
}

// #5001: XLiveInput
int __stdcall XLiveInput(DWORD * p)
{
	p[5] = 0;
	return 1;	// -1 ?
}


// #5002: XLiveRender
int __stdcall XLiveRender()
{
	return 0;
}

// #5003: XLiveUninitialize
int __stdcall XLiveUninitialize()
{
	return 0;
}

int __stdcall XLiveOnCreateDevice(DWORD, DWORD)
{
	return 0;
}

// #5007: XLiveOnResetDevice
int __stdcall XLiveOnResetDevice(DWORD)
{
	return 0;
}

// #5008: XHVCreateEngine
int __stdcall XHVCreateEngine(DWORD, DWORD, void ** ppEngine)
{
	if (ppEngine)
		*ppEngine = NULL;
	//*ppEngine = &fakeVoiceEngine;
	return -1;	// disable live voice   
}

// #5022: XLiveGetUpdateInformation
int __stdcall XLiveGetUpdateInformation(DWORD)
{
	return -1; // no update
}

// #5024: XLiveUpdateSystem
int __stdcall XLiveUpdateSystem(DWORD)
{
	return -1; // no update
}

// #5030: XLivePreTranslateMessage
int __stdcall XLivePreTranslateMessage(DWORD)
{
	return 0;
}

// #5031 XLiveSetDebugLevel
int __stdcall XLiveSetDebugLevel(DWORD xdlLevel, DWORD * pxdlOldLevel)
{
	return 0;
}

int __stdcall XLiveInitializeEx(void * pXii, DWORD dwVersion)
{
	return S_OK;
}

static HookFunction hookFunction([] ()
{
	hook::iat("xlive.dll", XLiveInitialize, 5000);
	hook::iat("xlive.dll", XLiveInitializeEx, 5297);
	hook::iat("xlive.dll", XLiveInput, 5001);
	hook::iat("xlive.dll", XLiveRender, 5002);
	hook::iat("xlive.dll", XLiveUninitialize, 5003);
	hook::iat("xlive.dll", XLiveOnCreateDevice, 5005);
	hook::iat("xlive.dll", XLiveOnResetDevice, 5007);
	hook::iat("xlive.dll", XHVCreateEngine, 5008);
	hook::iat("xlive.dll", XLiveGetUpdateInformation, 5022);
	hook::iat("xlive.dll", XLiveUpdateSystem, 5024);
	hook::iat("xlive.dll", XLivePreTranslateMessage, 5030);
	hook::iat("xlive.dll", XLiveSetDebugLevel, 5031);
	hook::iat("xlive.dll", XLiveContentCreateAccessHandle, 5350);
	hook::iat("xlive.dll", XLiveContentUninstall, 5352);
	hook::iat("xlive.dll", XLiveContentGetPath, 5355);
	hook::iat("xlive.dll", XLiveContentCreateEnumerator, 5360);
	hook::iat("xlive.dll", XLiveContentRetrieveOffersByDate, 5361);
	hook::iat("xlive.dll", XLiveProtectedVerifyFile, 5349);
	hook::iat("xlive.dll", XShowMarketplaceUI, 5365);
	hook::iat("xlive.dll", XLiveSetSponsorToken, 5026);
	hook::iat("xlive.dll", XLivePBufferGetByteArray, 5294);
	hook::iat("xlive.dll", XLivePBufferSetByteArray, 5295);
	hook::iat("xlive.dll", XLivePBufferAllocate, 5016);
	hook::iat("xlive.dll", XLivePBufferFree, 5017);
	hook::iat("xlive.dll", XLivePBufferGetByte, 5018);
	hook::iat("xlive.dll", XLivePBufferSetByte, 5019);
	hook::iat("xlive.dll", XLivePBufferGetDWORD, 5020);
	hook::iat("xlive.dll", XLivePBufferSetDWORD, 5021);
	hook::iat("xlive.dll", XLiveCreateProtectedDataContext, 5036);
	hook::iat("xlive.dll", XLiveQueryProtectedDataInformation, 5037);
	hook::iat("xlive.dll", XLiveCloseProtectedDataContext, 5038);
	hook::iat("xlive.dll", XLiveUnprotectData, 5035);
	hook::iat("xlive.dll", XLiveProtectData, 5034);
	hook::iat("xlive.dll", xlive_5367, 5367);
	hook::iat("xlive.dll", xlive_5372, 5372);
});