/*
* X9.42 PRF
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_ANSI_X942_PRF_H__
#define BOTAN_ANSI_X942_PRF_H__

#include <botan/kdf.h>

namespace Botan {

/**
* PRF from ANSI X9.42
*/
class BOTAN_DLL X942_PRF : public KDF
   {
   public:
      secure_vector<byte> derive(size_t, const byte[], size_t,
                                const byte[], size_t) const;

      std::string name() const { return "X942_PRF(" + key_wrap_oid + ")"; }
      KDF* clone() const { return new X942_PRF(key_wrap_oid); }

      X942_PRF(const std::string& oid);
   private:
      std::string key_wrap_oid;
   };

}

#endif
