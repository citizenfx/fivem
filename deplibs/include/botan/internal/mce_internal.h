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

#ifndef BOTAN_MCELIECE_INTERNAL_H__
#define BOTAN_MCELIECE_INTERNAL_H__

#include <botan/secmem.h>
#include <botan/types.h>
#include <botan/pk_ops.h>
#include <botan/mceliece.h>

namespace Botan {

void mceliece_decrypt(secure_vector<byte>& plaintext_out,
                      secure_vector<byte>& error_mask_out,
                      const byte ciphertext[],
                      size_t ciphertext_len,
                      const McEliece_PrivateKey& key);

void mceliece_decrypt(secure_vector<byte>& plaintext_out,
                      secure_vector<byte>& error_mask_out,
                      const secure_vector<byte>& ciphertext,
                      const McEliece_PrivateKey& key);

secure_vector<byte> mceliece_decrypt(
   secure_vector<gf2m> & error_pos,
   const byte *ciphertext, u32bit ciphertext_len,
   const McEliece_PrivateKey & key);

void mceliece_encrypt(secure_vector<byte>& ciphertext_out,
                      secure_vector<byte>& error_mask_out,
                      const secure_vector<byte>& plaintext,
                      const McEliece_PublicKey& key,
                      RandomNumberGenerator& rng);

McEliece_PrivateKey generate_mceliece_key(RandomNumberGenerator &rng,
                                          u32bit ext_deg,
                                          u32bit code_length,
                                          u32bit t);

}


#endif
