/**
 * (C) Copyright Projet SECRET, INRIA, Rocquencourt
 * (C) Bhaskar Biswas and  Nicolas Sendrier
 *
 * (C) 2014 cryptosource GmbH
 * (C) 2014 Falko Strenzke fstrenzke@cryptosource.de
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 *
 */

#ifndef BOTAN_BINARY_MATRIX_H__
#define BOTAN_BINARY_MATRIX_H__

#include <botan/secmem.h>

namespace Botan {

#define BITS_PER_U32 (8 * sizeof (u32bit))

struct binary_matrix
   {
   public:
      binary_matrix(u32bit m_rown, u32bit m_coln);

      void row_xor(u32bit a, u32bit b);
      secure_vector<int> row_reduced_echelon_form();

      /**
      * return the coefficient out of F_2
      */
      u32bit coef(u32bit i, u32bit j)
         {
         return (m_elem[(i) * m_rwdcnt + (j) / BITS_PER_U32] >> (j % BITS_PER_U32)) & 1;
         };

      void set_coef_to_one(u32bit i, u32bit j)
         {
         m_elem[(i) * m_rwdcnt + (j) / BITS_PER_U32] |= (1UL << ((j) % BITS_PER_U32)) ;
         };

      void toggle_coeff(u32bit i, u32bit j)
         {
         m_elem[(i) * m_rwdcnt + (j) / BITS_PER_U32] ^= (1UL << ((j) % BITS_PER_U32)) ;
         }

      void set_to_zero()
         {
         zeroise(m_elem);
         }

      u32bit m_rown;  // number of rows.
      u32bit m_coln; // number of columns.
      u32bit m_rwdcnt; // number of words in a row
      std::vector<u32bit> m_elem;
   };

}

#endif
