/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_GCM_CLMUL_SSSE3_H_
#define BOTAN_GCM_CLMUL_SSSE3_H_

#include <botan/types.h>

namespace Botan {

void gcm_multiply_ssse3(uint8_t x[16],
                        const uint64_t HM[256],
                        const uint8_t input[], size_t blocks);

}

#endif
