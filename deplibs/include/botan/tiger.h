/*
* Tiger
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TIGER_H__
#define BOTAN_TIGER_H__

#include <botan/mdx_hash.h>

namespace Botan {

/**
* Tiger
*/
class BOTAN_DLL Tiger : public MDx_HashFunction
   {
   public:
      std::string name() const;
      size_t output_length() const { return hash_len; }

      HashFunction* clone() const
         {
         return new Tiger(output_length(), passes);
         }

      void clear();

      /**
      * @param out_size specifies the output length; can be 16, 20, or 24
      * @param passes to make in the algorithm
      */
      Tiger(size_t out_size = 24, size_t passes = 3);
   private:
      void compress_n(const byte[], size_t block);
      void copy_out(byte[]);

      static void pass(u64bit& A, u64bit& B, u64bit& C,
                       const secure_vector<u64bit>& M,
                       byte mul);

      static const u64bit SBOX1[256];
      static const u64bit SBOX2[256];
      static const u64bit SBOX3[256];
      static const u64bit SBOX4[256];

      secure_vector<u64bit> X, digest;
      const size_t hash_len, passes;
   };

}

#endif
