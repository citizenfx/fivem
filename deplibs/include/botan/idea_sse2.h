/*
* IDEA in SSE2
* (C) 2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_IDEA_SSE2_H__
#define BOTAN_IDEA_SSE2_H__

#include <botan/idea.h>

namespace Botan {

/**
* IDEA in SSE2
*/
class BOTAN_DLL IDEA_SSE2 : public IDEA
   {
   public:
      size_t parallelism() const { return 8; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      BlockCipher* clone() const { return new IDEA_SSE2; }
   };

}

#endif
