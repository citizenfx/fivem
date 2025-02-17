/***
* Copyright (C) Microsoft. All rights reserved.
* Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
*
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* Standard macros and definitions.
* This header has minimal dependency on windows headers and is safe for use in the public API
*
* For the latest on this and related APIs, please see: https://github.com/Microsoft/cpprestsdk
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#if defined(_WIN32) // Settings specific to Windows

#if _MSC_VER >= 1900
#define CPPREST_NOEXCEPT noexcept
#else
#define CPPREST_NOEXCEPT
#endif

#define CASABLANCA_UNREFERENCED_PARAMETER(x) (x)

#include <sal.h>

#else // End settings specific to Windows

// Settings common to all but Windows

#define __declspec(x) __attribute__ ((x))
#define dllimport
#define novtable /* no novtable equivalent */
#define __assume(x) do { if (!(x)) __builtin_unreachable(); } while (false)
#define CASABLANCA_UNREFERENCED_PARAMETER(x) (void)x
#define CPPREST_NOEXCEPT noexcept

#include <assert.h>
#define _ASSERTE(x) assert(x)

#ifdef _In_
#undef _In_
#endif
#define _In_

#ifdef _Inout_
#undef _Inout_
#endif
#define _Inout_

#ifdef _Out_
#undef _Out_
#endif
#define _Out_

#ifdef _In_z_
#undef _In_z_
#endif
#define _In_z_

#ifdef _Out_z_
#undef _Out_z_
#endif
#define _Out_z_

#ifdef _Inout_z_
#undef _Inout_z_
#endif
#define _Inout_z_

#ifdef _In_opt_
#undef _In_opt_
#endif
#define _In_opt_

#ifdef _Out_opt_
#undef _Out_opt_
#endif
#define _Out_opt_

#ifdef _Inout_opt_
#undef _Inout_opt_
#endif
#define _Inout_opt_

#ifdef _Out_writes_
#undef _Out_writes_
#endif
#define _Out_writes_(x)

#ifdef _Out_writes_opt_
#undef _Out_writes_opt_
#endif
#define _Out_writes_opt_(x)

#ifdef _In_reads_
#undef _In_reads_
#endif
#define _In_reads_(x)

#ifdef _Inout_updates_bytes_
#undef _Inout_updates_bytes_
#endif
#define _Inout_updates_bytes_(x)

#if not defined __cdecl
#if defined cdecl
#define __cdecl __attribute__ ((cdecl))
#else
#define __cdecl
#endif

#ifdef __clang__
#include <cstdio>
#endif

#endif

#endif


#ifdef _NO_ASYNCRTIMP
#define _ASYNCRTIMP
#else
#ifdef _ASYNCRT_EXPORT
#define _ASYNCRTIMP __declspec(dllexport)
#else
#define _ASYNCRTIMP __declspec(dllimport)
#endif
#endif

#ifdef CASABLANCA_DEPRECATION_NO_WARNINGS
#define CASABLANCA_DEPRECATED(x)
#else
#define CASABLANCA_DEPRECATED(x) __declspec(deprecated(x))
#endif
