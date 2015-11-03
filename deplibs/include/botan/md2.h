/*
* MD2
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MD2_H__
#define BOTAN_MD2_H__

#include <botan/hash.h>

namespace Botan {

/**
* MD2
*/
class BOTAN_DLL MD2 : public HashFunction
   {
   public:
      std::string name() const override { return "MD2"; }
      size_t output_length() const override { return 16; }
      size_t hash_block_size() const override { return 16; }
      HashFunction* clone() const override { return new MD2; }

      void clear() override;

      MD2() : X(48), checksum(16), buffer(16)
         { clear(); }
   private:
      void add_data(const byte[], size_t) override;
      void hash(const byte[]);
      void final_result(byte[]) override;

      secure_vector<byte> X, checksum, buffer;
      size_t position;
   };

}

#endif
