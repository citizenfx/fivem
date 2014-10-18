/*
* SSLv3 PRF
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_SSLV3_PRF_H__
#define BOTAN_SSLV3_PRF_H__

#include <botan/kdf.h>

namespace Botan {

/**
* PRF used in SSLv3
*/
class BOTAN_DLL SSL3_PRF : public KDF
   {
   public:
      secure_vector<byte> derive(size_t, const byte[], size_t,
                                const byte[], size_t) const;

      std::string name() const { return "SSL3-PRF"; }
      KDF* clone() const { return new SSL3_PRF; }
   };

}

#endif
