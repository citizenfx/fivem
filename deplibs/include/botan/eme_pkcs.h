/*
* EME PKCS#1 v1.5
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_EME_PKCS1_H__
#define BOTAN_EME_PKCS1_H__

#include <botan/eme.h>

namespace Botan {

/**
* EME from PKCS #1 v1.5
*/
class BOTAN_DLL EME_PKCS1v15 : public EME
   {
   public:
      size_t maximum_input_size(size_t) const;
   private:
      secure_vector<byte> pad(const byte[], size_t, size_t,
                             RandomNumberGenerator&) const;
      secure_vector<byte> unpad(const byte[], size_t, size_t) const;
   };

}

#endif
