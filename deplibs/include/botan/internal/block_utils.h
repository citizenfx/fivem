/*
* Block Cipher Utility Header
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_BLOCK_CIPHER_UTILS_H__
#define BOTAN_BLOCK_CIPHER_UTILS_H__

#include <botan/block_cipher.h>
#include <botan/internal/algo_registry.h>
#include <botan/loadstor.h>
#include <botan/rotate.h>
#include <botan/internal/xor_buf.h>
#include <algorithm>
#include <functional>

namespace Botan {

#define BOTAN_REGISTER_BLOCK_CIPHER(name, maker) BOTAN_REGISTER_T(BlockCipher, name, maker)
#define BOTAN_REGISTER_BLOCK_CIPHER_NOARGS(name) BOTAN_REGISTER_T_NOARGS(BlockCipher, name)

#define BOTAN_REGISTER_BLOCK_CIPHER_1LEN(name, def) BOTAN_REGISTER_T_1LEN(BlockCipher, name, def)

#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_NOARGS(type, name) BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, make_new_T<type>)
#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_1LEN(type, name, def) \
   BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, (make_new_T_1len<type,def>))
#define BOTAN_REGISTER_BLOCK_CIPHER_NAMED_1STR(type, name, def) \
   BOTAN_REGISTER_NAMED_T(BlockCipher, name, type, std::bind(make_new_T_1str<type>, std::placeholders::_1, def));

#define BOTAN_REGISTER_BLOCK_CIPHER_NOARGS_IF(cond, type, name, provider, pref) \
   BOTAN_COND_REGISTER_NAMED_T_NOARGS(cond, BlockCipher, type, name, provider, pref)

}

#endif
