/*
* Enumerations
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_ENUMS_H__
#define BOTAN_ENUMS_H__

#include <botan/ber_dec.h>

namespace Botan {

/**
* X.509v3 Key Constraints.
*/
enum Key_Constraints {
   NO_CONSTRAINTS     = 0,
   DIGITAL_SIGNATURE  = 32768,
   NON_REPUDIATION    = 16384,
   KEY_ENCIPHERMENT   = 8192,
   DATA_ENCIPHERMENT  = 4096,
   KEY_AGREEMENT      = 2048,
   KEY_CERT_SIGN      = 1024,
   CRL_SIGN           = 512,
   ENCIPHER_ONLY      = 256,
   DECIPHER_ONLY      = 128
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
