/*
* Ed25519
* (C) 2017 Ribose Inc
*
* Based on the public domain code from SUPERCOP ref10 by
* Peter Schwabe, Daniel J. Bernstein, Niels Duif, Tanja Lange, Bo-Yin Yang
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ED25519_INT_H_
#define BOTAN_ED25519_INT_H_

#include <botan/internal/ed25519_fe.h>
#include <botan/loadstor.h>

namespace Botan {

inline uint64_t load_3(const uint8_t in[3])
   {
   return static_cast<uint64_t>(in[0]) |
      (static_cast<uint64_t>(in[1]) << 8) |
      (static_cast<uint64_t>(in[2]) << 16);
   }

inline uint64_t load_4(const uint8_t* in)
   {
   return load_le<uint32_t>(in, 0);
   }

/*
ge means group element.

Here the group is the set of pairs (x,y) of field elements (see fe.h)
satisfying -x^2 + y^2 = 1 + d x^2y^2
where d = -121665/121666.

Representations:
  ge_p3 (extended): (X:Y:Z:T) satisfying x=X/Z, y=Y/Z, XY=ZT
*/

typedef struct
   {
   fe X;
   fe Y;
   fe Z;
   fe T;
   } ge_p3;

int ge_frombytes_negate_vartime(ge_p3*, const uint8_t*);
void ge_scalarmult_base(uint8_t out[32], const uint8_t in[32]);

void ge_double_scalarmult_vartime(uint8_t out[32],
                                  const uint8_t a[],
                                  const ge_p3* A,
                                  const uint8_t b[]);

/*
The set of scalars is \Z/l
where l = 2^252 + 27742317777372353535851937790883648493.
*/

void sc_reduce(uint8_t*);
void sc_muladd(uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*);

}

#endif
