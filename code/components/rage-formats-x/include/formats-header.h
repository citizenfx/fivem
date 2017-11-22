/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#ifndef RAGE_FORMATS_GAME
#ifdef _M_IX86
#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#else
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#endif
#endif

#ifdef COMPILING_RAGE_FORMATS_X
#define FORMATS_EXPORT __declspec(dllexport)
#else
#define FORMATS_EXPORT __declspec(dllimport)
#endif

#define DEFAULT_IMPL __declspec(selectany)

#ifndef RAGE_FORMATS_GAME
#error "RAGE_FORMATS_GAME must be specified when including header files from rage-formats-x."
#endif

#define IS_RAGE_DEFINED_(game, file) RAGE_FORMATS_##game##_##file
#define IS_RAGE_DEFINED(game, file) IS_RAGE_DEFINED_(game, file)
#define IS_RAGE_GAME_DEFINED IS_RAGE_DEFINED(RAGE_FORMATS_GAME, RAGE_FORMATS_FILE)

#ifdef RAGE_FORMATS_OK
#undef RAGE_FORMATS_OK
#endif

#if !IS_RAGE_GAME_DEFINED
#define RAGE_FORMATS_OK
#endif

#ifdef RAGE_FORMATS_OK
#ifdef RAGE_FORMATS_GAME_NY
#pragma pack(push, 4)
#endif

#include <formats-namespace-header.h>
#endif