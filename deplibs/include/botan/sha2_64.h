/*
* SHA-{384,512}
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_SHA_64BIT_H__
#define BOTAN_SHA_64BIT_H__

#include <botan/mdx_hash.h>

namespace Botan {

/**
* SHA-384
*/
class BOTAN_DLL SHA_384 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "SHA-384"; }
      size_t output_length() const { return 48; }
      HashFunction* clone() const { return new SHA_384; }

      void clear();

      SHA_384() : MDx_HashFunction(128, true, true, 16), digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u64bit> digest;
   };

/**
* SHA-512
*/
class BOTAN_DLL SHA_512 : public MDx_HashFunction
   {
   public:
      std::string name() const { return "SHA-512"; }
      size_t output_length() const { return 64; }
      HashFunction* clone() const { return new SHA_512; }

      void clear();

      SHA_512() : MDx_HashFunction(128, true, true, 16), digest(8)
         { clear(); }
   private:
      void compress_n(const byte[], size_t blocks);
      void copy_out(byte[]);

      secure_vector<u64bit> digest;
   };

}

#endif
