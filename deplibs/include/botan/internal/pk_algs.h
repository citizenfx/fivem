/*
* PK Key Factory
* (C) 1999-2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_PK_KEY_FACTORY_H__
#define BOTAN_PK_KEY_FACTORY_H__

#include <botan/pk_keys.h>

namespace Botan {

Public_Key* make_public_key(const AlgorithmIdentifier& alg_id,
                            const secure_vector<byte>& key_bits);

Private_Key* make_private_key(const AlgorithmIdentifier& alg_id,
                              const secure_vector<byte>& key_bits,
                              RandomNumberGenerator& rng);

}

#endif
