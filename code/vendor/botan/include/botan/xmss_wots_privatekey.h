/*
 * XMSS WOTS Private Key
 * (C) 2016 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#ifndef BOTAN_XMSS_WOTS_PRIVATEKEY_H__
#define BOTAN_XMSS_WOTS_PRIVATEKEY_H__

#include <cstddef>
#include <memory>
#include <botan/alg_id.h>
#include <botan/assert.h>
#include <botan/exceptn.h>
#include <botan/pk_keys.h>
#include <botan/types.h>
#include <botan/xmss_wots_parameters.h>
#include <botan/xmss_address.h>
#include <botan/xmss_wots_publickey.h>

namespace Botan {

/** A Winternitz One Time Signature private key for use with Extended Hash-Based
 * Signatures.
 **/
class BOTAN_DLL XMSS_WOTS_PrivateKey : public virtual XMSS_WOTS_PublicKey,
                                       public virtual Private_Key
   {
   public:
      /**
       * Creates a WOTS private key for the chosen XMSS WOTS signature method.
       * Members need to be initialized manually.
       *
       * @param oid Identifier for the selected signature method.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid)
         : XMSS_WOTS_PublicKey(oid)
         {}

      /**
       * Creates a WOTS private key for the chosen XMSS WOTS signature method.
       *
       * @param oid Identifier for the selected signature method.
       * @param rng A random number generator to use for key generation.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                           RandomNumberGenerator& rng)
         : XMSS_WOTS_PublicKey(oid, rng),
           m_private_seed(rng.random_vec(m_wots_params.element_size()))
         {
         set_key_data(generate(m_private_seed));
         }

      /**
       * Constructs a WOTS private key. Chains will be generated on demand
       * applying a hash function to a unique value generated from a secret
       * seed and a counter. The secret seed of length n, will be
       * automatically generated using AutoSeeded_RNG(). "n" equals
       * the element size of the chosen WOTS security parameter set.
       *
       * @param oid Identifier for the selected signature method.
       * @param public_seed A public seed used for the pseudo random generation
       *        of public keys derived from this private key.
       * @param rng A random number generator to use for key generation.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                           const secure_vector<uint8_t>& public_seed,
                           RandomNumberGenerator &rng)
         : XMSS_WOTS_PublicKey(oid, public_seed),
           m_private_seed(rng.random_vec(m_wots_params.element_size()))
         {
         set_key_data(generate(m_private_seed));
         }

      /**
       * Constructs a WOTS private key. Chains will be generated on demand
       * applying a hash function to a unique value generated from a secret
       * seed and a counter. The secret seed of length n, will be
       * automatically generated using AutoSeeded_RNG(). "n" equals
       * the element size of the chosen WOTS security parameter set.
       *
       * @param oid Identifier for the selected signature method.
       * @param public_seed A public seed used for the pseudo random generation
       *        of public keys derived from this private key.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                           const secure_vector<uint8_t>& public_seed)
         : XMSS_WOTS_PublicKey(oid, public_seed)
         {}

      /**
       * Constructs a WOTS private key. Chains will be generated on demand
       * applying a hash function to a unique value generated from the
       * secret seed and a counter.
       *
       * @param oid Identifier for the selected signature method.
       * @param public_seed A public seed used for the pseudo random generation
       *        of public keys derived from this private key.
       * @param private_seed A secret uniformly random n-byte value.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                           const secure_vector<uint8_t>& public_seed,
                           const secure_vector<uint8_t>& private_seed)
         : XMSS_WOTS_PublicKey(oid, public_seed),
           m_private_seed(private_seed)
         {
         set_key_data(generate(private_seed));
         }

      /**
       * Retrieves the i-th WOTS private key using pseudo random key
       * (re-)generation.
       *
       * @param i Index of the key to retrieve.
       *
       * @return WOTS secret key.
       **/
      wots_keysig_t operator[](size_t i)
         {
         secure_vector<uint8_t> idx_bytes;
         XMSS_Tools::concat(idx_bytes, i, m_wots_params.element_size());
         m_hash.h(idx_bytes, m_private_seed, idx_bytes);
         return generate(idx_bytes);
         }

      /**
       * Retrieves the i-th WOTS private key using pseudo random key
       * (re-)generation.
       *
       * @param adrs The address of the key to retrieve.
       *
       * @return WOTS secret key.
       **/
      wots_keysig_t operator[](const XMSS_Address& adrs)
         {
         secure_vector<uint8_t> result;
         m_hash.prf(result, m_private_seed, adrs.bytes());
         return generate(result);
         }

      wots_keysig_t generate_private_key(const secure_vector<uint8_t>& priv_seed);

      /**
       * Algorithm 4: "WOTS_genPK"
       * Generates a Winternitz One Time Signature+ (WOTS+) Public Key from a
       * given private key.
       *
       * @param adrs Hash function address encoding the address of the WOTS+
       *             key pair within a greater structure.
       *
       * @return A XMSS_WOTS_PublicKey.
       **/
      XMSS_WOTS_PublicKey generate_public_key(XMSS_Address& adrs);

      /**
       * Algorithm 4: "WOTS_genPK"
       * Initializes a Winternitz One Time Signature+ (WOTS+) Public Key's
       * key_data() member, with data derived from in_key_data using the
       * WOTS chaining function.
       *
       * @param[out] pub_key Public key to initialize key_data() member on.
       * @param in_key_data Input key material from private key used for
       *        public key generation.
       * @param adrs Hash function address encoding the address of
       *        the WOTS+ key pair within a greater structure.
       **/
      void generate_public_key(XMSS_WOTS_PublicKey& pub_key,
                               wots_keysig_t&& in_key_data,
                               XMSS_Address& adrs);

      /**
       * Algorithm 5: "WOTS_sign"
       * Generates a signature from a private key and a message.
       *
       * @param msg A message to sign.
       * @param adrs An OTS hash address identifying the WOTS+ key pair
       *        used for signing.
       *
       * @return signature for msg.
       **/
      wots_keysig_t sign(const secure_vector<uint8_t>& msg,
                         XMSS_Address& adrs);

      /**
       * Retrieves the secret seed used to generate WOTS+ chains. The seed
       * should be a uniformly random n-byte value.
       *
       * @return secret seed.
       **/
      const secure_vector<uint8_t>& private_seed() const
         {
         return m_private_seed;
         }

      /**
       * Sets the secret seed used to generate WOTS+ chains. The seed
       * should be a uniformly random n-byte value.
       *
       * @param private_seed Uniformly random n-byte value.
       **/
      void set_private_seed(const secure_vector<uint8_t>& private_seed)
         {
         m_private_seed = private_seed;
         }

      /**
       * Sets the secret seed used to generate WOTS+ chains. The seed
       * should be a uniformly random n-byte value.
       *
       * @param private_seed Uniformly random n-byte value.
       **/
      void set_private_seed(secure_vector<uint8_t>&& private_seed)
         {
         m_private_seed = std::move(private_seed);
         }

      virtual AlgorithmIdentifier
      pkcs8_algorithm_identifier() const override
         {
         throw Not_Implemented("No AlgorithmIdentifier available for XMSS-WOTS.");
         }

      virtual std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator&,
                             const std::string&,
                             const std::string& provider) const override;

      virtual secure_vector<uint8_t> private_key_bits() const override
         {
         throw Not_Implemented("No PKCS8 key format defined for XMSS-WOTS.");
         }

   private:
      /**
       * Algorithm 3: "Generating a WOTS+ Private Key".
       * Generates a private key.
       *
       * @param private_seed Uniformly random n-byte value.
       *
       * @returns a vector of length key_size() of vectors of n bytes length
       *          containing uniformly random data.
       **/
      wots_keysig_t generate(const secure_vector<uint8_t>& private_seed);

      secure_vector<uint8_t> m_private_seed;
   };

}

#endif

