/*
* Enumerations
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ENUMS_H__
#define BOTAN_ENUMS_H__

#include <botan/ber_dec.h>

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
* Create the key constraints for a specific public key.
* @param pub_key the public key from which the basic set of
* constraints to be placed in the return value is derived
* @param limits additional limits that will be incorporated into the
* return value
* @return combination of key type specific constraints and
* additional limits
*/

BOTAN_DLL Key_Constraints find_constraints(const Public_Key& pub_key,
                                           Key_Constraints limits);

/**
* BER Decoding Function for key constraints
*/
namespace BER {

void BOTAN_DLL decode(BER_Decoder&, Key_Constraints&);

}

}

#endif
