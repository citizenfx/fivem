/*
* Threefish
* (C) 2013,2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_THREEFISH_H__
#define BOTAN_THREEFISH_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* Threefish-512
*/
class BOTAN_DLL Threefish_512 : public Block_Cipher_Fixed_Params<64, 64>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void set_tweak(const byte tweak[], size_t len);

      void clear() override;
      std::string name() const override { return "Threefish-512"; }
      BlockCipher* clone() const override { return new Threefish_512; }

      Threefish_512() : m_T(3) {}

   protected:
      const secure_vector<u64bit>& get_T() const { return m_T; }
      const secure_vector<u64bit>& get_K() const { return m_K; }
   private:
      void key_schedule(const byte key[], size_t key_len) override;

      // Interface for Skein
      friend class Skein_512;

      virtual void skein_feedfwd(const secure_vector<u64bit>& M,
                                 const secure_vector<u64bit>& T);

      // Private data
      secure_vector<u64bit> m_T;
      secure_vector<u64bit> m_K;
   };

}

#endif
