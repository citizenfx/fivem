#pragma once

#if defined(GTA_NY)
#define PRODUCT_NAME L"CitizenFX:IV"
#define CONTENT_URL "http://content.citizen.re/iv/"
#define CONTENT_URL_WIDE L"http://content.citizen.re/iv/"
#define GAME_EXECUTABLE L"GTAIV.exe"
#elif defined(PAYNE)
#define PRODUCT_NAME L"CitizenPayne"
#define CONTENT_URL "http://content.citizen.re/payne/"
#define CONTENT_URL_WIDE L"http://content.citizen.re/payne/"
#define GAME_EXECUTABLE L"MaxPayne3.exe"
#else
#define PRODUCT_NAME L"Unknown CitizenFX Game"
#define CONTENT_URL "http://localhost/citfx/"
#define CONTENT_URL_WIDE L"http://localhost/citfx/"
#define GAME_EXECUTABLE L"Game.exe"
#endif