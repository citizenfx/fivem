/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

// client-side shared include file
#pragma warning(disable: 4251) // needs to have dll-interface to be used by clients
#pragma warning(disable: 4273) // inconsistent dll linkage
#pragma warning(disable: 4275) // non dll-interface class used as base
#pragma warning(disable: 4244) // possible loss of data
#pragma warning(disable: 4800) // forcing value to bool
#pragma warning(disable: 4290) // C++ exception specification ignored

// MSVC odd defines
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

// platform primary include
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <versionhelpers.h>

#ifdef GTA_NY
#include <d3d9.h>
#endif

#undef NDEBUG

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

#include "Registry.h"
#include "HookFunction.h"

#ifdef HAS_LOCAL_H
#include "Local.h"
#endif