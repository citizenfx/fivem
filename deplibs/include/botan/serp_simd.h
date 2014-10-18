/*
* Serpent (SIMD)
* (C) 2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
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
      size_t parallelism() const { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      BlockCipher* clone() const { return new Serpent_SIMD; }
   };

}

#endif
