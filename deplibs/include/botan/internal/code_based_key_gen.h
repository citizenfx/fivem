/**
 * (C) Copyright Projet SECRET, INRIA, Rocquencourt
 * (C) Bhaskar Biswas and  Nicolas Sendrier
 *
 * (C) 2014 cryptosource GmbH
 * (C) 2014 Falko Strenzke fstrenzke@cryptosource.de
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 *
 */

#ifndef BOTAN_CODE_BASED_KEY_GEN_H__
#define BOTAN_CODE_BASED_KEY_GEN_H__

#include <botan/mceliece_key.h>

namespace Botan {

McEliece_PrivateKey generate_mceliece_key(RandomNumberGenerator &rng,
                                          u32bit ext_deg,
                                          u32bit code_length,
                                          u32bit t);

}

#endif
