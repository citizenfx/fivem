/*
* Noekeon in SIMD
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_NOEKEON_SIMD_H__
#define BOTAN_NOEKEON_SIMD_H__

#include <botan/noekeon.h>

namespace Botan {

/**
* Noekeon implementation using SIMD operations
*/
class BOTAN_DLL Noekeon_SIMD : public Noekeon
   {
   public:
      size_t parallelism() const { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      BlockCipher* clone() const { return new Noekeon_SIMD; }
   };

}

#endif
