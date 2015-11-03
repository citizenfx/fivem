/*
* Serpent (SIMD)
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SERPENT_SIMD_H__
#define BOTAN_SERPENT_SIMD_H__

#include <botan/serpent.h>

namespace Botan {

/**
* Serpent implementation using SIMD
*/
class BOTAN_DLL Serpent_SIMD : public Serpent
   {
   public:
      size_t parallelism() const override { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      BlockCipher* clone() const override { return new Serpent_SIMD; }
   };

}

#endif
