/*
* Low Level MPI Types
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MPI_TYPES_H__
#define BOTAN_MPI_TYPES_H__

#include <botan/types.h>

namespace Botan {

#if (BOTAN_MP_WORD_BITS == 8)
  typedef uint8_t word;
#elif (BOTAN_MP_WORD_BITS == 16)
  typedef uint16_t word;
#elif (BOTAN_MP_WORD_BITS == 32)
  typedef uint32_t word;
#elif (BOTAN_MP_WORD_BITS == 64)
  typedef uint64_t word;
#else
  #error BOTAN_MP_WORD_BITS must be 8, 16, 32, or 64
#endif

const word MP_WORD_MASK = ~static_cast<word>(0);
const word MP_WORD_TOP_BIT = static_cast<word>(1) << (8*sizeof(word) - 1);
const word MP_WORD_MAX = MP_WORD_MASK;

}

#endif
