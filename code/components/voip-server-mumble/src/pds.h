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
#ifndef PDS_H_457865876
#define PDS_H_457865876

#include <stdint.h>
#include "types.h"

typedef struct {
	uint8_t *data;
	uint32_t maxsize;
	uint32_t offset;
	uint32_t overshoot;
	bool_t bOk;
} pds_t;

void Pds_append_data(pds_t *pds, const uint8_t *data, uint32_t len);
void Pds_append_data_nosize(pds_t *pds, const uint8_t *data, uint32_t len);
uint64_t Pds_get_numval(pds_t *pds);
void Pds_add_numval(pds_t *pds, const uint64_t value);
pds_t *Pds_create(uint8_t *buf, int size);
void Pds_free(pds_t *pds);
void Pds_add_string(pds_t *pds, const char *str);
void Pds_get_string(pds_t *pds, char *str, int maxlen);
void Pds_add_double(pds_t *pds, double value);
double Pds_get_double(pds_t *pds);
int Pds_get_data(pds_t *pds, uint8_t *data, int maxlen);
uint8_t Pds_next8(pds_t *pds);
int Pds_skip(pds_t *pds, int offset);


#endif
