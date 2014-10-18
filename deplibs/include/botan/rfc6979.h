/*
* RFC 6979 Deterministic Nonce Generator
* (C) 2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_RFC6979_GENERATOR_H__
#define BOTAN_RFC6979_GENERATOR_H__

#include <botan/bigint.h>
#include <string>

namespace Botan {

/**
* @param x the secret (EC)DSA key
* @param q the group order
* @param h the message hash already reduced mod q
* @param hash the hash function used to generate h
*/
BigInt BOTAN_DLL generate_rfc6979_nonce(const BigInt& x,
                                        const BigInt& q,
                                        const BigInt& h,
                                        const std::string& hash);

}

#endif
