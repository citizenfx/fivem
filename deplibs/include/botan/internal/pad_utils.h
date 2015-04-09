/*
* Public Key Padding Utility Header
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PK_PAD_UTILS_H__
#define BOTAN_PK_PAD_UTILS_H__

#include <botan/internal/algo_registry.h>
#include <botan/hash_id.h>
#include <botan/internal/xor_buf.h>
#include <botan/loadstor.h>
#include <algorithm>

namespace Botan {

#define BOTAN_REGISTER_EME(name, maker) BOTAN_REGISTER_T(EME, name, maker)
#define BOTAN_REGISTER_EME_NOARGS(name) BOTAN_REGISTER_T_NOARGS(EME, name)

#define BOTAN_REGISTER_EME_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(EME, name, type, make_new_T<type>)

#define BOTAN_REGISTER_EMSA_1HASH_1LEN(type, name)                    \
   BOTAN_REGISTER_NAMED_T(EMSA, name, type, (make_new_T_1X_1len<type, HashFunction>))

#define BOTAN_REGISTER_EME_NAMED_1LEN(type, name, def)                  \
   BOTAN_REGISTER_NAMED_T(EME, name, type, (make_new_T_1len<type,def>))
#define BOTAN_REGISTER_EME_NAMED_1STR(type, name, def) \
   BOTAN_REGISTER_NAMED_T(EME, name, type, \
                          std::bind(make_new_T_1str<type>, std::placeholders::_1, def));

#define BOTAN_REGISTER_EMSA_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(EMSA, name, type, make_new_T<type>)

#define BOTAN_REGISTER_EMSA(name, maker) BOTAN_REGISTER_T(EMSA, name, maker)
#define BOTAN_REGISTER_EMSA_NOARGS(name) BOTAN_REGISTER_T_NOARGS(EMSA, name)

#define BOTAN_REGISTER_EMSA_1HASH(type, name)                    \
   BOTAN_REGISTER_NAMED_T(EMSA, name, type, (make_new_T_1X<type, HashFunction>))

}

#endif
