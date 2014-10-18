/*
* SAFER-SK
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_SAFER_SK_H__
#define BOTAN_SAFER_SK_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* SAFER-SK
*/
class BOTAN_DLL SAFER_SK : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const;
      BlockCipher* clone() const;

      /**
      * @param rounds the number of rounds to use - must be between 1
      * and 13
      */
      SAFER_SK(size_t rounds);
   private:
      void key_schedule(const byte[], size_t);

      size_t rounds;
      secure_vector<byte> EK;
   };

}

#endif
