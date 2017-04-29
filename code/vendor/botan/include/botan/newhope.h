/*
* NEWHOPE Ring-LWE scheme
* Based on the public domain reference implementation by the
* designers (https://github.com/tpoeppelmann/newhope)
*
* Further changes
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_NEWHOPE_H__
#define BOTAN_NEWHOPE_H__

#include <botan/rng.h>

namespace Botan {

/*
* WARNING: This API is preliminary and will change
* Currently pubkey.h does not support a 2-phase KEM scheme of
* the sort NEWHOPE exports.
*/

// TODO: change to just a secure_vector
class newhope_poly
   {
   public:
      uint16_t coeffs[1024];
      ~newhope_poly() { secure_scrub_memory(coeffs, sizeof(coeffs)); }
   };

enum Newhope_Params {
   NEWHOPE_SENDABYTES = 1824,
   NEWHOPE_SENDBBYTES = 2048,

   NEWHOPE_OFFER_BYTES  = 1824,
   NEWHOPE_ACCEPT_BYTES = 2048,
   NEWHOPE_SHARED_KEY_BYTES = 32,

   CECPQ1_OFFER_BYTES   = NEWHOPE_OFFER_BYTES + 32,
   CECPQ1_ACCEPT_BYTES  = NEWHOPE_ACCEPT_BYTES + 32,
   CECPQ1_SHARED_KEY_BYTES = NEWHOPE_SHARED_KEY_BYTES + 32
};

/**
* This chooses the XOF + hash for NewHope
* The official NewHope specification and reference implementation use
* SHA-3 and SHAKE-128. BoringSSL instead uses SHA-256 and AES-128 in
* CTR mode. CECPQ1 (x25519+NewHope) always uses BoringSSL's mode
*/
enum class Newhope_Mode {
   SHA3,
   BoringSSL
};

// offer
void BOTAN_DLL newhope_keygen(uint8_t *send,
                              newhope_poly *sk,
                              RandomNumberGenerator& rng,
                              Newhope_Mode = Newhope_Mode::SHA3);

// accept
void BOTAN_DLL newhope_sharedb(uint8_t *sharedkey,
                               uint8_t *send,
                               const uint8_t *received,
                               RandomNumberGenerator& rng,
                               Newhope_Mode mode = Newhope_Mode::SHA3);

// finish
void BOTAN_DLL newhope_shareda(uint8_t *sharedkey,
                               const newhope_poly *ska,
                               const uint8_t *received,
                               Newhope_Mode mode = Newhope_Mode::SHA3);

}

#endif
