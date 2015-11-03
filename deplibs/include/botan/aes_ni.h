/*
* AES using AES-NI instructions
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AES_NI_H__
#define BOTAN_AES_NI_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* AES-128 using AES-NI
*/
class BOTAN_DLL AES_128_NI : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      size_t parallelism() const override { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "AES-128"; }
      BlockCipher* clone() const override { return new AES_128_NI; }
   private:
      void key_schedule(const byte[], size_t) override;

      secure_vector<u32bit> EK, DK;
   };

/**
* AES-192 using AES-NI
*/
class BOTAN_DLL AES_192_NI : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      size_t parallelism() const override { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "AES-192"; }
      BlockCipher* clone() const override { return new AES_192_NI; }
   private:
      void key_schedule(const byte[], size_t) override;

      secure_vector<u32bit> EK, DK;
   };

/**
* AES-256 using AES-NI
*/
class BOTAN_DLL AES_256_NI : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      size_t parallelism() const override { return 4; }

      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "AES-256"; }
      BlockCipher* clone() const override { return new AES_256_NI; }
   private:
      void key_schedule(const byte[], size_t) override;

      secure_vector<u32bit> EK, DK;
   };

}

#endif
