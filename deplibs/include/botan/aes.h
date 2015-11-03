/*
* AES
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AES_H__
#define BOTAN_AES_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* AES-128
*/
class BOTAN_DLL AES_128 : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-128"; }
      BlockCipher* clone() const override { return new AES_128; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

/**
* AES-192
*/
class BOTAN_DLL AES_192 : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-192"; }
      BlockCipher* clone() const override { return new AES_192; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

/**
* AES-256
*/
class BOTAN_DLL AES_256 : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override { return "AES-256"; }
      BlockCipher* clone() const override { return new AES_256; }
   private:
      void key_schedule(const byte key[], size_t length) override;

      secure_vector<u32bit> EK, DK;
      secure_vector<byte> ME, MD;
   };

}

#endif
