#pragma once

// client-side shared include file

// platform primary include
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// MSVC odd defines
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

// C/C++ headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <map>
#include <unordered_map>
#include <vector>
#include <string>

// our common includes
#include <Utils.h>
#include <sigslot.h>

// module-specific includes
#ifdef COMPILING_HOOKS
#include "Hooking.h"
#include "HookFunction.h"
#endif