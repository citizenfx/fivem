/*
* KDF2
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_KDF2_H__
#define BOTAN_KDF2_H__

#include <botan/kdf.h>
#include <botan/hash.h>

namespace Botan {

/**
* KDF2, from IEEE 1363
*/
class BOTAN_DLL KDF2 : public KDF
   {
   public:
      secure_vector<byte> derive(size_t, const byte[], size_t,
                                const byte[], size_t) const;

      std::string name() const { return "KDF2(" + hash->name() + ")"; }
      KDF* clone() const { return new KDF2(hash->clone()); }

      KDF2(HashFunction* h) : hash(h) {}
   private:
      std::unique_ptr<HashFunction> hash;
   };

}

#endif
