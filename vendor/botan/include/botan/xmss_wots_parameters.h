/*
 * XMSS WOTS Parameters
 * (C) 2016 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#ifndef BOTAN_XMSS_WOTS_PARAMETERS_H__
#define BOTAN_XMSS_WOTS_PARAMETERS_H__

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <string>
#include <math.h>
#include <botan/assert.h>
#include <botan/types.h>
#include <botan/secmem.h>
#include <botan/exceptn.h>
#include <botan/xmss_tools.h>

namespace Botan {

/**
 * Descibes a signature method for XMSS Winternitz One Time Signatures,
 * as defined in:
 * [1] XMSS: Extended Hash-Based Signatures,
 *     draft-itrf-cfrg-xmss-hash-based-signatures-06
 *     Release: July 2016.
 *     https://datatracker.ietf.org/doc/
 *     draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class XMSS_WOTS_Parameters
   {
   public:
      enum ots_algorithm_t
         {
         WOTSP_SHA2_256_W16 = 0x01000001,
         WOTSP_SHA2_512_W16 = 0x02000002,
         WOTSP_SHAKE128_W16 = 0x03000003,
         WOTSP_SHAKE256_W16 = 0x04000004
         };

      XMSS_WOTS_Parameters(const std::string& algo_name);
      XMSS_WOTS_Parameters(ots_algorithm_t ots_spec);

      static ots_algorithm_t xmss_wots_id_from_string(const std::string& param_set);

      /**
       * Algorithm 1: convert input string to base.
       *
       * @param msg Input string (referred to as X in [1]).
       * @param out_size size of message in base w.
       *
       * @return Input string converted to the given base.
       **/
      secure_vector<uint8_t> base_w(const secure_vector<uint8_t>& msg, size_t out_size) const;

      secure_vector<uint8_t> base_w(size_t value) const;

      void append_checksum(secure_vector<uint8_t>& data);

      /**
       * @return XMSS WOTS registry name for the chosen parameter set.
       **/
      const std::string& name() const
         {
         return m_name;
         }

      /**
       * @return Botan name for the hash function used.
       **/
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
       * The Winternitz parameter.
       *
       * @return numeric base used for internal representation of
       *         data.
       **/
      size_t wots_parameter() const { return m_w; }

      size_t len() const { return m_len; }

      size_t len_1() const { return m_len_1; }

      size_t len_2() const { return m_len_2; }

      size_t lg_w() const { return m_lg_w; }

      ots_algorithm_t oid() const { return m_oid; }

      size_t estimated_strength() const { return m_strength; }

      bool operator==(const XMSS_WOTS_Parameters& p) const
         {
         return m_oid == p.m_oid;
         }

   private:
      static const std::map<std::string, ots_algorithm_t> m_oid_name_lut;
      ots_algorithm_t m_oid;
      std::string m_name;
      std::string m_hash_name;
      size_t m_element_size;
      size_t m_w;
      size_t m_len_1;
      size_t m_len_2;
      size_t m_len;
      size_t m_strength;
      uint8_t m_lg_w;
   };

}

#endif
