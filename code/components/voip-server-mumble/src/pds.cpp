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
#include "StdInc.h"
#include <string.h>
#include <stdlib.h>
#include "pds.h"
#include "log.h"
#include "memory.h"

/*
 * Data serialization functions below
 */

typedef union double64u {
	uint64_t u64;
	double dval;
} double64u_t;

static inline void append_val(pds_t *pds, uint64_t val)
{
	if (pds->offset < pds->maxsize)
		pds->data[pds->offset++] = ((uint8_t)(val));
	else {
		pds->bOk = false;
		pds->overshoot++;
	}
}

void Pds_append_data(pds_t *pds, const uint8_t *data, uint32_t len)
{
	int left;
	Pds_add_numval(pds, len);
	left = pds->maxsize - pds->offset;
	if (left >= len) {
		memcpy(&pds->data[pds->offset], data, len);
		pds->offset += len;
	} else {
		memset(&pds->data[pds->offset], 0, left);
		pds->offset += left;
		pds->overshoot += len - left;
		pds->bOk = false;
	}
}

void Pds_append_data_nosize(pds_t *pds, const uint8_t *data, uint32_t len)
{
	int left;
	left = pds->maxsize - pds->offset;
	if (left >= len) {
		memcpy(&pds->data[pds->offset], data, len);
		pds->offset += len;
	} else {
		memset(&pds->data[pds->offset], 0, left);
		pds->offset += left;
		pds->overshoot += len - left;
		pds->bOk = false;
	}
}

uint8_t Pds_next8(pds_t *pds)
{
	if (pds->offset < pds->maxsize)
		return pds->data[pds->offset++];
	else {
		pds->bOk = false;
		return 0;
	}
}

int Pds_skip(pds_t *pds, int offset)
{
	if (pds->offset + offset <= pds->maxsize) {
		pds->offset += offset;
		return offset;
	} else {
		pds->bOk = false;
		return 0;
	}

}

static inline uint64_t next(pds_t *pds)
{
	if (pds->offset < pds->maxsize)
		return pds->data[pds->offset++];
	else {
		pds->bOk = false;
		return 0;
	}
}

pds_t *Pds_create(uint8_t *buf, int size)
{
	pds_t *pds = (pds_t*)Memory_safeMalloc(1, sizeof(pds_t));
	pds->data = buf;
	pds->offset = pds->overshoot = 0;
	pds->maxsize = size;
	pds->bOk = true;
	return pds;
}

void Pds_free(pds_t *pds)
{
	free(pds);
}

void Pds_add_double(pds_t *pds, double value)
{
	double64u_t u;

	u.dval = value;

	Pds_add_numval(pds, u.u64);
}

double Pds_get_double(pds_t *pds)
{
	double64u_t u;
	u.u64 = Pds_get_numval(pds);
	return u.dval;
}
void Pds_add_numval(pds_t *pds, const uint64_t value)
{
	uint64_t i = value;

	if ((i & 0x8000000000000000LL) && (~i < 0x100000000LL)) {
		// Signed number.
		i = ~i;
		if (i <= 0x3) {
			// Shortcase for -1 to -4
			append_val(pds, 0xFC | i);
			return;
		} else {
			append_val(pds, 0xF8);
		}
	}
	if (i < 0x80) {
		// Need top bit clear
		append_val(pds, i);
	} else if (i < 0x4000) {
		// Need top two bits clear
		append_val(pds, (i >> 8) | 0x80);
		append_val(pds, i & 0xFF);
	} else if (i < 0x200000) {
		// Need top three bits clear
		append_val(pds, (i >> 16) | 0xC0);
		append_val(pds, (i >> 8) & 0xFF);
		append_val(pds, i & 0xFF);
	} else if (i < 0x10000000) {
		// Need top four bits clear
		append_val(pds, (i >> 24) | 0xE0);
		append_val(pds, (i >> 16) & 0xFF);
		append_val(pds, (i >> 8) & 0xFF);
		append_val(pds, i & 0xFF);
	} else if (i < 0x100000000LL) {
		// It's a full 32-bit integer.
		append_val(pds, 0xF0);
		append_val(pds, (i >> 24) & 0xFF);
		append_val(pds, (i >> 16) & 0xFF);
		append_val(pds, (i >> 8) & 0xFF);
		append_val(pds, i & 0xFF);
	} else {
		// It's a 64-bit value.
		append_val(pds, 0xF4);
		append_val(pds, (i >> 56) & 0xFF);
		append_val(pds, (i >> 48) & 0xFF);
		append_val(pds, (i >> 40) & 0xFF);
		append_val(pds, (i >> 32) & 0xFF);
		append_val(pds, (i >> 24) & 0xFF);
		append_val(pds, (i >> 16) & 0xFF);
		append_val(pds, (i >> 8) & 0xFF);
		append_val(pds, i & 0xFF);
	}
}

uint64_t Pds_get_numval(pds_t *pds)
{
	uint64_t i = 0;
	uint64_t v = next(pds);

	if ((v & 0x80) == 0x00) {
		i=(v & 0x7F);
	} else if ((v & 0xC0) == 0x80) {
		i=(v & 0x3F) << 8 | next(pds);
	} else if ((v & 0xF0) == 0xF0) {
		switch (v & 0xFC) {
		case 0xF0:
			i=next(pds) << 24 | next(pds) << 16 | next(pds) << 8 | next(pds);
			break;
		case 0xF4:
			i=next(pds) << 56 | next(pds) << 48 | next(pds) << 40 | next(pds) << 32 | next(pds) << 24 | next(pds) << 16 | next(pds) << 8 | next(pds);
			break;
		case 0xF8:
			i = Pds_get_numval(pds);
			i = ~i;
			break;
		case 0xFC:
			i=v & 0x03;
			i = ~i;
			break;
		default:
			//ok = false;
			i = 0;
			break;
		}
	} else if ((v & 0xF0) == 0xE0) {
		i=(v & 0x0F) << 24 | next(pds) << 16 | next(pds) << 8 | next(pds);
	} else if ((v & 0xE0) == 0xC0) {
		i=(v & 0x1F) << 16 | next(pds) << 8 | next(pds);
	}
	return i;
}

void Pds_add_string(pds_t *pds, const char *str)
{
	Pds_append_data(pds, (uint8_t *)str, strlen(str));
}

void Pds_get_string(pds_t *pds, char *str, int maxlen)
{
	int len = Pds_get_numval(pds);
	if (len < maxlen) {
		memcpy(str, &pds->data[pds->offset], len);
		str[len] = '\0';
		pds->offset += len;
	} else {
		Log_warn("Too long string from network");
		strcpy(str, "N/A");
	}
}

int Pds_get_data(pds_t *pds, uint8_t *data, int maxlen)
{
	int len = Pds_get_numval(pds);
	if (len < maxlen) {
		memcpy(data, &pds->data[pds->offset], len);
		pds->offset += len;
		return len;
	} else {
		Log_warn("Pds_get_data: Too much data from network");
		return -1;
	}
}
