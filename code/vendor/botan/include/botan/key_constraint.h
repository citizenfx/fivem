/*
* Enumerations
* (C) 1999-2007 Jack Lloyd
* (C) 2016 René Korthaus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ENUMS_H_
#define BOTAN_ENUMS_H_

#include <botan/types.h>
#include <string>

namespace Botan {

/**
* X.509v3 Key Constraints.
* If updating update copy in ffi.h
*/
enum Key_Constraints {
   NO_CONSTRAINTS     = 0,
   DIGITAL_SIGNATURE  = 1 << 15,
   NON_REPUDIATION    = 1 << 14,
   KEY_ENCIPHERMENT   = 1 << 13,
   DATA_ENCIPHERMENT  = 1 << 12,
   KEY_AGREEMENT      = 1 << 11,
   KEY_CERT_SIGN      = 1 << 10,
   CRL_SIGN           = 1 << 9,
   ENCIPHER_ONLY      = 1 << 8,
   DECIPHER_ONLY      = 1 << 7
};

class Public_Key;

/**
* Check that key constraints are permitted for a specific public key.
* @param pub_key the public key on which the constraints shall be enforced on
* @param constraints the constraints that shall be enforced on the key
* @throw Invalid_Argument if the given constraints are not permitted for this key
*/
BOTAN_PUBLIC_API(2,0) void verify_cert_constraints_valid_for_key_type(const Public_Key& pub_key,
                                                                Key_Constraints constraints);

std::string BOTAN_PUBLIC_API(2,0) key_constraints_to_string(Key_Constraints);

}

#endif
