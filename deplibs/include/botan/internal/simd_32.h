/*
* Lightweight wrappers for SIMD operations
* (C) 2009,2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SIMD_32_H__
#define BOTAN_SIMD_32_H__

#include <botan/types.h>

#if defined(BOTAN_HAS_SIMD_SSE2)
  #include <botan/internal/simd_sse2.h>
  namespace Botan { typedef SIMD_SSE2 SIMD_32; }

#elif defined(BOTAN_HAS_SIMD_ALTIVEC)
  #include <botan/internal/simd_altivec.h>
  namespace Botan { typedef SIMD_Altivec SIMD_32; }

#elif defined(BOTAN_HAS_SIMD_SCALAR)
  #include <botan/internal/simd_scalar.h>
  namespace Botan { typedef SIMD_Scalar<u32bit,4> SIMD_32; }

#else
  #error "No SIMD module defined"

#endif

#endif
