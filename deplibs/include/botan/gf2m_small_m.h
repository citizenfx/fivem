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

#ifndef BOTAN_GF2M_SMALL_M_H__
#define BOTAN_GF2M_SMALL_M_H__

#include <vector>
#include <botan/types.h>

namespace Botan {

namespace gf2m_small_m {

typedef u16bit gf2m;

class Gf2m_Field
   {
   public:
      Gf2m_Field(size_t extdeg);

      gf2m gf_mul(gf2m x, gf2m y)
         {
         return ((x) ? gf_mul_fast(x, y) : 0);
         }

      gf2m gf_square(gf2m x)
         {
         return ((x) ? m_gf_exp_table[_gf_modq_1(m_gf_log_table[x] << 1)] : 0);
         }

      gf2m square_rr(gf2m x)
         {
         return _gf_modq_1(x << 1);
         }

      // naming convention of GF(2^m) field operations:
      // l logarithmic, unreduced
      // r logarithmic, reduced
      // n normal, non-zero
      // z normal, might be zero
      //
      inline gf2m gf_mul_lll(gf2m a, gf2m b);
      inline gf2m gf_mul_rrr(gf2m a, gf2m b);
      inline gf2m gf_mul_nrr(gf2m a, gf2m b);
      inline gf2m gf_mul_rrn(gf2m a, gf2m y);
      inline gf2m gf_mul_lnn(gf2m x, gf2m y);
      inline gf2m gf_mul_rnn(gf2m x, gf2m y);
      inline gf2m gf_mul_nrn(gf2m a, gf2m y);
      inline gf2m gf_mul_rnr(gf2m y, gf2m a);
      inline gf2m gf_mul_zrz(gf2m a, gf2m y);
      inline gf2m gf_mul_zzr(gf2m a, gf2m y);
      inline gf2m gf_mul_nnr(gf2m y, gf2m a);
      inline gf2m gf_sqrt(gf2m x) ;
      gf2m gf_div(gf2m x, gf2m y);
      inline gf2m gf_div_rnn(gf2m x, gf2m y);
      inline gf2m gf_div_rnr(gf2m x, gf2m b);
      inline gf2m gf_div_nrr(gf2m a, gf2m b);
      inline gf2m gf_div_zzr(gf2m x, gf2m b);
      inline gf2m gf_inv(gf2m x);
      inline gf2m gf_inv_rn(gf2m x);
      inline gf2m gf_square_ln(gf2m x);
      inline gf2m gf_square_rr(gf2m a) ;
      inline gf2m gf_l_from_n(gf2m x);

      inline gf2m gf_mul_fast(gf2m a, gf2m b);

      gf2m gf_exp(gf2m i)
         {
         return m_gf_exp_table[i]; /* alpha^i */
         }

      gf2m gf_log(gf2m i)
         {
         return m_gf_log_table[i]; /* return i when x=alpha^i */
         }

      inline gf2m gf_ord() const
         {
         return m_gf_multiplicative_order;
         }

      inline gf2m get_extension_degree() const
         {
         return m_gf_extension_degree;
         }

      inline gf2m get_cardinality() const
         {
         return m_gf_cardinality;
         }

      gf2m gf_pow(gf2m x, int i) ;

   private:
      gf2m m_gf_extension_degree, m_gf_cardinality, m_gf_multiplicative_order;
      std::vector<gf2m> m_gf_log_table;
      std::vector<gf2m> m_gf_exp_table;

      inline gf2m _gf_modq_1(s32bit d);
      void init_log();
      void init_exp();
   };

gf2m Gf2m_Field::_gf_modq_1(s32bit d)
   {
   return  (((d) & gf_ord()) + ((d) >> m_gf_extension_degree));
   }

gf2m Gf2m_Field::gf_mul_fast(gf2m x, gf2m y)
   {
   return ((y) ? m_gf_exp_table[_gf_modq_1(m_gf_log_table[x] + m_gf_log_table[y])] : 0);
   }

gf2m Gf2m_Field::gf_mul_lll(gf2m a, gf2m b)
   {
   return  (a + b);
   }

gf2m Gf2m_Field::gf_mul_rrr(gf2m a, gf2m b)
   {
   return (_gf_modq_1(gf_mul_lll(a, b)));
   }

gf2m Gf2m_Field::gf_mul_nrr(gf2m a, gf2m b)
   {
   return (gf_exp(gf_mul_rrr(a, b)));
   }

gf2m Gf2m_Field::gf_mul_rrn(gf2m a, gf2m y)
   {
   return _gf_modq_1(gf_mul_lll(a, gf_log(y)));
   }

gf2m Gf2m_Field::gf_mul_rnr(gf2m y, gf2m a)
   {
   return gf_mul_rrn(a, y);
   }

gf2m Gf2m_Field::gf_mul_lnn(gf2m x, gf2m y)
   {
   return (m_gf_log_table[x] + m_gf_log_table[y]);
   }
gf2m Gf2m_Field::gf_mul_rnn(gf2m x, gf2m y)
   {
   return _gf_modq_1(gf_mul_lnn(x, y));
   }

gf2m Gf2m_Field::gf_mul_nrn(gf2m a, gf2m y)
   {
   return m_gf_exp_table[_gf_modq_1((a) + m_gf_log_table[y])];
   }

/**
* zero operand allowed
*/
gf2m Gf2m_Field::gf_mul_zrz(gf2m a, gf2m y)
   {
   return ( (y == 0) ? 0 : gf_mul_nrn(a, y) );
   }

gf2m Gf2m_Field::gf_mul_zzr(gf2m a, gf2m y)
   {
   return gf_mul_zrz(y, a);
   }
/**
* non-zero operand
*/
gf2m Gf2m_Field::gf_mul_nnr(gf2m y, gf2m a)
   {
   return gf_mul_nrn( a, y);
   }

gf2m Gf2m_Field::gf_sqrt(gf2m x)
   {
   return ((x) ? m_gf_exp_table[_gf_modq_1(m_gf_log_table[x] << (m_gf_extension_degree-1))] : 0);
   }

gf2m Gf2m_Field::gf_div_rnn(gf2m x, gf2m y)
   {
   return _gf_modq_1(m_gf_log_table[x] - m_gf_log_table[y]);
   }
gf2m Gf2m_Field::gf_div_rnr(gf2m x, gf2m b)
   {
   return _gf_modq_1(m_gf_log_table[x] - b);
   }
gf2m Gf2m_Field::gf_div_nrr(gf2m a, gf2m b)
   {
   return m_gf_exp_table[_gf_modq_1(a - b)];
   }

gf2m Gf2m_Field::gf_div_zzr(gf2m x, gf2m b)
   {
   return ((x) ? m_gf_exp_table[_gf_modq_1(m_gf_log_table[x] - b)] : 0);
   }

gf2m Gf2m_Field::gf_inv(gf2m x)
   {
   return m_gf_exp_table[gf_ord() - m_gf_log_table[x]];
   }
gf2m Gf2m_Field::gf_inv_rn(gf2m x)
   {
   return (gf_ord() - m_gf_log_table[x]);
   }

gf2m Gf2m_Field::gf_square_ln(gf2m x)
   {
   return m_gf_log_table[x] << 1;
   }

gf2m Gf2m_Field::gf_square_rr(gf2m a)
   {
   return a << 1;
   }

gf2m Gf2m_Field::gf_l_from_n(gf2m x)
   {
   return m_gf_log_table[x];
   }

u32bit encode_gf2m(gf2m to_enc, byte* mem);

gf2m decode_gf2m(const byte* mem);

}

}

#endif
