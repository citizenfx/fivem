/*
* EMSA1
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
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
      void update(const byte[], size_t);
      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng);

      bool verify(const secure_vector<byte>& coded,
                  const secure_vector<byte>& raw,
                  size_t key_bits);

      std::unique_ptr<HashFunction> m_hash;
   };

}

#endif
