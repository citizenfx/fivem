/*
* Stream Cipher Utility Header
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_STREAM_CIPHER_UTILS_H__
#define BOTAN_STREAM_CIPHER_UTILS_H__

#include <botan/stream_cipher.h>
#include <botan/internal/algo_registry.h>
#include <botan/loadstor.h>
#include <botan/rotate.h>
#include <botan/internal/xor_buf.h>
#include <algorithm>

namespace Botan {

#define BOTAN_REGISTER_STREAM_CIPHER(name, maker) BOTAN_REGISTER_T(StreamCipher, name, maker)
#define BOTAN_REGISTER_STREAM_CIPHER_NOARGS(name) BOTAN_REGISTER_T_NOARGS(StreamCipher, name)

#define BOTAN_REGISTER_STREAM_CIPHER_1LEN(name, def) BOTAN_REGISTER_T_1LEN(StreamCipher, name, def)

#define BOTAN_REGISTER_STREAM_CIPHER_NAMED_NOARGS(type, name) BOTAN_REGISTER_NAMED_T(StreamCipher, name, type, make_new_T<type>)
#define BOTAN_REGISTER_STREAM_CIPHER_NAMED_1LEN(type, name, def) \
   BOTAN_REGISTER_NAMED_T(StreamCipher, name, type, (make_new_T_1len<type,def>))

}

#endif
