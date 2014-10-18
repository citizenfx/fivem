/*
* EMSA1 BSI Variant
* (C) 1999-2008 Jack Lloyd
*     2007 FlexSecure GmbH
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_EMSA1_BSI_H__
#define BOTAN_EMSA1_BSI_H__

#include <botan/emsa1.h>

namespace Botan {

/**
* EMSA1_BSI is a variant of EMSA1 specified by the BSI. It accepts
* only hash values which are less or equal than the maximum key
* length. The implementation comes from InSiTo
*/
class BOTAN_DLL EMSA1_BSI : public EMSA1
   {
   public:
      /**
      * @param hash the hash object to use
      */
      EMSA1_BSI(HashFunction* hash) : EMSA1(hash) {}
   private:
      secure_vector<byte> encoding_of(const secure_vector<byte>&, size_t,
                                     RandomNumberGenerator& rng);
   };

}

#endif
