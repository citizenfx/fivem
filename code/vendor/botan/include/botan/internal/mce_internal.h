/*
 * (C) Copyright Projet SECRET, INRIA, Rocquencourt
 * (C) Bhaskar Biswas and  Nicolas Sendrier
 *
 * (C) 2014 cryptosource GmbH
 * (C) 2014 Falko Strenzke fstrenzke@cryptosource.de
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 *
 */

#ifndef BOTAN_MCELIECE_INTERNAL_H_
#define BOTAN_MCELIECE_INTERNAL_H_

#include <botan/secmem.h>
#include <botan/types.h>
#include <botan/pk_ops.h>
#include <botan/mceliece.h>

namespace Botan {

void mceliece_decrypt(secure_vector<uint8_t>& plaintext_out,
                      secure_vector<uint8_t>& error_mask_out,
                      const uint8_t ciphertext[],
                      size_t ciphertext_len,
                      const McEliece_PrivateKey& key);

void mceliece_decrypt(secure_vector<uint8_t>& plaintext_out,
                      secure_vector<uint8_t>& error_mask_out,
                      const secure_vector<uint8_t>& ciphertext,
                      const McEliece_PrivateKey& key);

secure_vector<uint8_t> mceliece_decrypt(
   secure_vector<gf2m> & error_pos,
   const uint8_t *ciphertext, uint32_t ciphertext_len,
   const McEliece_PrivateKey & key);

void mceliece_encrypt(secure_vector<uint8_t>& ciphertext_out,
                      secure_vector<uint8_t>& error_mask_out,
                      const secure_vector<uint8_t>& plaintext,
                      const McEliece_PublicKey& key,
                      RandomNumberGenerator& rng);

McEliece_PrivateKey generate_mceliece_key(RandomNumberGenerator &rng,
                                          uint32_t ext_deg,
                                          uint32_t code_length,
                                          uint32_t t);

}


#endif
