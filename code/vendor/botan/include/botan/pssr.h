/*
* PSSR
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PSSR_H_
#define BOTAN_PSSR_H_

#include <botan/emsa.h>
#include <botan/hash.h>

namespace Botan {

/**
* PSSR (called EMSA4 in IEEE 1363 and in old versions of the library)
*/
class BOTAN_PUBLIC_API(2,0) PSSR final : public EMSA
   {
   public:

      /**
      * @param hash the hash function to use
      */
      explicit PSSR(HashFunction* hash);

      /**
      * @param hash the hash function to use
      * @param salt_size the size of the salt to use in bytes
      */
      PSSR(HashFunction* hash, size_t salt_size);

      EMSA* clone() override { return new PSSR(m_hash->clone(), m_SALT_SIZE); }
   private:
      void update(const uint8_t input[], size_t length) override;

      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>& coded,
                  const secure_vector<uint8_t>& raw,
                  size_t key_bits) override;

      std::unique_ptr<HashFunction> m_hash;
      size_t m_SALT_SIZE;
   };

/**
* PSSR_Raw
* This accepts a pre-hashed buffer
*/
class BOTAN_DLL PSSR_Raw final : public EMSA
   {
   public:

      /**
      * @param hash the hash function to use
      */
      explicit PSSR_Raw(HashFunction* hash);

      /**
      * @param hash the hash function to use
      * @param salt_size the size of the salt to use in bytes
      */
      PSSR_Raw(HashFunction* hash, size_t salt_size);

      EMSA* clone() override { return new PSSR_Raw(m_hash->clone(), m_SALT_SIZE); }
   private:
      void update(const uint8_t input[], size_t length) override;

      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>& msg,
                                         size_t output_bits,
                                         RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>& coded,
                  const secure_vector<uint8_t>& raw,
                  size_t key_bits) override;

      std::unique_ptr<HashFunction> m_hash;
      size_t m_SALT_SIZE;
      secure_vector<uint8_t> m_msg;
   };

}

#endif
