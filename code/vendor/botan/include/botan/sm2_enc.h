/*
* SM2 Encryption
* (C) 2017 Ribose Inc
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SM2_ENC_KEY_H_
#define BOTAN_SM2_ENC_KEY_H_

#include <botan/ecc_key.h>

namespace Botan {

/**
* This class represents a public key used for SM2 encryption
*/
class BOTAN_PUBLIC_API(2,2) SM2_Encryption_PublicKey : public virtual EC_PublicKey
   {
   public:

      /**
      * Create a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      SM2_Encryption_PublicKey(const EC_Group& dom_par,
                               const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      SM2_Encryption_PublicKey(const AlgorithmIdentifier& alg_id,
                              const std::vector<uint8_t>& key_bits) :
         EC_PublicKey(alg_id, key_bits) {}

      /**
      * Get this keys algorithm name.
      * @result this keys algorithm name
      */
      std::string algo_name() const override { return "SM2_Enc"; }

      std::unique_ptr<PK_Ops::Encryption>
         create_encryption_op(RandomNumberGenerator& rng,
                              const std::string& params,
                              const std::string& provider) const override;
   protected:
      SM2_Encryption_PublicKey() = default;
   };

/**
* This class represents a private key used for SM2 encryption
*/
class BOTAN_PUBLIC_API(2,2) SM2_Encryption_PrivateKey final :
   public SM2_Encryption_PublicKey, public EC_PrivateKey
   {
   public:

      /**
      * Load a private key
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits ECPrivateKey bits
      */
      SM2_Encryption_PrivateKey(const AlgorithmIdentifier& alg_id,
                                const secure_vector<uint8_t>& key_bits);

      /**
      * Create a private key.
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key (if zero, generate a new random key)
      */
      SM2_Encryption_PrivateKey(RandomNumberGenerator& rng,
                                const EC_Group& domain,
                                const BigInt& x = 0);

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      std::unique_ptr<PK_Ops::Decryption>
         create_decryption_op(RandomNumberGenerator& rng,
                              const std::string& params,
                              const std::string& provider) const override;
   };

}

#endif
