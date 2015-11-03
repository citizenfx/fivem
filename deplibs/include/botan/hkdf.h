/*
* HKDF
* (C) 2013,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_HKDF_H__
#define BOTAN_HKDF_H__

#include <botan/mac.h>
#include <botan/hash.h>
#include <botan/kdf.h>

namespace Botan {

/**
* HKDF, see @rfc 5869 for details
* This is only the expansion portion of HKDF
*/
class BOTAN_DLL HKDF : public KDF
   {
   public:
      HKDF(MessageAuthenticationCode* prf) : m_prf(prf) {}

      static HKDF* make(const Spec& spec);

      KDF* clone() const override { return new HKDF(m_prf->clone()); }

      std::string name() const override { return "HKDF(" + m_prf->name() + ")"; }

      size_t kdf(byte out[], size_t out_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

}

#endif
