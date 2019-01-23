/*
 * XMSS_PrivateKey.h
 * (C) 2016,2017 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#ifndef BOTAN_XMSS_PRIVATEKEY_H_
#define BOTAN_XMSS_PRIVATEKEY_H_

#include <cstddef>
#include <iterator>
#include <memory>
#include <botan/alg_id.h>
#include <botan/exceptn.h>
#include <botan/pk_keys.h>
#include <botan/types.h>
#include <botan/xmss_parameters.h>
#include <botan/xmss_publickey.h>
#include <botan/atomic.h>
#include <botan/xmss_common_ops.h>
#include <botan/xmss_wots_privatekey.h>
#include <botan/xmss_index_registry.h>

namespace Botan {

/**
 * An XMSS: Extended Hash-Based Signature private key.
 * The XMSS private key does not support the X509 and PKCS7 standard. Instead
 * the raw format described in [1] is used.
 *
 *   [1] XMSS: Extended Hash-Based Signatures,
 *       draft-itrf-cfrg-xmss-hash-based-signatures-06
 *       Release: July 2016.
 *       https://datatracker.ietf.org/doc/
 *       draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class BOTAN_PUBLIC_API(2,0) XMSS_PrivateKey final : public virtual XMSS_PublicKey,
   public XMSS_Common_Ops,
   public virtual Private_Key
   {
   public:
      /**
      * Creates a new XMSS private key for the chosen XMSS signature method.
      * New seeds for public/private key and pseudo random function input are
      * generated using the provided RNG. The appropriate WOTS signature method
      * will be automatically set based on the chosen XMSS signature method.
      *
      * @param xmss_algo_id Identifier for the selected XMSS signature method.
      * @param rng A random number generator to use for key generation.
      **/
      XMSS_PrivateKey(XMSS_Parameters::xmss_algorithm_t xmss_algo_id,
                      RandomNumberGenerator& rng);

      /**
       * Creates an XMSS_PrivateKey from a byte sequence produced by
       * raw_private_key().
       *
       * @param raw_key An XMSS private key serialized using raw_private_key().
       **/
      XMSS_PrivateKey(const secure_vector<uint8_t>& raw_key);

      /**
       * Creates a new XMSS private key for the chosen XMSS signature method
       * using precomputed seeds for public/private keys and pseudo random
       * function input. The appropriate WOTS signature method will be
       * automatically set, based on the chosen XMSS signature method.
       *
       * @param xmss_algo_id Identifier for the selected XMSS signature method.
       * @param idx_leaf Index of the next unused leaf.
       * @param wots_priv_seed A seed to generate a Winternitz-One-Time-
       *                      Signature private key from.
       * @param prf a secret n-byte key sourced from a secure source
       *        of uniformly random data.
       * @param root Root node of the binary hash tree.
       * @param public_seed The public seed.
       **/
      XMSS_PrivateKey(XMSS_Parameters::xmss_algorithm_t xmss_algo_id,
                      size_t idx_leaf,
                      const secure_vector<uint8_t>& wots_priv_seed,
                      const secure_vector<uint8_t>& prf,
                      const secure_vector<uint8_t>& root,
                      const secure_vector<uint8_t>& public_seed)
         : XMSS_PublicKey(xmss_algo_id, root, public_seed),
           XMSS_Common_Ops(xmss_algo_id),
           m_wots_priv_key(XMSS_PublicKey::m_xmss_params.ots_oid(),
                           public_seed,
                           wots_priv_seed),
           m_prf(prf),
           m_index_reg(XMSS_Index_Registry::get_instance())
         {
         set_unused_leaf_index(idx_leaf);
         }

      /**
       * Retrieves the last unused leaf index of the private key. Reusing a leaf
       * by utilizing leaf indices lower than the last unused leaf index will
       * compromise security.
       *
       * @return Index of the last unused leaf.
       **/
      size_t unused_leaf_index() const
         {
         return *recover_global_leaf_index();
         }

      /**
       * Sets the last unused leaf index of the private key. The leaf index
       * will be updated automatically during every signing operation, and
       * should not be set manually.
       *
       * @param idx Index of the last unused leaf.
       **/
      void set_unused_leaf_index(size_t idx)
         {
         if(idx >= (1ull << XMSS_PublicKey::m_xmss_params.tree_height()))
            {
            throw Integrity_Failure("XMSS private key leaf index out of "
                                    "bounds.");
            }
         else
            {
            std::atomic<size_t>& index =
               static_cast<std::atomic<size_t>&>(*recover_global_leaf_index());
            size_t current = 0;

            do
               {
               current = index.load();
               if(current > idx)
                  { return; }
               }
            while(!index.compare_exchange_strong(current, idx));
            }
         }

      size_t reserve_unused_leaf_index()
         {
         size_t idx = (static_cast<std::atomic<size_t>&>(
                          *recover_global_leaf_index())).fetch_add(1);
         if(idx >= (1ull << XMSS_PublicKey::m_xmss_params.tree_height()))
            {
            throw Integrity_Failure("XMSS private key, one time signatures "
                                    "exhausted.");
            }
         return idx;
         }

      /**
       * Winternitz One Time Signature Scheme key utilized for signing
       * operations.
       *
       * @return WOTS+ private key.
       **/
      const XMSS_WOTS_PrivateKey& wots_private_key() const
         {
         return m_wots_priv_key;
         }

      /**
       * Winternitz One Time Signature Scheme key utilized for signing
       * operations.
       *
       * @return WOTS+ private key.
       **/
      XMSS_WOTS_PrivateKey& wots_private_key()
         {
         return m_wots_priv_key;
         }

      const secure_vector<uint8_t>& prf() const
         {
         return m_prf;
         }

      secure_vector<uint8_t>& prf()
         {
         return m_prf;
         }

      void set_public_seed(
         const secure_vector<uint8_t>& public_seed) override
         {
         m_public_seed = public_seed;
         m_wots_priv_key.set_public_seed(public_seed);
         }

      void set_public_seed(secure_vector<uint8_t>&& public_seed) override
         {
         m_public_seed = std::move(public_seed);
         m_wots_priv_key.set_public_seed(m_public_seed);
         }

      const secure_vector<uint8_t>& public_seed() const override
         {
         return m_public_seed;
         }

      std::unique_ptr<PK_Ops::Signature>
      create_signature_op(RandomNumberGenerator&,
                          const std::string&,
                          const std::string& provider) const override;

      secure_vector<uint8_t> private_key_bits() const override
         {
         return raw_private_key();
         }

      size_t size() const override
         {
         return XMSS_PublicKey::size() +
                sizeof(uint64_t) +
                2 * XMSS_PublicKey::m_xmss_params.element_size();
         }

      /**
       * Generates a non standartized byte sequence representing the XMSS
       * private key.
       *
       * @return byte sequence consisting of the following elements in order:
       *         4-byte OID, n-byte root node, n-byte public seed,
       *         8-byte unused leaf index, n-byte prf seed, n-byte private seed.
       **/
      secure_vector<uint8_t> raw_private_key() const;
      /**
       * Algorithm 9: "treeHash"
       * Computes the internal n-byte nodes of a Merkle tree.
       *
       * @param start_idx The start index.
       * @param target_node_height Height of the target node.
       * @param adrs Address of the tree containing the target node.
       *
       * @return The root node of a tree of height target_node height with the
       *         leftmost leaf being the hash of the WOTS+ pk with index
       *         start_idx.
       **/
      secure_vector<uint8_t> tree_hash(
         size_t start_idx,
         size_t target_node_height,
         XMSS_Address& adrs);

   private:
      /**
       * Fetches shared unused leaf index from the index registry
       **/
      std::shared_ptr<Atomic<size_t>> recover_global_leaf_index() const;

      inline void tree_hash_subtree(secure_vector<uint8_t>& result,
                                    size_t start_idx,
                                    size_t target_node_height,
                                    XMSS_Address& adrs)
         {
         return tree_hash_subtree(result, start_idx, target_node_height, adrs, m_hash);
         }


      /**
       * Helper for multithreaded tree hashing.
       */
      void tree_hash_subtree(secure_vector<uint8_t>& result,
                             size_t start_idx,
                             size_t target_node_height,
                             XMSS_Address& adrs,
                             XMSS_Hash& hash);

      XMSS_WOTS_PrivateKey m_wots_priv_key;
      secure_vector<uint8_t> m_prf;
      XMSS_Index_Registry& m_index_reg;
   };

}

#endif

