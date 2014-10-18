/*
* Whirlpool
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_WHIRLPOOL_H__
#define BOTAN_WHIRLPOOL_H__

#include <botan/mdx_hash.h>

namespace Botan {

/**
* Whirlpool
*/
class BOTAN_DLL Whirlpool : public MDx_HashFunction
   {
   public:
      std::string name() const { return "Whirlpool"; }
      size_t output_length() const { return 64; }
      HashFunction* clone() const { return new Whirlpool; }

      void clear();

      Whirlpool() : MDx_HashFunction(64, true, true, 32), M(8), digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      static const u64bit C0[256];
      static const u64bit C1[256];
      static const u64bit C2[256];
      static const u64bit C3[256];
      static const u64bit C4[256];
      static const u64bit C5[256];
      static const u64bit C6[256];
      static const u64bit C7[256];

      secure_vector<u64bit> M, digest;
   };

}

#endif
