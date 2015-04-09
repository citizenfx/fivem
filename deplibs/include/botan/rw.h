/*
* Rabin-Williams
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_RW_H__
#define BOTAN_RW_H__

#include <botan/if_algo.h>

namespace Botan {

/**
* Rabin-Williams Public Key
*/
class BOTAN_DLL RW_PublicKey : public virtual IF_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "RW"; }

      RW_PublicKey(const AlgorithmIdentifier& alg_id,
                   const secure_vector<byte>& key_bits) :
         IF_Scheme_PublicKey(alg_id, key_bits)
         {}

      RW_PublicKey(const BigInt& mod, const BigInt& exponent) :
         IF_Scheme_PublicKey(mod, exponent)
         {}

   protected:
      RW_PublicKey() {}
   };

/**
* Rabin-Williams Private Key
*/
class BOTAN_DLL RW_PrivateKey : public RW_PublicKey,
                                public IF_Scheme_PrivateKey
   {
   public:
      RW_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<byte>& key_bits,
                    RandomNumberGenerator& rng) :
         IF_Scheme_PrivateKey(rng, alg_id, key_bits) {}

      RW_PrivateKey(RandomNumberGenerator& rng,
                    const BigInt& p, const BigInt& q,
                    const BigInt& e, const BigInt& d = 0,
                    const BigInt& n = 0) :
         IF_Scheme_PrivateKey(rng, p, q, e, d, n) {}

      RW_PrivateKey(RandomNumberGenerator& rng, size_t bits, size_t = 2);

      bool check_key(RandomNumberGenerator& rng, bool) const;
   };

}

#endif
