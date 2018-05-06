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

#ifndef VOICETARGET_H_98762439875
#define VOICETARGET_H_98762439875

#include "client.h"
#include "list.h"

#define TARGET_MAX_CHANNELS 16
#define TARGET_MAX_SESSIONS 32

typedef struct {
	int channel;
	bool_t linked;
	bool_t children;
} channeltarget_t;

typedef struct {
	int id;
	channeltarget_t channels[TARGET_MAX_CHANNELS];
	int sessions[TARGET_MAX_SESSIONS];
	struct dlist node;
} voicetarget_t;

void Voicetarget_add_id(client_t *client, int targetId);
void Voicetarget_del_id(client_t *client, int targetId);

void Voicetarget_add_session(client_t *client, int targetId, int sessionId);
void Voicetarget_add_channel(client_t *client, int targetId, int channelId,
							 bool_t linked, bool_t children);
voicetarget_t *Voicetarget_get_id(client_t *client, int targetId);

void Voicetarget_free_all(client_t *client);


#endif
