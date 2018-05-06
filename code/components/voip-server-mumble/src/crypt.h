/* Copyright (C) 2009-2014, Martin Johansson <martin@fatbob.nu>
   Copyright (C) 2005-2014, Thorvald Natvig <thorvald@natvig.com>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Developers nor the names of its contributors may
     be used to endorse or promote products derived from this software without
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CRYPTSTATE_H_34564356
#define CRYPTSTATE_H_34564356

#include "byteorder.h"
#include "config.h"

#if defined(USE_POLARSSL)

#include <polarssl/havege.h>
#include <polarssl/aes.h>

#define CRYPT_AES_KEY aes_context
#define AES_BLOCK_SIZE 16

#define CRYPT_RANDOM_BYTES(dest, size) RAND_bytes((unsigned char *)(dest), (size))
#define CRYPT_SET_ENC_KEY(dest, source, size) aes_setkey_enc((dest), (source), (size));
#define CRYPT_SET_DEC_KEY(dest, source, size) aes_setkey_dec((dest), (source), (size));

#define CRYPT_AES_ENCRYPT(src, dst, cryptstate) aes_crypt_ecb(&(cryptstate)->encrypt_key, AES_ENCRYPT, (unsigned char *)(src), (unsigned char *)(dst));
#define CRYPT_AES_DECRYPT(src, dst, cryptstate) aes_crypt_ecb(&(cryptstate)->decrypt_key, AES_DECRYPT, (unsigned char *)(src), (unsigned char *)(dst));

#elif defined(USE_MBEDTLS)

#include <mbedtls/havege.h>
#include <mbedtls/aes.h>

#define CRYPT_AES_KEY mbedtls_aes_context
#define AES_BLOCK_SIZE 16

#define CRYPT_RANDOM_BYTES(dest, size) RAND_bytes((unsigned char *)(dest), (size))
#define CRYPT_SET_ENC_KEY(dest, source, size) mbedtls_aes_setkey_enc((dest), (source), (size));
#define CRYPT_SET_DEC_KEY(dest, source, size) mbedtls_aes_setkey_dec((dest), (source), (size));

#define CRYPT_AES_ENCRYPT(src, dst, cryptstate) mbedtls_aes_crypt_ecb(&(cryptstate)->encrypt_key, MBEDTLS_AES_ENCRYPT, (unsigned char *)(src), (unsigned char *)(dst));
#define CRYPT_AES_DECRYPT(src, dst, cryptstate) mbedtls_aes_crypt_ecb(&(cryptstate)->decrypt_key, MBEDTLS_AES_DECRYPT, (unsigned char *)(src), (unsigned char *)(dst));

#elif defined(USE_GNUTLS)

#include <nettle/aes.h>
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>

#define CRYPT_AES_KEY struct aes_ctx
#define CRYPT_RANDOM_BYTES(dest, size) gnutls_rnd(GNUTLS_RND_KEY, (dest), (size))
#define CRYPT_SET_ENC_KEY(dest, source, size) aes_set_encrypt_key((dest), (size)/8, (source));
#define CRYPT_SET_DEC_KEY(dest, source, size) aes_set_decrypt_key((dest), (size)/8, (source));

#define CRYPT_AES_ENCRYPT(src, dest, ctx) aes_encrypt(&(ctx)->encrypt_key, AES_BLOCK_SIZE, (uint8_t *)(dest), (uint8_t *)(src))
#define CRYPT_AES_DECRYPT(src, dest, ctx) aes_decrypt(&(ctx)->decrypt_key, AES_BLOCK_SIZE, (uint8_t *)(dest), (uint8_t *)(src))

#else

#include <botan/auto_rng.h>
#include <botan/aes.h>

#include <array>

#define AES_BLOCK_SIZE 16

#define CRYPT_AES_KEY std::unique_ptr<Botan::BlockCipher>

inline void rand_bytes(uint8_t* dest, size_t size)
{
	Botan::AutoSeeded_RNG rng;
	rng.randomize(dest, size);
}

inline void AES_set_crypt_key(uint8_t* key, size_t size, std::unique_ptr<Botan::BlockCipher>* aes)
{
	switch (size / 8)
	{
	case 16:
		*aes = std::make_unique<Botan::AES_128>();
		break;
	case 24:
		*aes = std::make_unique<Botan::AES_192>();
		break;
	case 32:
		*aes = std::make_unique<Botan::AES_256>();
		break;
	}

	(*aes)->set_key(key, size / 8);
}

inline void AES_encrypt(const uint8_t* in, uint8_t* out, std::unique_ptr<Botan::BlockCipher>* aes)
{
	(*aes)->encrypt(in, out);
}

inline void AES_decrypt(const uint8_t* in, uint8_t* out, std::unique_ptr<Botan::BlockCipher>* aes)
{
	(*aes)->decrypt(in, out);
}

//#define CRYPT_AES_KEY AES_KEY
#define CRYPT_RANDOM_BYTES(dest, size) rand_bytes((unsigned char *)(dest), (size))
#define CRYPT_SET_ENC_KEY(dest, source, size) AES_set_crypt_key((source), (size), (dest));
#define CRYPT_SET_DEC_KEY(dest, source, size) AES_set_crypt_key((source), (size), (dest));

#define CRYPT_AES_ENCRYPT(src, dst, cryptstate) AES_encrypt((unsigned char *)(src), (unsigned char *)(dst), &(cryptstate)->encrypt_key);
#define CRYPT_AES_DECRYPT(src, dst, cryptstate) AES_decrypt((unsigned char *)(src), (unsigned char *)(dst), &(cryptstate)->decrypt_key);

#endif

#include <stdint.h>
#include "timer.h"
#include "types.h"

typedef struct CryptState {
	uint8_t raw_key[AES_BLOCK_SIZE];
	uint8_t encrypt_iv[AES_BLOCK_SIZE];
	uint8_t decrypt_iv[AES_BLOCK_SIZE];
	uint8_t decrypt_history[0x100];

	unsigned int uiGood;
	unsigned int uiLate;
	unsigned int uiLost;
	unsigned int uiResync;

	unsigned int uiRemoteGood;
	unsigned int uiRemoteLate;
	unsigned int uiRemoteLost;
	unsigned int uiRemoteResync;

	CRYPT_AES_KEY encrypt_key;
	CRYPT_AES_KEY decrypt_key;

	etimer_t tLastGood;
	etimer_t tLastRequest;
	bool_t bInit;
} cryptState_t;

void CryptState_init(cryptState_t *cs);
bool_t CryptState_isValid(cryptState_t *cs);
void CryptState_genKey(cryptState_t *cs);
void CryptState_setKey(cryptState_t *cs, const unsigned char *rkey, const unsigned char *eiv, const unsigned char *div);
void CryptState_setDecryptIV(cryptState_t *cs, const unsigned char *iv);

bool_t CryptState_decrypt(cryptState_t *cs, const unsigned char *source, unsigned char *dst, unsigned int crypted_length);
void CryptState_encrypt(cryptState_t *cs, const unsigned char *source, unsigned char *dst, unsigned int plain_length);

#endif
