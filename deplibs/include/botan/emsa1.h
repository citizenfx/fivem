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
      EMSA1(HashFunction* hash) : m_hash(hash) {}

   protected:
      size_t hash_output_length() const { return m_hash->output_length(); }
   private:
      void update(const byte[], size_t) override;
      secure_vector<byte> raw_data() override;

      secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<byte>& coded,
                  const secure_vector<byte>& raw,
                  size_t key_bits) override;

      std::unique_ptr<HashFunction> m_hash;
   };

}

#endif
