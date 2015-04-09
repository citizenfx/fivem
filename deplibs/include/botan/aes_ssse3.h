/*
* AES using SSSE3
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AES_SSSE3_H__
#define BOTAN_AES_SSSE3_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* AES-128 using SSSE3
*/
class BOTAN_DLL AES_128_SSSE3 : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-128"; }
      BlockCipher* clone() const { return new AES_128_SSSE3; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

/**
* AES-192 using SSSE3
*/
class BOTAN_DLL AES_192_SSSE3 : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-192"; }
      BlockCipher* clone() const { return new AES_192_SSSE3; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

/**
* AES-256 using SSSE3
*/
class BOTAN_DLL AES_256_SSSE3 : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "AES-256"; }
      BlockCipher* clone() const { return new AES_256_SSSE3; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK, DK;
   };

}

#endif
