/*
 * XMSS Parameters
 * (C) 2016 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#ifndef BOTAN_XMSS_PARAMETERS_H__
#define BOTAN_XMSS_PARAMETERS_H__

#include <cstddef>
#include <map>
#include <string>
#include <botan/assert.h>
#include <botan/types.h>
#include <botan/xmss_wots_parameters.h>

namespace Botan {

/**
 * Descibes a signature method for XMSS, as defined in:
 * [1] XMSS: Extended Hash-Based Signatures,
 *     draft-itrf-cfrg-xmss-hash-based-signatures-06
 *     Release: July 2016.
 *     https://datatracker.ietf.org/doc/
 *     draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class BOTAN_DLL XMSS_Parameters
   {
   public:
      enum xmss_algorithm_t
         {
         XMSS_SHA2_256_W16_H10 = 0x01000001,
         XMSS_SHA2_256_W16_H16 = 0x02000002,
         XMSS_SHA2_256_W16_H20 = 0x03000003,
         XMSS_SHA2_512_W16_H10 = 0x04000004,
         XMSS_SHA2_512_W16_H16 = 0x05000005,
         XMSS_SHA2_512_W16_H20 = 0x06000006,
         XMSS_SHAKE128_W16_H10 = 0x07000007,
         XMSS_SHAKE128_W16_H16 = 0x08000008,
         XMSS_SHAKE128_W16_H20 = 0x09000009,
         XMSS_SHAKE256_W16_H10 = 0x0a00000a,
         XMSS_SHAKE256_W16_H16 = 0x0b00000b,
         XMSS_SHAKE256_W16_H20 = 0x0c00000c
         };

      static xmss_algorithm_t xmss_id_from_string(const std::string& algo_name);

      XMSS_Parameters(const std::string& algo_name);
      XMSS_Parameters(xmss_algorithm_t oid);

      /**
       * @return XMSS registry name for the chosen parameter set.
       **/
      const std::string& name() const
         {
         return m_name;
         }

      const std::string& hash_function_name() const
         {
         return m_hash_name;
         }

      /**
       * Retrieves the uniform length of a message, and the size of
       * each node. This correlates to XMSS parameter "n" defined
       * in [1].
       *
       * @return element length in bytes.
       **/
      size_t element_size() const { return m_element_size; }

      /**
       * @returns The height (number of levels - 1) of the tree
       **/
      size_t tree_height() const { return m_tree_height; }

      /**
       * The Winternitz parameter.
       *
       * @return numeric base used for internal representation of
       *         data.
       **/
      size_t wots_parameter() const { return m_w; }

      size_t len() const { return m_len; }

      xmss_algorithm_t oid() const { return m_oid; }

      XMSS_WOTS_Parameters::ots_algorithm_t ots_oid() const
         {
         return m_wots_oid;
         }

      /**
       * Returns the estimated pre-quantum security level of
       * the chosen algorithm.
       **/
      size_t estimated_strength() const
         {
         return m_strength;
         }

      bool operator==(const XMSS_Parameters& p) const
         {
         return m_oid == p.m_oid;
         }

   private:
      xmss_algorithm_t m_oid;
      XMSS_WOTS_Parameters::ots_algorithm_t m_wots_oid;
      std::string m_name;
      std::string m_hash_name;
      size_t m_element_size;
      size_t m_tree_height;
      size_t m_w;
      size_t m_len;
      size_t m_strength;
   };

}

#endif
