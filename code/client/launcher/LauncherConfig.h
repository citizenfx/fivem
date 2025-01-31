/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifndef CFX_UPDATER_URL
#define CFX_UPDATER_URL "https://content.cfx.re/updates"
#endif

#if defined(GTA_NY)
#define PRODUCT_NAME L"LibertyM"
#define CONTENT_NAME "libertym"
#define GAME_EXECUTABLE L"GTAIV.exe"
#define EXE_TEXT_SIZE 0xA7181A
#define EXE_RDATA_SIZE 0x1BCD03
#define EXE_DATA_SIZE 0xC6B50C
#elif defined(PAYNE)
#define PRODUCT_NAME L"CitizenPayne"
#define CONTENT_NAME "paynefx"
#define GAME_EXECUTABLE L"MaxPayne3.exe"
#elif defined(GTA_FIVE)
#define PRODUCT_NAME L"FiveM"
#define GAME_EXECUTABLE L"GTA5.exe"
#define CONTENT_NAME "fivereborn"
#define LINK_PROTOCOL L"fivem"
#elif defined(IS_RDR3)
#define PRODUCT_NAME L"RedM"
#define CONTENT_NAME "redm"
#define GAME_EXECUTABLE L"RDR2.exe"
#define LINK_PROTOCOL L"redm"
#elif defined(IS_LAUNCHER)
#define PRODUCT_NAME L"Cfx.re Launcher"
#define GAME_EXECUTABLE L"DUMMY.exe"
#define CONTENT_NAME "launcher"
#else
#define PRODUCT_NAME L"Unknown CitizenFX Game"
#define GAME_EXECUTABLE L"Game.exe"
#define CONTENT_NAME "unk"
#endif
