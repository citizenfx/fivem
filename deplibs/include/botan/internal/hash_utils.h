/*
* Hash Utility Header
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_HASH_UTILS_H__
#define BOTAN_HASH_UTILS_H__

#include <botan/hash.h>
#include <botan/internal/algo_registry.h>
#include <botan/loadstor.h>
#include <botan/rotate.h>

namespace Botan {

#define BOTAN_REGISTER_HASH(name, maker) BOTAN_REGISTER_T(HashFunction, name, maker)
#define BOTAN_REGISTER_HASH_NOARGS(name) BOTAN_REGISTER_T_NOARGS(HashFunction, name)

#define BOTAN_REGISTER_HASH_1LEN(name, def) BOTAN_REGISTER_T_1LEN(HashFunction, name, def)

#define BOTAN_REGISTER_HASH_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(HashFunction, name, type, make_new_T<type>)
#define BOTAN_REGISTER_HASH_NAMED_1LEN(type, name, def) \
   BOTAN_REGISTER_NAMED_T(HashFunction, name, type, (make_new_T_1len<type,def>))

#define BOTAN_REGISTER_HASH_NOARGS_IF(cond, type, name, provider, pref)      \
   BOTAN_COND_REGISTER_NAMED_T_NOARGS(cond, HashFunction, type, name, provider, pref)

}

#endif
