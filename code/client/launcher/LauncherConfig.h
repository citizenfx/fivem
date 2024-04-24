/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#define CONTENT_URL "https://content.cfx.re/updates"
#define CONTENT_URL_WIDE L"https://content.cfx.re/updates"

#if defined(GTA_NY)
#define GAME_EXECUTABLE L"GTAIV.exe"
#define EXE_TEXT_SIZE 0xA7181A
#define EXE_RDATA_SIZE 0x1BCD03
#define EXE_DATA_SIZE 0xC6B50C
#elif defined(PAYNE)
#define GAME_EXECUTABLE L"MaxPayne3.exe"
#elif defined(GTA_FIVE)
#define GAME_EXECUTABLE L"GTA5.exe"
#elif defined(IS_RDR3)
#define GAME_EXECUTABLE L"RDR2.exe"
#elif defined(IS_LAUNCHER)
#define GAME_EXECUTABLE L"DUMMY.exe"
#else
#define GAME_EXECUTABLE L"Game.exe"
#endif
