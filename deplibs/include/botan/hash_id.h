/*
* Hash Function Identification
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_HASHID_H__
#define BOTAN_HASHID_H__

#include <botan/secmem.h>
#include <string>

namespace Botan {

/**
* Return the PKCS #1 hash identifier
* @see RFC 3447 section 9.2
* @param hash_name the name of the hash function
* @return byte sequence identifying the hash
* @throw Invalid_Argument if the hash has no known PKCS #1 hash id
*/
BOTAN_DLL std::vector<byte> pkcs_hash_id(const std::string& hash_name);

/**
* Return the IEEE 1363 hash identifier
* @param hash_name the name of the hash function
* @return byte code identifying the hash, or 0 if not known
*/
BOTAN_DLL byte ieee1363_hash_id(const std::string& hash_name);

}

#endif
