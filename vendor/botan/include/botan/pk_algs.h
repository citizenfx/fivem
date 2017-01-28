/*
* PK Key Factory
* (C) 1999-2010,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PK_KEY_FACTORY_H__
#define BOTAN_PK_KEY_FACTORY_H__

#include <botan/pk_keys.h>
#include <botan/alg_id.h>
#include <memory>

namespace Botan {

BOTAN_DLL std::unique_ptr<Public_Key>
load_public_key(const AlgorithmIdentifier& alg_id,
                const std::vector<uint8_t>& key_bits);

BOTAN_DLL std::unique_ptr<Private_Key>
load_private_key(const AlgorithmIdentifier& alg_id,
                 const secure_vector<uint8_t>& key_bits);

/**
* Create a new key
* For ECC keys, algo_params specifies EC group (eg, "secp256r1")
* For DH/DSA/ElGamal keys, algo_params is DL group (eg, "modp/ietf/2048")
* For RSA, algo_params is integer keylength
* For McEliece, algo_params is n,t
* If algo_params is left empty, suitable default parameters are chosen.
*/
BOTAN_DLL std::unique_ptr<Private_Key>
create_private_key(const std::string& algo_name,
                   RandomNumberGenerator& rng,
                   const std::string& algo_params = "");

}

#endif
