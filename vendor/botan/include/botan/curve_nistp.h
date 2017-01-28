/*
* Arithmetic operations specialized for NIST ECC primes
* (C) 2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_NIST_PRIMES_H__
#define BOTAN_NIST_PRIMES_H__

#include <botan/bigint.h>

namespace Botan {

/**
* NIST Prime reduction functions.
*
* Reduces the value in place
*
* ws is a workspace function which is used as a temporary,
* and will be resized as needed.
*/
BOTAN_DLL const BigInt& prime_p521();
BOTAN_DLL void redc_p521(BigInt& x, secure_vector<word>& ws);

#if (BOTAN_MP_WORD_BITS == 32) || (BOTAN_MP_WORD_BITS == 64)

#define BOTAN_HAS_NIST_PRIME_REDUCERS_W32

BOTAN_DLL const BigInt& prime_p384();
BOTAN_DLL void redc_p384(BigInt& x, secure_vector<word>& ws);

BOTAN_DLL const BigInt& prime_p256();
BOTAN_DLL void redc_p256(BigInt& x, secure_vector<word>& ws);

BOTAN_DLL const BigInt& prime_p224();
BOTAN_DLL void redc_p224(BigInt& x, secure_vector<word>& ws);

BOTAN_DLL const BigInt& prime_p192();
BOTAN_DLL void redc_p192(BigInt& x, secure_vector<word>& ws);

#endif

}

#endif
