// ==========================================================
// alterIWnet project
// 
// Component: xnp
// Sub-component: libnp
// Purpose: type definitions for libnp
//
// Initial author: NTAuthority
// Started: 2011-06-28
// ==========================================================

#pragma once

// ----------------------------------------------------------
// stdint.h-style integer definitions
// ----------------------------------------------------------
#if _MSC_VER < 1600
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

// ----------------------------------------------------------
// C export definitions
// ----------------------------------------------------------
#ifdef LIBNP_EXPORTS
#define LIBNP_API extern "C" __declspec(dllexport)
#else
#define LIBNP_API extern "C" __declspec(dllimport)
#endif

#define LIBNP_CALL __cdecl

// ----------------------------------------------------------
// Various (base) includes
// ----------------------------------------------------------
#define NPID_GET_USERID(x) (unsigned int)(x & 0xFFFFFFFF)

typedef uint64_t NPID;

#include "NPAsync.h"

// ----------------------------------------------------------
// Referenceable type
// ----------------------------------------------------------

class NPReferenceable
{
public:
	// adds a reference
	virtual void AddReference() = 0;

	// releases a reference
	virtual void Release() = 0;
};