/**
 * XMSS WOTS Common Operations
 * (C) 2016 Matthias Gierlings
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 **/

#ifndef BOTAN_XMSS_WOTS_COMMON_OPS_H__
#define BOTAN_XMSS_WOTS_COMMON_OPS_H__

#include <cstddef>
#include <botan/types.h>
#include <botan/xmss_wots_parameters.h>
#include <botan/xmss_address.h>
#include <botan/xmss_hash.h>

namespace Botan {

/**
 * Operations shared by XMSS WOTS signature generation and verification
 * operations.
 **/
class XMSS_WOTS_Common_Ops
   {
   public:
      XMSS_WOTS_Common_Ops(XMSS_WOTS_Parameters::ots_algorithm_t oid)
         : m_wots_params(oid), m_hash(m_wots_params.hash_function_name()) {}


   protected:
      /**
       * Algorithm 2: Chaining Function.
       *
       * @param[out] result Contains the n-byte input string "x" upon call to chain(),
       *               that will be replaced with the value obtained by iterating
       *               the cryptographic hash function "F" steps times on the
       *               input x using the outputs of the PRNG "G".
       * @param[in] start_idx The start index.
       * @param[in] steps A number of steps.
       * @param[in] adrs An OTS Hash Address.
       * @param[in] seed A Seed.
       **/
      void chain(secure_vector<uint8_t>& result,
                 size_t start_idx,
                 size_t steps,
                 XMSS_Address& adrs,
                 const secure_vector<uint8_t>& seed);

      XMSS_WOTS_Parameters m_wots_params;
      XMSS_Hash m_hash;
   };

}

#endif
