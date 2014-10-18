/*
* Public Key Work Factor Functions
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_WORKFACTOR_H__
#define BOTAN_WORKFACTOR_H__

#include <botan/types.h>

namespace Botan {

/**
* Estimate work factor for discrete logarithm
* @param prime_group_size size of the group in bits
* @return estimated security level for this group
*/
size_t dl_work_factor(size_t prime_group_size);

}

#endif
