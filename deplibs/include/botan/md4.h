/*
* MD4
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MD4_H__
#define BOTAN_MD4_H__

#include <botan/mdx_hash.h>

namespace Botan {

/**
* MD4
*/
class BOTAN_DLL MD4 : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "MD4"; }
      size_t output_length() const override { return 16; }
      HashFunction* clone() const override { return new MD4; }

      void clear() override;

      MD4() : MDx_HashFunction(64, false, true), M(16), digest(4)
         { clear(); }
   protected:
      void compress_n(const byte input[], size_t blocks) override;
      void copy_out(byte[]) override;

      /**
      * The message buffer, exposed for use by subclasses (x86 asm)
      */
      secure_vector<u32bit> M;

      /**
      * The digest value, exposed for use by subclasses (x86 asm)
      */
      secure_vector<u32bit> digest;
   };

}

#endif
