#include "StdInc.h"
#include "Live.h"
#include "CrossLibraryInterfaces.h"

// #5312: XFriendsCreateEnumerator
DWORD __stdcall XFriendsCreateEnumerator(DWORD, DWORD, DWORD, DWORD, HANDLE * phEnum)
{
	*phEnum = INVALID_HANDLE_VALUE;
	return 0;
}

// #5314: XUserMuteListQuery
int __stdcall XUserMuteListQuery(DWORD dwUserIndex,
											 XUID XuidRemoteTalker,
											 BOOL *pfOnMuteList)
{
	//trace ("XUserMuteListQuery\n");
	*pfOnMuteList = FALSE;
	return 0;
}

// #5315: XInviteGetAcceptedInfo
int __stdcall XInviteGetAcceptedInfo(DWORD, DWORD)
{
	return 1;
}

// #5316: XInviteSend
int __stdcall XInviteSend(DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5317: XSessionWriteStats
DWORD __stdcall XSessionWriteStats(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5318
int __stdcall XSessionStart(DWORD, DWORD, DWORD)
{
	return 0;
}

void GSClient_QueryMaster(PXSESSION_SEARCHRESULT_HEADER h, PXOVERLAPPED overlapped);

// #5319: XSessionSearchEx
DWORD __stdcall XSessionSearchEx(DWORD dwProcedureIndex,
											 DWORD dwUserIndex,
											 DWORD dwNumResults,
											 DWORD dwNumUsers,
											 WORD wNumProperties,
											 WORD wNumContexts,
											 PXUSER_PROPERTY pSearchProperties,
											 PXUSER_CONTEXT pSearchContexts,
											 DWORD *pcbResultsBuffer,
											 PXSESSION_SEARCHRESULT_HEADER pSearchResults,
											 PXOVERLAPPED pXOverlapped
											 )
{
	if (!pSearchResults)
	{
		*pcbResultsBuffer = sizeof(XSESSION_SEARCHRESULT_HEADER)+(((sizeof(XSESSION_SEARCHRESULT)+(sizeof(XUSER_PROPERTY)* 4) + (sizeof(XUSER_CONTEXT)* 4)) * dwNumResults));

		return ERROR_IO_PENDING; // actually ERROR_INSUFFICIENT_BUFFER
	}

	pXOverlapped->InternalLow = ERROR_IO_PENDING;

	return ERROR_IO_PENDING;
}

// #5322: XSessionModify
DWORD __stdcall XSessionModify(DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5323: XSessionMigrateHost
DWORD __stdcall XSessionMigrateHost(HANDLE hSession, DWORD dwUserIndex, XSESSION_INFO* pSessionInfo, PXOVERLAPPED* xOverlapped)
{
	return ERROR_SUCCESS;
}

// #5324: XOnlineGetNatType
int __stdcall XOnlineGetNatType()
{
	//trace ("XOnlineGetNatType\n");
	return 1;
}

// #5325: XSessionLeaveLocal
DWORD __stdcall XSessionLeaveLocal(DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5326: XSessionJoinRemote
DWORD __stdcall XSessionJoinRemote(DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5327: XSessionJoinLocal
DWORD __stdcall XSessionJoinLocal(DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5328: XSessionGetDetails
DWORD __stdcall XSessionGetDetails(HANDLE hSession,
											   DWORD *pcbResultsBuffer,
											   XSESSION_LOCAL_DETAILS *pSessionDetails,
											   PXOVERLAPPED *pXOverlapped
											   )
{
	memset(pSessionDetails, 0, *pcbResultsBuffer);
	pSessionDetails->dwReturnedMemberCount = 3;

	return 0;
}

// #5329: XSessionFlushStats
int __stdcall XSessionFlushStats(DWORD, DWORD)
{
	return 0;
}

// #5330: XSessionDelete
DWORD __stdcall XSessionDelete(DWORD, DWORD)
{
	return 0;
}

struct XUSER_READ_PROFILE_SETTINGS
{
	DWORD	dwLength;
	BYTE *	pSettings;
};

// #5331: XUserReadProfileSettings
DWORD __stdcall XUserReadProfileSettings(DWORD dwTitleId, DWORD dwUserIndex, DWORD dwNumSettingIds,
													 DWORD * pdwSettingIds, DWORD * pcbResults, XUSER_READ_PROFILE_SETTINGS * pResults, DWORD pOverlapped)
{
	if (*pcbResults < 1036)
	{
		*pcbResults = 1036;
		return ERROR_INSUFFICIENT_BUFFER;
	}
	memset(pResults, 0, *pcbResults);
	pResults->dwLength = *pcbResults - sizeof (XUSER_READ_PROFILE_SETTINGS);
	pResults->pSettings = (BYTE *)pResults + sizeof (XUSER_READ_PROFILE_SETTINGS);
	return 0;
}

// #5332: XSessionEnd
int __stdcall XSessionEnd(DWORD, DWORD)
{
	return 0;
}

// #5333: XSessionArbitrationRegister
DWORD __stdcall XSessionArbitrationRegister(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5335: XTitleServerCreateEnumerator
DWORD __stdcall XTitleServerCreateEnumerator(LPCSTR pszServerInfo, DWORD cItem, DWORD * pcbBuffer, PHANDLE phEnum)
{
	return 1;
}

// #5336: XSessionLeaveRemote
DWORD __stdcall XSessionLeaveRemote(DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5337: XUserWriteProfileSettings
DWORD __stdcall XUserWriteProfileSettings(DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5339: XUserReadProfileSettingsByXuid
DWORD __stdcall XUserReadProfileSettingsByXuid(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5343: XLiveCalculateSkill
DWORD __stdcall XLiveCalculateSkill(DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5344: XStorageBuildServerPath
DWORD __stdcall XStorageBuildServerPath(DWORD dwUserIndex, DWORD StorageFacility,
													void * pvStorageFacilityInfo, DWORD dwStorageFacilityInfoSize,
													void * pwszItemName, void * pwszServerPath, DWORD * pdwServerPathLength)
{
	return 0;
}

// #5345: XStorageDownloadToMemory
DWORD __stdcall XStorageDownloadToMemory(DWORD dwUserIndex,
													 const WCHAR *wszServerPath,
													 DWORD dwBufferSize,
													 BYTE *pbBuffer,
													 DWORD cbResults,
													 XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS *pResults,
													 XOVERLAPPED *pOverlapped
													 )
{
	pOverlapped->InternalLow = ERROR_SUCCESS;
	pOverlapped->InternalHigh = 0;

	if (pOverlapped->hEvent)
	{
		SetEvent(pOverlapped->hEvent);
	}

	pResults->dwBytesTotal = 4 * 147;
	pResults->xuidOwner = 0x1234;

	memset(pbBuffer, 0, 4 * 147);

	return ERROR_IO_PENDING;
}

typedef struct _STRING_VERIFY_RESPONSE
{
	WORD wNumStrings;
	HRESULT *pStringResult;
} STRING_VERIFY_RESPONSE;

DWORD __stdcall XStringVerify(DWORD, DWORD, DWORD numStrings, DWORD, DWORD, STRING_VERIFY_RESPONSE * pResult, DWORD)
{ // XStringVerify
	pResult->wNumStrings = (WORD)numStrings;
	pResult->pStringResult = (HRESULT *)pResult + 1;

	for (DWORD i = 0; i < numStrings; i++)
	{
		pResult->pStringResult[i] = 0;
	}

	return 0;
}

DWORD __stdcall XNetGetTitleXnAddr(XNADDR * pAddr);

void HandleSessionInfo(PXSESSION_INFO pSessionInfo)
{
	memset(pSessionInfo, 0, sizeof(XSESSION_INFO));
	pSessionInfo->sessionID.ab[0] = 1;
	pSessionInfo->keyExchangeKey.ab[0] = 1;
	pSessionInfo->hostAddress.inaOnline.s_addr = g_netLibrary->GetServerNetID();
	pSessionInfo->hostAddress.ina.s_addr = g_netLibrary->GetServerNetID();

	XNetGetTitleXnAddr(&pSessionInfo->hostAddress);
}

DWORD __stdcall XSessionCreate(DWORD dwFlags,
										   DWORD dwUserIndex,
										   DWORD dwMaxPublicSlots,
										   DWORD dwMaxPrivateSlots,
										   ULONGLONG *pqwSessionNonce,
										   PXSESSION_INFO pSessionInfo,
										   PXOVERLAPPED pXOverlapped,
										   HANDLE *ph)
{
	*ph = (HANDLE)1;

	if (dwFlags & 1)
	{
		HandleSessionInfo(pSessionInfo);
	}

	return ERROR_SUCCESS;
}

// #5305: XStorageUploadFromMemory
DWORD __stdcall XStorageUploadFromMemory(DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5310: XOnlineStartup
int __stdcall XOnlineStartup()
{
	return 0;
}

// #5311: XOnlineCleanup
int __stdcall XOnlineCleanup()
{
	return 0;
}

static HookFunction hookFunction([] ()
{
	hook::iat("xlive.dll", XSessionCreate, 5300);
	hook::iat("xlive.dll", XStorageUploadFromMemory, 5305);
	hook::iat("xlive.dll", XStringVerify, 5303);
	hook::iat("xlive.dll", XOnlineStartup, 5310);
	hook::iat("xlive.dll", XOnlineCleanup, 5311);
	hook::iat("xlive.dll", XFriendsCreateEnumerator, 5312);
	hook::iat("xlive.dll", XUserMuteListQuery, 5314);
	hook::iat("xlive.dll", XInviteGetAcceptedInfo, 5315);
	hook::iat("xlive.dll", XInviteSend, 5316);
	hook::iat("xlive.dll", XSessionWriteStats, 5317);
	hook::iat("xlive.dll", XSessionStart, 5318);
	hook::iat("xlive.dll", XSessionSearchEx, 5319);
	hook::iat("xlive.dll", XSessionModify, 5322);
	hook::iat("xlive.dll", XSessionMigrateHost, 5323);
	hook::iat("xlive.dll", XOnlineGetNatType, 5324);
	hook::iat("xlive.dll", XSessionLeaveLocal, 5325);
	hook::iat("xlive.dll", XSessionJoinRemote, 5326);
	hook::iat("xlive.dll", XSessionJoinLocal, 5327);
	hook::iat("xlive.dll", XSessionGetDetails, 5328);
	hook::iat("xlive.dll", XSessionFlushStats, 5329);
	hook::iat("xlive.dll", XSessionDelete, 5330);
	hook::iat("xlive.dll", XUserReadProfileSettings, 5331);
	hook::iat("xlive.dll", XSessionEnd, 5332);
	hook::iat("xlive.dll", XSessionArbitrationRegister, 5333);
	hook::iat("xlive.dll", XTitleServerCreateEnumerator, 5335);
	hook::iat("xlive.dll", XSessionLeaveRemote, 5336);
	hook::iat("xlive.dll", XUserWriteProfileSettings, 5337);
	hook::iat("xlive.dll", XUserReadProfileSettingsByXuid, 5339);
	hook::iat("xlive.dll", XLiveCalculateSkill, 5343);
	hook::iat("xlive.dll", XStorageBuildServerPath, 5344);
	hook::iat("xlive.dll", XStorageDownloadToMemory, 5345);
});