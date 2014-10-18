/*
* ECDSA
* (C) 2007 Falko Strenzke, FlexSecure GmbH
*          Manuel Hartl, FlexSecure GmbH
* (C) 2008-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_ECDSA_KEY_H__
#define BOTAN_ECDSA_KEY_H__

#include <botan/ecc_key.h>
#include <botan/reducer.h>
#include <botan/pk_ops.h>

namespace Botan {

/**
* This class represents ECDSA Public Keys.
*/
class BOTAN_DLL ECDSA_PublicKey : public virtual EC_PublicKey
   {
   public:

      /**
      * Construct a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      ECDSA_PublicKey(const EC_Group& dom_par,
                      const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      ECDSA_PublicKey(const AlgorithmIdentifier& alg_id,
                      const secure_vector<byte>& key_bits) :
         EC_PublicKey(alg_id, key_bits) {}

      /**
      * Get this keys algorithm name.
      * @result this keys algorithm name ("ECDSA")
      */
      std::string algo_name() const { return "ECDSA"; }

      /**
      * Get the maximum number of bits allowed to be fed to this key.
      * This is the bitlength of the order of the base point.
      * @result the maximum number of input bits
      */
      size_t max_input_bits() const { return domain().get_order().bits(); }

      size_t message_parts() const { return 2; }

      size_t message_part_size() const
         { return domain().get_order().bytes(); }

   protected:
      ECDSA_PublicKey() {}
   };

/**
* This class represents ECDSA Private Keys
*/
class BOTAN_DLL ECDSA_PrivateKey : public ECDSA_PublicKey,
                                   public EC_PrivateKey
   {
   public:

      /**
      * Load a private key
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      ECDSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                       const secure_vector<byte>& key_bits) :
         EC_PrivateKey(alg_id, key_bits) {}

      /**
      * Generate a new private key
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key (if zero, generate a ney random key)
      */
      ECDSA_PrivateKey(RandomNumberGenerator& rng,
                       const EC_Group& domain,
                       const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x) {}

      bool check_key(RandomNumberGenerator& rng, bool) const;
   };

/**
* ECDSA signature operation
*/
class BOTAN_DLL ECDSA_Signature_Operation : public PK_Ops::Signature
   {
   public:
      ECDSA_Signature_Operation(const ECDSA_PrivateKey& ecdsa);

      secure_vector<byte> sign(const byte msg[], size_t msg_len,
                              RandomNumberGenerator& rng);

      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return order.bytes(); }
      size_t max_input_bits() const { return order.bits(); }

   private:
      const PointGFp& base_point;
      const BigInt& order;
      const BigInt& x;
      Modular_Reducer mod_order;
   };

/**
* ECDSA verification operation
*/
class BOTAN_DLL ECDSA_Verification_Operation : public PK_Ops::Verification
   {
   public:
      ECDSA_Verification_Operation(const ECDSA_PublicKey& ecdsa);

      size_t message_parts() const { return 2; }
      size_t message_part_size() const { return order.bytes(); }
      size_t max_input_bits() const { return order.bits(); }

      bool with_recovery() const { return false; }

      bool verify(const byte msg[], size_t msg_len,
                  const byte sig[], size_t sig_len);
   private:
      const PointGFp& base_point;
      const PointGFp& public_point;
      const BigInt& order;
   };

}

#endif
