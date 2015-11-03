/*
* Blinding for public key operations
* (C) 1999-2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_BLINDER_H__
#define BOTAN_BLINDER_H__

#include <botan/bigint.h>
#include <botan/reducer.h>
#include <functional>

namespace Botan {

class RandomNumberGenerator;

/**
* Blinding Function Object
*/
class BOTAN_DLL Blinder
   {
   public:
      BigInt blind(const BigInt& x) const;

      BigInt unblind(const BigInt& x) const;

      bool initialized() const { return m_reducer.initialized(); }

      Blinder() {}

      Blinder(const BigInt& modulus,
              std::function<BigInt (const BigInt&)> fwd_func,
              std::function<BigInt (const BigInt&)> inv_func);

      Blinder(const Blinder&) = delete;

      Blinder& operator=(const Blinder&) = delete;

   private:
      BigInt blinding_nonce() const;

      Modular_Reducer m_reducer;
      std::unique_ptr<RandomNumberGenerator> m_rng;
      std::function<BigInt (const BigInt&)> m_fwd_fn;
      std::function<BigInt (const BigInt&)> m_inv_fn;
      size_t m_modulus_bits = 0;

      mutable BigInt m_e, m_d;
      mutable size_t m_counter = 0;
   };

}

#endif
