#pragma once

// client-side shared include file

// MSVC odd defines
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

// platform primary include
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef GTA_NY
#include <d3d9.h>
#endif

// C/C++ headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

// our common includes
#define COMPONENT_EXPORT

#include "EventCore.h"

#include <Utils.h>
#include <sigslot.h>

// module-specific includes
#ifdef COMPILING_HOOKS
#include "Hooking.h"
#endif

#include "HookFunction.h"

#ifdef HAS_LOCAL_H
#include "Local.h"
#endif