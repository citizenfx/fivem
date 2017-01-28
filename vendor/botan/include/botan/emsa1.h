/*
* EMSA1
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_EMSA1_H__
#define BOTAN_EMSA1_H__

#include <botan/emsa.h>
#include <botan/hash.h>

namespace Botan {

/**
* EMSA1 from IEEE 1363
* Essentially, sign the hash directly
*/
class BOTAN_DLL EMSA1 : public EMSA
   {
   public:
      /**
      * @param hash the hash function to use
      */
      explicit EMSA1(HashFunction* hash) : m_hash(hash) {}

      EMSA* clone() override;

   protected:
      size_t hash_output_length() const { return m_hash->output_length(); }

      std::unique_ptr<HashFunction> m_hash;

   private:
      void update(const uint8_t[], size_t) override;
      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>& coded,
                  const secure_vector<uint8_t>& raw,
                  size_t key_bits) override;

   };

}

#endif
