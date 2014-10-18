/*
* XTEA in SIMD
* (C) 2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_XTEA_SIMD_H__
#define BOTAN_XTEA_SIMD_H__

#include <botan/xtea.h>

namespace Botan {

/**
* XTEA implemented using SIMD operations
*/
class BOTAN_DLL XTEA_SIMD : public XTEA
   {
   public:
      size_t parallelism() const { return 8; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;
      BlockCipher* clone() const { return new XTEA_SIMD; }
   };

}

#endif
