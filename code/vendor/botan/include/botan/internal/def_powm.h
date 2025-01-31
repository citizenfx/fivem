/*
* Modular Exponentiation
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_DEFAULT_MODEXP_H_
#define BOTAN_DEFAULT_MODEXP_H_

#include <botan/pow_mod.h>
#include <botan/reducer.h>
#include <vector>

namespace Botan {

/**
* Fixed Window Exponentiator
*/
class Fixed_Window_Exponentiator final : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&) override;
      void set_base(const BigInt&) override;
      BigInt execute() const override;

      Modular_Exponentiator* copy() const override
         { return new Fixed_Window_Exponentiator(*this); }

      Fixed_Window_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      Modular_Reducer m_reducer;
      BigInt m_exp;
      size_t m_window_bits;
      std::vector<BigInt> m_g;
      Power_Mod::Usage_Hints m_hints;
   };

class Montgomery_Params;
class Montgomery_Exponentation_State;

/**
* Montgomery Exponentiator
*/
class Montgomery_Exponentiator final : public Modular_Exponentiator
   {
   public:
      void set_exponent(const BigInt&) override;
      void set_base(const BigInt&) override;
      BigInt execute() const override;

      Modular_Exponentiator* copy() const override
         { return new Montgomery_Exponentiator(*this); }

      Montgomery_Exponentiator(const BigInt&, Power_Mod::Usage_Hints);
   private:
      BigInt m_p;
      Modular_Reducer m_mod_p;
      std::shared_ptr<const Montgomery_Params> m_monty_params;
      std::shared_ptr<const Montgomery_Exponentation_State> m_monty;

      BigInt m_e;
      Power_Mod::Usage_Hints m_hints;
   };

}

#endif
