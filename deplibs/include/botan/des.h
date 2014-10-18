/*
* DES
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_DES_H__
#define BOTAN_DES_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* DES
*/
class BOTAN_DLL DES : public Block_Cipher_Fixed_Params<8, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "DES"; }
      BlockCipher* clone() const { return new DES; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> round_key;
   };

/**
* Triple DES
*/
class BOTAN_DLL TripleDES : public Block_Cipher_Fixed_Params<8, 16, 24, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "TripleDES"; }
      BlockCipher* clone() const { return new TripleDES; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> round_key;
   };

/*
* DES Tables
*/
extern const u32bit DES_SPBOX1[256];
extern const u32bit DES_SPBOX2[256];
extern const u32bit DES_SPBOX3[256];
extern const u32bit DES_SPBOX4[256];
extern const u32bit DES_SPBOX5[256];
extern const u32bit DES_SPBOX6[256];
extern const u32bit DES_SPBOX7[256];
extern const u32bit DES_SPBOX8[256];

extern const u64bit DES_IPTAB1[256];
extern const u64bit DES_IPTAB2[256];
extern const u64bit DES_FPTAB1[256];
extern const u64bit DES_FPTAB2[256];

}

#endif
