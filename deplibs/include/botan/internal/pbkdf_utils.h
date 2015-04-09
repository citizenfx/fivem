/*
* PBKDF Utility Header
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PBKDF_UTILS_H__
#define BOTAN_PBKDF_UTILS_H__

#include <botan/pbkdf.h>
#include <botan/internal/algo_registry.h>

namespace Botan {

#define BOTAN_REGISTER_PBKDF_1HASH(type, name)                    \
   BOTAN_REGISTER_NAMED_T(PBKDF, name, type, (make_new_T_1X<type, HashFunction>))
#define BOTAN_REGISTER_PBKDF_1MAC(type, name)                    \
   BOTAN_REGISTER_NAMED_T(PBKDF, name, type, (make_new_T_1X<type, MessageAuthenticationCode>))

}

#endif
