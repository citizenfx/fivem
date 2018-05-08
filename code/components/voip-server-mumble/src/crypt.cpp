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

/*
 * This code implements OCB-AES128.
 * In the US, OCB is covered by patents. The inventor has given a license
 * to all programs distributed under the GPL.
 * uMurmur is BSD (revised) licensed, meaning you can use the code in a
 * closed-source program. If you do, you'll have to either replace
 * OCB with something else or get yourself a license.
 */

#include "StdInc.h"
#include <string.h>

#ifndef _WIN32
#include <arpa/inet.h>
#endif

#include "crypt.h"
#include "byteorder.h"
#include "ssl.h"

#if defined(USE_POLARSSL_HAVEGE) || defined(USE_MBEDTLS_HAVEGE)
extern havege_state hs;
#endif

static void CryptState_ocb_encrypt(cryptState_t *cs, const unsigned char *plain, unsigned char *encrypted, unsigned int len, const unsigned char *nonce, unsigned char *tag);
static void CryptState_ocb_decrypt(cryptState_t *cs, const unsigned char *encrypted, unsigned char *plain, unsigned int len, const unsigned char *nonce, unsigned char *tag);

void CryptState_init(cryptState_t *cs)
{
	memset(cs->decrypt_history, 0, 0xff);
	memset(cs->raw_key, 0, AES_BLOCK_SIZE);
	memset(cs->encrypt_iv, 0, AES_BLOCK_SIZE);
	memset(cs->decrypt_iv, 0, AES_BLOCK_SIZE);
	cs->bInit = false;
	cs->uiGood = cs->uiLate = cs->uiLost = cs->uiResync = 0;
	cs->uiRemoteGood = cs->uiRemoteLate = cs->uiRemoteLost = cs->uiRemoteResync = 0;
	Timer_init(&cs->tLastGood);
	Timer_init(&cs->tLastRequest);
}

bool_t CryptState_isValid(cryptState_t *cs)
{
	return cs->bInit;
}

void CryptState_genKey(cryptState_t *cs)
{
	CRYPT_RANDOM_BYTES(&cs->raw_key, AES_BLOCK_SIZE);
	CRYPT_RANDOM_BYTES(&cs->encrypt_iv, AES_BLOCK_SIZE);
	CRYPT_RANDOM_BYTES(&cs->decrypt_iv, AES_BLOCK_SIZE);

	CRYPT_SET_ENC_KEY(&cs->encrypt_key, cs->raw_key, 128);
	CRYPT_SET_DEC_KEY(&cs->decrypt_key, cs->raw_key, 128);

	cs->bInit = true;
}

void CryptState_setKey(cryptState_t *cs, const unsigned char *rkey, const unsigned char *eiv, const unsigned char *div)
{
	memcpy(cs->raw_key, rkey, AES_BLOCK_SIZE);
	memcpy(cs->encrypt_iv, eiv, AES_BLOCK_SIZE);
	memcpy(cs->decrypt_iv, div, AES_BLOCK_SIZE);

	CRYPT_SET_ENC_KEY(&cs->encrypt_key, cs->decrypt_iv, 128);
	CRYPT_SET_DEC_KEY(&cs->decrypt_key, cs->raw_key, 128);

	cs->bInit = true;
}

void CryptState_setDecryptIV(cryptState_t *cs, const unsigned char *iv)
{
	memcpy(cs->decrypt_iv, iv, AES_BLOCK_SIZE);
}

void CryptState_encrypt(cryptState_t *cs, const unsigned char *source, unsigned char *dst, unsigned int plain_length)
{
	unsigned char tag[AES_BLOCK_SIZE];
	int i;
	// First, increase our IV.
	for (i = 0; i < AES_BLOCK_SIZE; i++)
		if (++cs->encrypt_iv[i])
			break;

	CryptState_ocb_encrypt(cs, source, dst+4, plain_length, cs->encrypt_iv, tag);

	dst[0] = cs->encrypt_iv[0];
	dst[1] = tag[0];
	dst[2] = tag[1];
	dst[3] = tag[2];
}

bool_t CryptState_decrypt(cryptState_t *cs, const unsigned char *source, unsigned char *dst, unsigned int crypted_length)
{
	if (crypted_length < 4)
		return false;

	unsigned int plain_length = crypted_length - 4;

	unsigned char saveiv[AES_BLOCK_SIZE];
	unsigned char ivbyte = source[0];
	bool_t restore = false;
	unsigned char tag[AES_BLOCK_SIZE];

	int lost = 0;
	int late = 0;

	memcpy(saveiv, cs->decrypt_iv, AES_BLOCK_SIZE);

	if (((cs->decrypt_iv[0] + 1) & 0xFF) == ivbyte) {
		// In order as expected.
		if (ivbyte > cs->decrypt_iv[0]) {
			cs->decrypt_iv[0] = ivbyte;
		} else if (ivbyte < cs->decrypt_iv[0]) {
			int i;
			cs->decrypt_iv[0] = ivbyte;
			for (i = 1; i < AES_BLOCK_SIZE; i++)
				if (++cs->decrypt_iv[i])
					break;
		} else {
			return false;
		}
	} else {
		// This is either out of order or a repeat.

		int diff = ivbyte - cs->decrypt_iv[0];
		if (diff > 128)
			diff = diff-256;
		else if (diff < -128)
			diff = diff+256;

		if ((ivbyte < cs->decrypt_iv[0]) && (diff > -30) && (diff < 0)) {
			// Late packet, but no wraparound.
			late = 1;
			lost = -1;
			cs->decrypt_iv[0] = ivbyte;
			restore = true;
		} else if ((ivbyte > cs->decrypt_iv[0]) && (diff > -30) && (diff < 0)) {
			int i;
			// Last was 0x02, here comes 0xff from last round
			late = 1;
			lost = -1;
			cs->decrypt_iv[0] = ivbyte;
			for (i = 1; i < AES_BLOCK_SIZE; i++)
				if (cs->decrypt_iv[i]--)
					break;
			restore = true;
		} else if ((ivbyte > cs->decrypt_iv[0]) && (diff > 0)) {
			// Lost a few packets, but beyond that we're good.
			lost = ivbyte - cs->decrypt_iv[0] - 1;
			cs->decrypt_iv[0] = ivbyte;
		} else if ((ivbyte < cs->decrypt_iv[0]) && (diff > 0)) {
			int i;
			// Lost a few packets, and wrapped around
			lost = 256 - cs->decrypt_iv[0] + ivbyte - 1;
			cs->decrypt_iv[0] = ivbyte;
			for (i = 1; i < AES_BLOCK_SIZE; i++)
				if (++cs->decrypt_iv[i])
					break;
		} else {
			return false;
		}

		if (cs->decrypt_history[cs->decrypt_iv[0]] == cs->decrypt_iv[1]) {
			memcpy(cs->decrypt_iv, saveiv, AES_BLOCK_SIZE);
			return false;
		}
	}

	CryptState_ocb_decrypt(cs, source+4, dst, plain_length, cs->decrypt_iv, tag);

	if (memcmp(tag, source+1, 3) != 0) {
		memcpy(cs->decrypt_iv, saveiv, AES_BLOCK_SIZE);
		return false;
	}
	cs->decrypt_history[cs->decrypt_iv[0]] = cs->decrypt_iv[1];

	if (restore)
		memcpy(cs->decrypt_iv, saveiv, AES_BLOCK_SIZE);

	cs->uiGood++;
	cs->uiLate += late;
	cs->uiLost += lost;

	Timer_restart(&cs->tLastGood);
	return true;
}

static void inline XOR(subblock *dst, const subblock *a, const subblock *b) {
	int i;
	for (i=0;i<BLOCKSIZE;i++) {
		dst[i] = a[i] ^ b[i];
	}
}

static void inline S2(subblock *block) {
	subblock carry = SWAPPED(block[0]) >> SHIFTBITS;
	int i;
	for (i=0;i<BLOCKSIZE-1;i++)
		block[i] = SWAPPED((SWAPPED(block[i]) << 1) | (SWAPPED(block[i+1]) >> SHIFTBITS));
	block[BLOCKSIZE-1] = SWAPPED((SWAPPED(block[BLOCKSIZE-1]) << 1) ^(carry * 0x87));
}

static void inline S3(subblock *block) {
	subblock carry = SWAPPED(block[0]) >> SHIFTBITS;
	int i;
	for (i=0;i<BLOCKSIZE-1;i++)
		block[i] ^= SWAPPED((SWAPPED(block[i]) << 1) | (SWAPPED(block[i+1]) >> SHIFTBITS));
	block[BLOCKSIZE-1] ^= SWAPPED((SWAPPED(block[BLOCKSIZE-1]) << 1) ^(carry * 0x87));
}

static void inline ZERO(subblock *block) {
	int i;
	for (i=0;i<BLOCKSIZE;i++)
		block[i]=0;
}

void CryptState_ocb_encrypt(cryptState_t *cs, const unsigned char *plain, unsigned char *encrypted, unsigned int len, const unsigned char *nonce, unsigned char *tag) {
	subblock checksum[BLOCKSIZE], delta[BLOCKSIZE], tmp[BLOCKSIZE], pad[BLOCKSIZE];

	// Initialize
	CRYPT_AES_ENCRYPT(nonce, delta, cs);
	ZERO(checksum);

	while (len > AES_BLOCK_SIZE) {
		S2(delta);
		XOR(tmp, delta, (const subblock *)(plain));
		CRYPT_AES_ENCRYPT(tmp, tmp, cs);
		XOR((subblock *)(encrypted), delta, tmp);
		XOR(checksum, checksum, (subblock *)(plain));
		len -= AES_BLOCK_SIZE;
		plain += AES_BLOCK_SIZE;
		encrypted += AES_BLOCK_SIZE;
	}

	S2(delta);
	ZERO(tmp);
	tmp[BLOCKSIZE - 1] = SWAPPED(len * 8);
	XOR(tmp, tmp, delta);
	CRYPT_AES_ENCRYPT(tmp, pad, cs);
	memcpy(tmp, plain, len);
	memcpy((unsigned char *)tmp + len, (unsigned char *)pad + len, AES_BLOCK_SIZE - len);
	XOR(checksum, checksum, tmp);
	XOR(tmp, pad, tmp);
	memcpy(encrypted, tmp, len);

	S3(delta);
	XOR(tmp, delta, checksum);
	CRYPT_AES_ENCRYPT(tmp, tag, cs);
}

void CryptState_ocb_decrypt(cryptState_t *cs, const unsigned char *encrypted, unsigned char *plain, unsigned int len, const unsigned char *nonce, unsigned char *tag) {
	subblock checksum[BLOCKSIZE], delta[BLOCKSIZE], tmp[BLOCKSIZE], pad[BLOCKSIZE];
	// Initialize
	CRYPT_AES_ENCRYPT(nonce, delta, cs);
	ZERO(checksum);

	while (len > AES_BLOCK_SIZE) {
		S2(delta);
		XOR(tmp, delta, (const subblock *)(encrypted));
		CRYPT_AES_DECRYPT(tmp, tmp, cs);
		XOR((subblock *)(plain), delta, tmp);
		XOR(checksum, checksum, (const subblock *)(plain));
		len -= AES_BLOCK_SIZE;
		plain += AES_BLOCK_SIZE;
		encrypted += AES_BLOCK_SIZE;
	}

	S2(delta);
	ZERO(tmp);
	tmp[BLOCKSIZE - 1] = SWAPPED(len * 8);
	XOR(tmp, tmp, delta);
	CRYPT_AES_ENCRYPT(tmp, pad, cs);
	memset(tmp, 0, AES_BLOCK_SIZE);
	memcpy(tmp, encrypted, len);
	XOR(tmp, tmp, pad);
	XOR(checksum, checksum, tmp);
	memcpy(plain, tmp, len);

	S3(delta);
	XOR(tmp, delta, checksum);
	CRYPT_AES_ENCRYPT(tmp, tag, cs);
}
