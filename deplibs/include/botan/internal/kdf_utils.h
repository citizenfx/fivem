/*
* KDF Utility Header
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_KDF_UTILS_H__
#define BOTAN_KDF_UTILS_H__

#include <botan/kdf.h>
#include <botan/internal/algo_registry.h>
#include <botan/exceptn.h>
#include <botan/internal/xor_buf.h>

namespace Botan {

#define BOTAN_REGISTER_KDF_NOARGS(type, name)                    \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T<type>))
#define BOTAN_REGISTER_KDF_1HASH(type, name)                    \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T_1X<type, HashFunction>))

#define BOTAN_REGISTER_KDF_NAMED_1STR(type, name) \
   BOTAN_REGISTER_NAMED_T(KDF, name, type, (make_new_T_1str_req<type>))

}

#endif
