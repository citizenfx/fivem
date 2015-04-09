/*
* Camellia
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CAMELLIA_H__
#define BOTAN_CAMELLIA_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* Camellia-128
*/
class BOTAN_DLL Camellia_128 : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Camellia-128"; }
      BlockCipher* clone() const { return new Camellia_128; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u64bit> SK;
   };

/**
* Camellia-192
*/
class BOTAN_DLL Camellia_192 : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Camellia-192"; }
      BlockCipher* clone() const { return new Camellia_192; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u64bit> SK;
   };

/**
* Camellia-256
*/
class BOTAN_DLL Camellia_256 : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Camellia-256"; }
      BlockCipher* clone() const { return new Camellia_256; }
   private:
      void key_schedule(const byte key[], size_t length);

      secure_vector<u64bit> SK;
   };

}

#endif
