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

#ifndef BOTAN_MCE_GOPPA_CODE_H__
#define BOTAN_MCE_GOPPA_CODE_H__

#include <botan/polyn_gf2m.h>
#include <botan/mceliece_key.h>

namespace Botan {

std::vector<byte> mceliece_encrypt(const secure_vector<byte>& cleartext,
                                   const std::vector<byte>& public_matrix,
                                   const secure_vector<gf2m> & err_pos,
                                   u32bit code_length);

secure_vector<byte> mceliece_decrypt(secure_vector<gf2m> & error_pos,
                                     const byte *ciphertext,
                                     u32bit ciphertext_len,
                                     const McEliece_PrivateKey& key);

}

#endif
