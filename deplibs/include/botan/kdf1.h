/*
* KDF1
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_KDF1_H__
#define BOTAN_KDF1_H__

#include <botan/kdf.h>
#include <botan/hash.h>

namespace Botan {

/**
* KDF1, from IEEE 1363
*/
class BOTAN_DLL KDF1 : public KDF
   {
   public:
      std::string name() const override { return "KDF1(" + m_hash->name() + ")"; }

      KDF* clone() const override { return new KDF1(m_hash->clone()); }

      size_t kdf(byte key[], size_t key_len,
                 const byte secret[], size_t secret_len,
                 const byte salt[], size_t salt_len) const override;

      KDF1(HashFunction* h) : m_hash(h) {}
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}

#endif
