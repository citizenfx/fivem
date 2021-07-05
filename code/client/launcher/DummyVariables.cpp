/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "LauncherConfig.h"

#ifdef IS_TLS_DLL
#define DECLARE_TLS_VARS(i) \
	__declspec(thread) uint8_t tls1[sizeof(int) * i];
 
// dummy TLS variables to allocate TLS for the game to use
#pragma region tls
DECLARE_TLS_VARS(7096);
#pragma endregion

extern "C" extern char _tls_start;
extern "C" extern ULONG _tls_index;

extern "C" DLL_EXPORT void GetThreadLocalStorage(void** base, uint32_t* index)
{
	*base = &_tls_start;
	*index = _tls_index;
}
#elif defined(LAUNCHER_PERSONALITY_GAME)
void InitializeDummies()
{
}

// dummy game memory to overwrite with, well, the game
#if defined(LAUNCHER_PERSONALITY_GAME_MTL)
#pragma bss_seg(".cdummy")
char dummy_seg[0x02E22600];

char stub_seg[0x100000];
#elif defined(GTA_NY)
#define EXE_TEXT_SIZE 0xA7181A
#define EXE_RDATA_SIZE 0x1BCD03
#define EXE_DATA_SIZE 0xC6B50C

#pragma bss_seg(".crkstr")
char rkstr[0x000FC000 + 0x00135B4A + 0x00115D28 + 0x2000 + 0x00400000 + EXE_TEXT_SIZE + EXE_RDATA_SIZE + EXE_DATA_SIZE];
#elif defined(PAYNE)
#pragma bss_seg(".ctext")
char text[0x123FC000];

#pragma bss_seg(".crsrc")
char rsrc[0x38000];

#pragma bss_seg(".cdata")
char data[0x95000];
#elif defined(GTA_FIVE)
// only use a single segment as we're supposed to be patch-proof; we'll protect these appropriately later
#pragma bss_seg(".cdummy")
char dummy_seg[0x6000000];

char stub_seg[0x100000];
#elif defined(IS_RDR3)
// only use a single segment as we're supposed to be patch-proof; we'll protect these appropriately later
#pragma bss_seg(".cdummy")
char dummy_seg[0x8000000];

char stub_seg[0x100000];
#elif (!defined(IS_LAUNCHER))
#error No dummy segments defined!
#endif

#pragma data_seg(".zdata")
char zdata[200] = { 1 };
#endif
