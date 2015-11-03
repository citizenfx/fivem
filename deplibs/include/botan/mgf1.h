/*
* MGF1
* (C) 1999-2007,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MGF1_H__
#define BOTAN_MGF1_H__

#include <botan/hash.h>

namespace Botan {

/**
* MGF1 from PKCS #1 v2.0
*/
void mgf1_mask(HashFunction& hash,
               const byte in[], size_t in_len,
               byte out[], size_t out_len);

}

#endif
