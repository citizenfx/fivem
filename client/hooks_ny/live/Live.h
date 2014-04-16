#pragma once

#include <WS2tcpip.h>

typedef struct
{
	IN_ADDR ina;
	IN_ADDR inaOnline;
	WORD wPortOnline;
	BYTE abEnet[6];
	BYTE abOnline[20];
} XNADDR;

typedef struct
{
	BYTE        ab[8];                          // xbox to xbox key identifier
} XNKID;

typedef struct
{
	BYTE        ab[16];                         // xbox to xbox key exchange key
} XNKEY;

typedef VOID XNQOS;

typedef struct _XOVERLAPPED             XOVERLAPPED, *PXOVERLAPPED;

typedef
VOID
(WINAPI *PXOVERLAPPED_COMPLETION_ROUTINE)(
__in    DWORD                       dwErrorCode,
__in    DWORD                       dwNumberOfBytesTransfered,
__inout PXOVERLAPPED                pOverlapped
);

typedef enum _XSESSION_STATE
{
	XSESSION_STATE_LOBBY = 0,
	XSESSION_STATE_REGISTRATION,
	XSESSION_STATE_INGAME,
	XSESSION_STATE_REPORTING,
	XSESSION_STATE_DELETED
} XSESSION_STATE;

typedef ULONGLONG XUID;

typedef struct _XUSER_DATA
{
	BYTE                                type;

	union
	{
		LONG                            nData;     // XUSER_DATA_TYPE_INT32
		LONGLONG                        i64Data;   // XUSER_DATA_TYPE_INT64
		double                          dblData;   // XUSER_DATA_TYPE_DOUBLE
		struct                                     // XUSER_DATA_TYPE_UNICODE
		{
			DWORD                       cbData;    // Includes null-terminator
			LPWSTR                      pwszData;
		} string;
		FLOAT                           fData;     // XUSER_DATA_TYPE_FLOAT
		struct                                     // XUSER_DATA_TYPE_BINARY
		{
			DWORD                       cbData;
			PBYTE                       pbData;
		} binary;
		FILETIME                        ftData;    // XUSER_DATA_TYPE_DATETIME
	};
} XUSER_DATA, *PXUSER_DATA;

typedef struct
{
	DWORD dwPropertyId;
	XUSER_DATA value;
} XUSER_PROPERTY, *PXUSER_PROPERTY;

typedef struct
{
	DWORD dwContextId;
	DWORD dwValue;
} XUSER_CONTEXT, *PXUSER_CONTEXT;

typedef struct
{
	XNKID sessionID;
	XNADDR hostAddress;
	XNKEY keyExchangeKey;
} XSESSION_INFO, *PXSESSION_INFO; // size 60

typedef struct
{
	XSESSION_INFO info;
	DWORD dwOpenPublicSlots;
	DWORD dwOpenPrivateSlots;
	DWORD dwFilledPublicSlots;
	DWORD dwFilledPrivateSlots;
	DWORD cProperties;
	DWORD cContexts;
	PXUSER_PROPERTY pProperties;
	PXUSER_CONTEXT pContexts;
} XSESSION_SEARCHRESULT, *PXSESSION_SEARCHRESULT;

typedef struct
{
	DWORD dwSearchResults;
	XSESSION_SEARCHRESULT *pResults;
} XSESSION_SEARCHRESULT_HEADER, *PXSESSION_SEARCHRESULT_HEADER;

typedef struct
{
	XUID xuidOnline;
	DWORD dwUserIndex;
	DWORD dwFlags;
} XSESSION_MEMBER;

typedef struct
{
	DWORD dwUserIndexHost;
	DWORD dwGameType;
	DWORD dwGameMode;
	DWORD dwFlags;
	DWORD dwMaxPublicSlots;
	DWORD dwMaxPrivateSlots;
	DWORD dwAvailablePublicSlots;
	DWORD dwAvailablePrivateSlots;
	DWORD dwActualMemberCount;
	DWORD dwReturnedMemberCount;
	XSESSION_STATE eState;
	ULONGLONG qwNonce;
	XSESSION_INFO sessionInfo;
	XNKID xnkidArbitration;
	XSESSION_MEMBER *pSessionMembers;
} XSESSION_LOCAL_DETAILS;

typedef struct _XOVERLAPPED
{
	ULONG_PTR InternalLow;
	ULONG_PTR InternalHigh;
	ULONG_PTR InternalContext;
	HANDLE hEvent;
	PXOVERLAPPED_COMPLETION_ROUTINE pCompletionRoutine;
	DWORD_PTR dwCompletionContext;
	DWORD dwExtendedError;
} XOVERLAPPED, *PXOVERLAPPED;

typedef struct _XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS
{
	DWORD dwBytesTotal;
	XUID xuidOwner;
	FILETIME ftCreated;
} XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS;