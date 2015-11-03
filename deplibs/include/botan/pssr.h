/*
* PSSR
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PSSR_H__
#define BOTAN_PSSR_H__

#include <botan/emsa.h>
#include <botan/hash.h>

namespace Botan {

/**
* PSSR (called EMSA4 in IEEE 1363 and in old versions of the library)
*/
class BOTAN_DLL PSSR : public EMSA
   {
   public:

      /**
      * @param hash the hash object to use
      */
      PSSR(HashFunction* hash);

      /**
      * @param hash the hash object to use
      * @param salt_size the size of the salt to use in bytes
      */
      PSSR(HashFunction* hash, size_t salt_size);

      static PSSR* make(const Spec& spec);
   private:
      void update(const byte input[], size_t length) override;

      secure_vector<byte> raw_data() override;

      secure_vector<byte> encoding_of(const secure_vector<byte>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<byte>& coded,
                  const secure_vector<byte>& raw,
                  size_t key_bits) override;

      size_t SALT_SIZE;
      std::unique_ptr<HashFunction> hash;
   };

}

#endif
