/*
* Modular Exponentiation
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_DEFAULT_MODEXP_H__
#define BOTAN_DEFAULT_MODEXP_H__

#include <botan/pow_mod.h>
#include <botan/reducer.h>
#include <vector>

namespace Botan {

/**
* Fixed Window Exponentiator
*/
class Fixed_Window_Exponentiator : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&);
      void set_base(const BigInt&);
      BigInt execute() const;

      Modular_Exponentiator* copy() const
         { return new Fixed_Window_Exponentiator(*this); }

      Fixed_Window_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      Modular_Reducer reducer;
      BigInt exp;
      size_t window_bits;
      std::vector<BigInt> g;
      Power_Mod::Usage_Hints hints;
   };

/**
* Montgomery Exponentiator
*/
class Montgomery_Exponentiator : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&);
      void set_base(const BigInt&);
      BigInt execute() const;

      Modular_Exponentiator* copy() const
         { return new Montgomery_Exponentiator(*this); }

      Montgomery_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      BigInt m_exp, m_modulus, m_R_mod, m_R2_mod;
      word m_mod_prime;
      size_t m_mod_words, m_exp_bits, m_window_bits;
      Power_Mod::Usage_Hints m_hints;
      std::vector<BigInt> m_g;
   };

}

#endif
