/*
* Division
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_DIVISON_ALGORITHM_H__
#define BOTAN_DIVISON_ALGORITHM_H__

#include <botan/bigint.h>

namespace Botan {

/**
* BigInt Division
* @param x an integer
* @param y a non-zero integer
* @param q will be set to x / y
* @param r will be set to x % y
*/
void BOTAN_DLL divide(const BigInt& x,
                      const BigInt& y,
                      BigInt& q,
                      BigInt& r);

}

#endif
