/*
* X9.31 EMSA
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_EMSA_X931_H__
#define BOTAN_EMSA_X931_H__

#include <botan/emsa.h>
#include <botan/hash.h>

namespace Botan {

/**
* EMSA from X9.31 (EMSA2 in IEEE 1363)
* Useful for Rabin-Williams, also sometimes used with RSA in
* odd protocols.
*/
class BOTAN_DLL EMSA_X931 : public EMSA
   {
   public:
      /**
      * @param hash the hash object to use
      */
      EMSA_X931(HashFunction* hash);
   private:
      void update(const byte[], size_t);
      secure_vector<byte> raw_data();

      secure_vector<byte> encoding_of(const secure_vector<byte>&, size_t,
                                     RandomNumberGenerator& rng);

      bool verify(const secure_vector<byte>&, const secure_vector<byte>&,
                  size_t);

      secure_vector<byte> m_empty_hash;
      std::unique_ptr<HashFunction> m_hash;
      byte m_hash_id;
   };

}

#endif
