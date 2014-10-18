/*
* RC2
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_RC2_H__
#define BOTAN_RC2_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* RC2
*/
class BOTAN_DLL RC2 : public Block_Cipher_Fixed_Params<8, 1, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      /**
      * Return the code of the effective key bits
      * @param bits key length
      * @return EKB code
      */
      static byte EKB_code(size_t bits);

      void clear();
      std::string name() const { return "RC2"; }
      BlockCipher* clone() const { return new RC2; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u16bit> K;
   };

}

#endif
