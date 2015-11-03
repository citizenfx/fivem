/*
* MD5
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MD5_H__
#define BOTAN_MD5_H__

#include <botan/mdx_hash.h>

namespace Botan {

/**
* MD5
*/
class BOTAN_DLL MD5 : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "MD5"; }
      size_t output_length() const override { return 16; }
      HashFunction* clone() const override { return new MD5; }

      void clear() override;

      MD5() : MDx_HashFunction(64, false, true), M(16), digest(4)
         { clear(); }
   protected:
      void compress_n(const byte[], size_t blocks) override;
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
