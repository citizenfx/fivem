/*
* Authentication Code Utility Header
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MAC_UTILS_H__
#define BOTAN_MAC_UTILS_H__

#include <botan/internal/algo_registry.h>
#include <botan/internal/xor_buf.h>
#include <botan/loadstor.h>
#include <botan/rotate.h>
#include <algorithm>

namespace Botan {

#define BOTAN_REGISTER_MAC(name, maker) BOTAN_REGISTER_T(MessageAuthenticationCode, name, maker)
#define BOTAN_REGISTER_MAC_NOARGS(name) BOTAN_REGISTER_T_NOARGS(MessageAuthenticationCode, name)

#define BOTAN_REGISTER_MAC_1LEN(name, def) BOTAN_REGISTER_T_1LEN(MessageAuthenticationCode, name, def)

#define BOTAN_REGISTER_MAC_NAMED_NOARGS(type, name) \
   BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, name, type, make_new_T<type>)

#define BOTAN_REGISTER_MAC_NAMED_1LEN(type, name, def)                  \
   BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, name, type, (make_new_T_1len<type,def>))
#define BOTAN_REGISTER_MAC_NAMED_1STR(type, name, def) \
   BOTAN_REGISTER_NAMED_T(MessageAuthenticationCode, name, type, \
                          std::bind(make_new_T_1str<type>, std::placeholders::_1, def));

}

#endif
