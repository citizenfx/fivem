/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#if defined(GTA_NY)
#define PRODUCT_NAME L"CitizenFX:IV"
#define CONTENT_URL "http://content.citizen.re/iv/"
#define CONTENT_URL_WIDE L"http://content.citizen.re/iv/"
#define GAME_EXECUTABLE L"GTAIV.exe"
#define EXE_TEXT_SIZE 0x94C000
#define EXE_RDATA_SIZE 0x1BF000
#define EXE_DATA_SIZE 0xB46000
#elif defined(PAYNE)
#define PRODUCT_NAME L"CitizenPayne"
#define CONTENT_URL "http://content.citizen.re/payne/"
#define CONTENT_URL_WIDE L"http://content.citizen.re/payne/"
#define GAME_EXECUTABLE L"MaxPayne3.exe"
#elif defined(GTA_FIVE)
#define PRODUCT_NAME L"FiveM"
#define CONTENT_URL "https://runtime.fivem.net/client"
#define CONTENT_URL_WIDE L"https://runtime.fivem.net/client"
#define GAME_EXECUTABLE L"GTA5.exe"
#else
#define PRODUCT_NAME L"Unknown CitizenFX Game"
#define CONTENT_URL "http://localhost/citfx/"
#define CONTENT_URL_WIDE L"http://localhost/citfx/"
#define GAME_EXECUTABLE L"Game.exe"
#endif