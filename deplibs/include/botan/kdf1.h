/*
* KDF1
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
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
      secure_vector<byte> derive(size_t,
                                const byte secret[], size_t secret_len,
                                const byte P[], size_t P_len) const;

      std::string name() const { return "KDF1(" + hash->name() + ")"; }
      KDF* clone() const { return new KDF1(hash->clone()); }

      KDF1(HashFunction* h) : hash(h) {}
   private:
      std::unique_ptr<HashFunction> hash;
   };

}

#endif
