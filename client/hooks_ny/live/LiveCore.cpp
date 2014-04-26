#include "StdInc.h"
#include "Live.h"
#include "CrossLibraryInterfaces.h"

DWORD xuidBase;

// #5214: XShowPlayerReviewUI
int __stdcall XShowPlayerReviewUI(DWORD, DWORD, DWORD)
{
	return 0;
}

// #5215: XShowGuideUI
int __stdcall XShowGuideUI(DWORD)
{
	return 1;
}

// #5216: XShowKeyboardUI
extern "C" int __stdcall XShowKeyboardUI(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5251: XCloseHandle
int __stdcall XCloseHandle(DWORD)
{
	return 0;
}

// #5252: XShowGamerCardUI
int __stdcall XShowGamerCardUI(DWORD, DWORD, DWORD)
{
	return 0;
}

// #5254: XCancelOverlapped
int __stdcall XCancelOverlapped(DWORD)
{
	return 0;
}

// #5256: XEnumerate
int __stdcall XEnumerate(HANDLE hEnum, void * pvBuffer, DWORD cbBuffer, DWORD * pcItemsReturned, XOVERLAPPED * pOverlapped)
{
	if (pcItemsReturned)
		*pcItemsReturned = 0;
	return 0;	// some error ? 
}

// #5260: XShowSigninUI
int __stdcall XShowSigninUI(DWORD, DWORD)
{
	return 0;
}

// #5261: XUserGetXUID
int __stdcall XUserGetXUID(DWORD, __int64 * pXuid)
{
	*pXuid = xuidBase;
	return 0; // ???
}


// #5262: XUserGetSigninState
DWORD __stdcall XUserGetSigninState(DWORD dwUserIndex)
{
	return 2;
}

// #5263: XUserGetName
int __stdcall XUserGetName(DWORD dwUserId, char * pBuffer, DWORD dwBufLen)
{
	if (dwBufLen < 8)
		return 1;
	//memcpy (pBuffer, "Player1", 8);
	_snprintf(pBuffer, 8, "P%d", xuidBase);

	return 0;
}

// #5264: XUserAreUsersFriends
int __stdcall XUserAreUsersFriends(DWORD dwUserIndex, DWORD * pXuids, DWORD dwXuidCount, DWORD * pResult, void * pOverlapped)
{
	return ERROR_SUCCESS;
}

// #5265: XUserCheckPrivilege
int __stdcall XUserCheckPrivilege(DWORD user, DWORD priv, PBOOL b)
{
	*b = TRUE;
	return ERROR_SUCCESS;
}

struct XUSER_SIGNIN_INFO
{
	DWORD	xuidL;
	DWORD	xuidH;
	DWORD    dwInfoFlags;
	DWORD	UserSigninState;
	DWORD    dwGuestNumber;
	DWORD    dwSponsorUserIndex;
	CHAR     szUserName[16];
};

// #5267: XUserGetSigninInfo
int __stdcall XUserGetSigninInfo(DWORD dwUser, DWORD dwFlags, XUSER_SIGNIN_INFO * pInfo)
{
	pInfo->xuidL = dwFlags != 1 ? 1337 : xuidBase;
	pInfo->xuidH = 0;
	if (dwFlags == 2)
	{
		pInfo->dwInfoFlags = 1;
		pInfo->UserSigninState = 1;
	}
	else
	{
		pInfo->dwInfoFlags = 1;
		pInfo->UserSigninState = 2;
	}
	//strcpy (pInfo->szUserName, "Player");
	_snprintf(pInfo->szUserName, 8, "P%d", xuidBase);
	pInfo->dwSponsorUserIndex = 0;
	pInfo->dwGuestNumber = 0;

	return 0;
}

// #5270: XNotifyCreateListener
HANDLE __stdcall XNotifyCreateListener(DWORD l, DWORD h)
{
	g_netLibrary->SetBase(xuidBase);

	// 'has' to be a valid handle as GTA will call CloseHandle on it
	return CreateEvent(NULL, FALSE, FALSE, NULL);
}

// #5273: XUserReadGamerpictureByKey
extern "C" int __stdcall XUserReadGamerpictureByKey(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD)
{
	return 0;
}

// #5275: XShowFriendsUI
extern "C" int __stdcall XShowFriendsUI(DWORD)
{
	return 0;
}

// #5276: XUserSetProperty
int __stdcall XUserSetProperty(DWORD dwUserIndex,
										   DWORD dwPropertyId,
										   DWORD cbValue,
										   CONST VOID *pvValue
										   )
{
	return 0;
}

// #5277: XUserSetContext
int __stdcall XUserSetContext(DWORD dwUserIndex,
										  DWORD dwContextId,
										  DWORD dwContextValue
										  )
{
	return 0;
}

// #5278: XUserWriteAchievements
DWORD __stdcall XUserWriteAchievements(DWORD, DWORD, DWORD)
{
	return 0;
}

// #5280: XUserCreateAchievementEnumerator
DWORD __stdcall XUserCreateAchievementEnumerator(DWORD dwTitleId, DWORD dwUserIndex, DWORD xuidL, DWORD xuidHi, DWORD dwDetailFlags, DWORD dwStartingIndex, DWORD cItem, DWORD * pcbBuffer, HANDLE * phEnum)
{
	return 1;   // return error (otherwise, 0-size buffer will be allocated)
}

// #5281: XUserReadStats
DWORD __stdcall XUserReadStats(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD * pcbResults, DWORD * pResults, void *)
{
	if (pcbResults)
		*pcbResults = 4;
	if (pResults)
		*pResults = 0;
	return 0;
}

// #5284: XUserCreateStatsEnumeratorByRank
DWORD __stdcall XUserCreateStatsEnumeratorByRank(DWORD dwTitleId, DWORD dwRankStart, DWORD dwNumRows, DWORD dwNuStatSpec, void * pSpecs, DWORD * pcbBuffer, PHANDLE phEnum)
{
	if (pcbBuffer)
		*pcbBuffer = 0;
	*phEnum = INVALID_HANDLE_VALUE;
	return 1;
}

// #5286: XUserCreateStatsEnumeratorByXuid
DWORD __stdcall XUserCreateStatsEnumeratorByXuid(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD * pcbBuffer, PHANDLE phEnum)
{
	if (pcbBuffer)
		pcbBuffer = 0;
	*phEnum = INVALID_HANDLE_VALUE;
	return 1;
}

// #5292: XUserSetContextEx
int __stdcall XUserSetContextEx(DWORD dwUserIndex, DWORD dwContextId, DWORD dwContextValue, PXOVERLAPPED pOverlapped)
{
	pOverlapped->InternalLow = ERROR_SUCCESS;
	pOverlapped->InternalHigh = 0;

	if (pOverlapped->hEvent)
	{
		SetEvent(pOverlapped->hEvent);
	}

	return ERROR_IO_PENDING;
}

// #5293: XUserSetPropertyEx
extern "C" int __stdcall XUserSetPropertyEx(DWORD dwUserIndex, DWORD dwPropertyId, DWORD cbValue, void * pvValue, void * pOverlapped)
{
	//trace ("XUserSetPropertyEx (%d, 0x%x, ...)\n", dwUserIndex, dwPropertyId);
	return 0;
}

static uint16_t g_lastServerNetID;

// #651: XNotifyGetNext
int __stdcall XNotifyGetNext(HANDLE hNotification, DWORD dwMsgFilter, DWORD * pdwId, void * pParam)
{
	if (g_netLibrary->GetServerNetID() != g_lastServerNetID)
	{
		*pdwId = 0xA; // 'signin changed'
		*(DWORD*)pParam = 1; // user 1 is valid

		g_lastServerNetID = g_netLibrary->GetServerNetID();

		SetWindowTextA(*(HWND*)0x1849DDC, va("GTAIV (NetID: %d)", g_lastServerNetID));

		XNADDR* g_titleAddrCached = (XNADDR*)0x197CF18;
		g_titleAddrCached->ina.s_addr = g_lastServerNetID;
		g_titleAddrCached->inaOnline.s_addr = g_lastServerNetID;

		CPlayerInfo* playerInfo = CPlayerInfo::GetLocalPlayer();

		if (playerInfo)
		{
			playerInfo->address.ina.s_addr = g_lastServerNetID;
			playerInfo->address.inaOnline.s_addr = g_lastServerNetID;
		}

		trace("set xnaddr, %08x, %04x, %016llx\n", g_titleAddrCached->inaOnline.s_addr, g_titleAddrCached->wPortOnline, *(uint64_t*)g_titleAddrCached->abOnline);

		// notify signin status change
		((void(__stdcall *)(int))0x75DE80)(0);

		return 1;
	}

	return 0;   // no notifications
}

// #652: XNotifyPositionUI
DWORD __stdcall XNotifyPositionUI(DWORD dwPosition)
{
	return 0;
}

// #1082: XGetOverlappedExtendedError
DWORD __stdcall XGetOverlappedExtendedError(void *)
{
	return 0;
}

// #1083: XGetOverlappedResult
DWORD __stdcall XGetOverlappedResult(PXOVERLAPPED overlapped, DWORD * pResult, DWORD bWait)
{
	if (pResult)
		*pResult = overlapped->InternalContext;

	return 0;
}

static HookFunction hookFunction([] ()
{
	srand(GetTickCount());
	xuidBase = rand();

	hook::iat("xlive.dll", XNotifyGetNext, 651);
	hook::iat("xlive.dll", XNotifyPositionUI, 652);
	hook::iat("xlive.dll", XGetOverlappedExtendedError, 1082);
	hook::iat("xlive.dll", XGetOverlappedResult, 1083);
	hook::iat("xlive.dll", XCloseHandle, 5251);
	hook::iat("xlive.dll", XShowPlayerReviewUI, 5214);
	hook::iat("xlive.dll", XShowGuideUI, 5215);
	hook::iat("xlive.dll", XShowKeyboardUI, 5216);
	hook::iat("xlive.dll", XShowGamerCardUI, 5252);
	hook::iat("xlive.dll", XCancelOverlapped, 5254);
	hook::iat("xlive.dll", XEnumerate, 5256);
	hook::iat("xlive.dll", XShowSigninUI, 5260);
	hook::iat("xlive.dll", XUserGetXUID, 5261);
	hook::iat("xlive.dll", XUserGetSigninState, 5262);
	hook::iat("xlive.dll", XUserGetName, 5263);
	hook::iat("xlive.dll", XUserAreUsersFriends, 5264);
	hook::iat("xlive.dll", XUserCheckPrivilege, 5265);
	hook::iat("xlive.dll", XUserGetSigninInfo, 5267);
	hook::iat("xlive.dll", XNotifyCreateListener, 5270);
	hook::iat("xlive.dll", XUserReadGamerpictureByKey, 5273);
	hook::iat("xlive.dll", XShowFriendsUI, 5275);
	hook::iat("xlive.dll", XUserSetProperty, 5276);
	hook::iat("xlive.dll", XUserSetContext, 5277);
	hook::iat("xlive.dll", XUserWriteAchievements, 5278);
	hook::iat("xlive.dll", XUserCreateAchievementEnumerator, 5280);
	hook::iat("xlive.dll", XUserReadStats, 5281);
	hook::iat("xlive.dll", XUserCreateStatsEnumeratorByRank, 5284);
	hook::iat("xlive.dll", XUserCreateStatsEnumeratorByXuid, 5286);
	hook::iat("xlive.dll", XUserSetContextEx, 5292);
	hook::iat("xlive.dll", XUserSetPropertyEx, 5293);
});