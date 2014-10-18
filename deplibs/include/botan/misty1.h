/*
* MISTY1
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_MISTY1_H__
#define BOTAN_MISTY1_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* MISTY1
*/
class BOTAN_DLL MISTY1 : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "MISTY1"; }
      BlockCipher* clone() const { return new MISTY1; }

      /**
      * @param rounds the number of rounds. Must be 8 with the current
      * implementation
      */
      MISTY1(size_t rounds = 8);
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u16bit> EK, DK;
   };

}

#endif
