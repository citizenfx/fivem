/*
* CLMUL hook
* (C) 2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_GCM_CLMUL_H_
#define BOTAN_GCM_CLMUL_H_

#include <botan/types.h>

namespace Botan {

void gcm_clmul_precompute(const uint8_t H[16], uint64_t H_pow[4*2]);

void gcm_multiply_clmul(uint8_t x[16],
                        const uint64_t H_pow[4*2],
                        const uint8_t input[], size_t blocks);

}

#endif
