/*
* XTEA in SIMD
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
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
      size_t parallelism() const override { return 8; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;
      BlockCipher* clone() const override { return new XTEA_SIMD; }
   };

}

#endif
