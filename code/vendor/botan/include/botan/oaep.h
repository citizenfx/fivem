/*
* OAEP
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OAEP_H__
#define BOTAN_OAEP_H__

#include <botan/eme.h>
#include <botan/hash.h>

namespace Botan {

/**
* OAEP (called EME1 in IEEE 1363 and in earlier versions of the library)
*/
class BOTAN_DLL OAEP final : public EME
   {
   public:
      size_t maximum_input_size(size_t) const override;

      /**
      * @param hash function to use for hashing (takes ownership)
      * @param P an optional label. Normally empty.
      */
      OAEP(HashFunction* hash, const std::string& P = "");
   private:
      secure_vector<uint8_t> pad(const uint8_t in[],
                              size_t in_length,
                              size_t key_length,
                              RandomNumberGenerator& rng) const override;

      secure_vector<uint8_t> unpad(uint8_t& valid_mask,
                                const uint8_t in[],
                                size_t in_len) const override;

      secure_vector<uint8_t> m_Phash;
      std::unique_ptr<HashFunction> m_hash;
   };

}

#endif
