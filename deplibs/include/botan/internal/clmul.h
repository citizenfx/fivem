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

void gcm_multiply_clmul(byte x[16], const byte H[16]);

}

#endif
