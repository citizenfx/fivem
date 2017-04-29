/*
* CLMUL hook
* (C) 2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_GCM_CLMUL_H__
#define BOTAN_GCM_CLMUL_H__

#include <botan/types.h>

namespace Botan {

void gcm_multiply_clmul(uint8_t x[16], const uint8_t H[16]);

}

#endif
