/*
* (C) 2013 Jack Lloyd
* (C) 2016 Daniel Neus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_GCM_GHASH_H_
#define BOTAN_GCM_GHASH_H_

#include <botan/sym_algo.h>

namespace Botan {

/**
* GCM's GHASH
* This is not intended for general use, but is exposed to allow
* shared code between GCM and GMAC
*/
class BOTAN_PUBLIC_API(2,0) GHASH final : public SymmetricAlgorithm
   {
   public:
      void set_associated_data(const uint8_t ad[], size_t ad_len);

      secure_vector<uint8_t> nonce_hash(const uint8_t nonce[], size_t len);

      void start(const uint8_t nonce[], size_t len);

      /*
      * Assumes input len is multiple of 16
      */
      void update(const uint8_t in[], size_t len);

      /*
      * Incremental update of associated data
      */
      void update_associated_data(const uint8_t ad[], size_t len);

      secure_vector<uint8_t> final();

      Key_Length_Specification key_spec() const override
         { return Key_Length_Specification(16); }

      void clear() override;

      void reset();

      std::string name() const override { return "GHASH"; }

      std::string provider() const;

      void ghash_update(secure_vector<uint8_t>& x,
                        const uint8_t input[], size_t input_len);

      void add_final_block(secure_vector<uint8_t>& x,
                           size_t ad_len, size_t pt_len);
   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      void gcm_multiply(secure_vector<uint8_t>& x,
                        const uint8_t input[],
                        size_t blocks);

      static const size_t GCM_BS = 16;

      secure_vector<uint8_t> m_H;
      secure_vector<uint8_t> m_H_ad;
      secure_vector<uint8_t> m_ghash;
      secure_vector<uint8_t> m_nonce;
      secure_vector<uint64_t> m_HM;
      secure_vector<uint64_t> m_H_pow;
      size_t m_ad_len = 0;
      size_t m_text_len = 0;
   };

}

#endif
