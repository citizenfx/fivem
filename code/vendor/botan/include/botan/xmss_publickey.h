/*
 * XMSS Public Key
 * (C) 2016,2017 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#ifndef BOTAN_XMSS_PUBLICKEY_H_
#define BOTAN_XMSS_PUBLICKEY_H_

#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <botan/alg_id.h>
#include <botan/asn1_oid.h>
#include <botan/der_enc.h>
#include <botan/exceptn.h>
#include <botan/rng.h>
#include <botan/types.h>
#include <botan/pk_keys.h>
#include <botan/xmss_parameters.h>
#include <botan/xmss_wots_parameters.h>
#include <botan/pk_ops.h>

namespace Botan {

class XMSS_Verification_Operation;

/**
 * An XMSS: Extended Hash-Based Signature public key.
 * The XMSS public key does not support the X509 standard. Instead the
 * raw format described in [1] is used.
 *
 *   [1] XMSS: Extended Hash-Based Signatures,
 *       draft-itrf-cfrg-xmss-hash-based-signatures-06
 *       Release: July 2016.
 *       https://datatracker.ietf.org/doc/
 *       draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class BOTAN_PUBLIC_API(2,0) XMSS_PublicKey : public virtual Public_Key
   {
   public:
      /**
       * Creates a new XMSS public key for the chosen XMSS signature method.
       * New public and prf seeds are generated using rng. The appropriate WOTS
       * signature method will be automatically set based on the chosen XMSS
       * signature method.
       *
       * @param xmss_oid Identifier for the selected XMSS signature method.
       * @param rng A random number generator to use for key generation.
       **/
      XMSS_PublicKey(XMSS_Parameters::xmss_algorithm_t xmss_oid,
                     RandomNumberGenerator& rng)
         : m_xmss_params(xmss_oid), m_wots_params(m_xmss_params.ots_oid()),
           m_root(m_xmss_params.element_size()),
           m_public_seed(rng.random_vec(m_xmss_params.element_size())) {}

      /**
       * Creates an XMSS public key from a byte sequence produced by
       * raw_private_key().
       **/
      XMSS_PublicKey(const std::vector<uint8_t>& raw_key);

      /**
       * Creates a new XMSS public key for a chosen XMSS signature method as
       * well as pre-computed root node and public_seed values.
       *
       * @param xmss_oid Identifier for the selected XMSS signature method.
       * @param root Root node value.
       * @param public_seed Public seed value.
       **/
      XMSS_PublicKey(XMSS_Parameters::xmss_algorithm_t xmss_oid,
                     const secure_vector<uint8_t>& root,
                     const secure_vector<uint8_t>& public_seed)
         : m_xmss_params(xmss_oid), m_wots_params(m_xmss_params.ots_oid()),
           m_root(root), m_public_seed(public_seed) {}

      /**
       * Creates a new XMSS public key for a chosen XMSS signature method as
       * well as pre-computed root node and public_seed values.
       *
       * @param xmss_oid Identifier for the selected XMSS signature method.
       * @param root Root node value.
       * @param public_seed Public seed value.
       **/
      XMSS_PublicKey(XMSS_Parameters::xmss_algorithm_t xmss_oid,
                     secure_vector<uint8_t>&& root,
                     secure_vector<uint8_t>&& public_seed)
         : m_xmss_params(xmss_oid), m_wots_params(m_xmss_params.ots_oid()),
           m_root(std::move(root)), m_public_seed(std::move(public_seed)) {}

      /**
       * Retrieves the chosen XMSS signature method.
       *
       * @return XMSS signature method identifier.
       **/
      XMSS_Parameters::xmss_algorithm_t xmss_oid() const
         {
         return m_xmss_params.oid();
         }

      /**
       * Sets the chosen XMSS signature method
       **/
      void set_xmss_oid(XMSS_Parameters::xmss_algorithm_t xmss_oid)
         {
         m_xmss_params = XMSS_Parameters(xmss_oid);
         m_wots_params = XMSS_WOTS_Parameters(m_xmss_params.ots_oid());
         }

      /**
       * Retrieves the XMSS parameters determined by the chosen XMSS Signature
       * method.
       *
       * @return XMSS parameters.
       **/
      const XMSS_Parameters& xmss_parameters() const
         {
         return m_xmss_params;
         }

      /**
       * Retrieves the Winternitz One Time Signature (WOTS) method,
       * corrseponding to the chosen XMSS signature method.
       *
       * @return XMSS WOTS signature method identifier.
       **/
      XMSS_WOTS_Parameters::ots_algorithm_t wots_oid() const
         {
         return m_wots_params.oid();
         }

      /**
       * Retrieves the Winternitz One Time Signature (WOTS) parameters
       * corresponding to the chosen XMSS signature method.
       *
       * @return XMSS WOTS signature method parameters.
       **/
      const XMSS_WOTS_Parameters& wots_parameters() const
         {
         return m_wots_params;
         }

      secure_vector<uint8_t>& root()
         {
         return m_root;
         }

      void set_root(const secure_vector<uint8_t>& root)
         {
         m_root = root;
         }

      void set_root(secure_vector<uint8_t>&& root)
         {
         m_root = std::move(root);
         }

      const secure_vector<uint8_t>& root() const
         {
         return m_root;
         }

      virtual secure_vector<uint8_t>& public_seed()
         {
         return m_public_seed;
         }

      virtual void set_public_seed(const secure_vector<uint8_t>& public_seed)
         {
         m_public_seed = public_seed;
         }

      virtual void set_public_seed(secure_vector<uint8_t>&& public_seed)
         {
         m_public_seed = std::move(public_seed);
         }

      virtual const secure_vector<uint8_t>& public_seed() const
         {
         return m_public_seed;
         }

      std::string algo_name() const override
         {
         return "XMSS";
         }

      AlgorithmIdentifier algorithm_identifier() const override
         {
         return AlgorithmIdentifier(get_oid(), AlgorithmIdentifier::USE_NULL_PARAM);
         }

      bool check_key(RandomNumberGenerator&, bool) const override
         {
         return true;
         }

      std::unique_ptr<PK_Ops::Verification>
      create_verification_op(const std::string&,
                             const std::string& provider) const override;

      size_t estimated_strength() const override
         {
         return m_xmss_params.estimated_strength();
         }

      size_t key_length() const override
         {
         return m_xmss_params.estimated_strength();
         }

      /**
       * Returns a raw byte sequence as defined in [1].
       * This method acts as an alias for raw_public_key().
       *
       * @return raw public key bits.
       **/
      std::vector<uint8_t> public_key_bits() const override
         {
         return raw_public_key();
         }

      /**
       * Size in bytes of the serialized XMSS public key produced by
       * raw_public_key().
       *
       * @return size in bytes of serialized Public Key.
       **/
      virtual size_t size() const
         {
         return sizeof(uint32_t) + 2 * m_xmss_params.element_size();
         }

      /**
       * Generates a non standardized byte sequence representing the XMSS
       * public key, as defined in [1] (p. 23, "XMSS Public Key")
       *
       * @return 4-byte OID, followed by n-byte root node, followed by
       *         public seed.
       **/
      virtual std::vector<uint8_t> raw_public_key() const;

   protected:
      XMSS_Parameters m_xmss_params;
      XMSS_WOTS_Parameters m_wots_params;
      secure_vector<uint8_t> m_root;
      secure_vector<uint8_t> m_public_seed;

   private:
      XMSS_Parameters::xmss_algorithm_t deserialize_xmss_oid(
         const std::vector<uint8_t>& raw_key);
   };

}

#endif
