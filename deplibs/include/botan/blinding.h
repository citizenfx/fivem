/*
* Blinding for public key operations
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_BLINDER_H__
#define BOTAN_BLINDER_H__

#include <botan/bigint.h>
#include <botan/reducer.h>
#include <functional>

namespace Botan {

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

   private:
      Modular_Reducer m_reducer;
      mutable BigInt m_e, m_d;
   };

}

#endif
