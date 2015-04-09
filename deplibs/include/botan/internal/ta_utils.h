/*
* Timing Attack Countermeasure Functions
* (C) 2010 Falko Strenzke, Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TIMING_ATTACK_CM_H__
#define BOTAN_TIMING_ATTACK_CM_H__

#include <botan/types.h>

namespace Botan {

namespace TA_CM {

/**
* Function used in timing attack countermeasures
* See Wagner, Molnar, et al "The Program Counter Security Model"
*
* @param in an integer
* @return 0 if in == 0 else 0xFFFFFFFF
*/
u32bit expand_mask_u32bit(u32bit in);


/**
 * Expand an input to a bit mask depending on it being being zero or
 * non-zero
 * @ param in the input
 * @return the mask 0xFFFF if tst is non-zero and 0 otherwise
 */
u16bit expand_mask_u16bit(u16bit in);

/**
* Branch-free maximum
* Note: assumes twos-complement signed representation
* @param a an integer
* @param b an integer
* @return max(a,b)
*/
u32bit max_32(u32bit a, u32bit b);

/**
* Branch-free minimum
* Note: assumes twos-complement signed representation
* @param a an integer
* @param b an integer
* @return min(a,b)
*/
u32bit min_32(u32bit a, u32bit b);

}

}

#endif
