/*
* DES
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
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
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "DES"; }
      BlockCipher* clone() const override { return new DES; }
   private:
      void key_schedule(const byte[], size_t) override;

      secure_vector<u32bit> round_key;
   };

/**
* Triple DES
*/
class BOTAN_DLL TripleDES : public Block_Cipher_Fixed_Params<8, 16, 24, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "TripleDES"; }
      BlockCipher* clone() const override { return new TripleDES; }
   private:
      void key_schedule(const byte[], size_t) override;

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
