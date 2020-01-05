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
#include <stdlib.h>
#include <string.h>
#include "voicetarget.h"
#include "log.h"
#include "memory.h"

void Voicetarget_add_session(client_t *client, int targetId, int sessionId)
{
	struct dlist *itr;
	voicetarget_t *vt;

	list_iterate(itr, &client->voicetargets) {
		if (targetId == list_get_entry(itr, voicetarget_t, node)->id) {
			int i;
			vt = list_get_entry(itr, voicetarget_t, node);
			for (i = 0; i < TARGET_MAX_SESSIONS; i++) {
				if (vt->sessions[i] == -1) {
					vt->sessions[i] = sessionId;
					Log_debug("Adding session ID %d to voicetarget ID %d", sessionId, targetId);
					return;
				}
			}
		}
	}
}

void Voicetarget_add_channel(client_t *client, int targetId, int channelId,
							 bool_t linked, bool_t children)
{
	struct dlist *itr;
	voicetarget_t *vt;

	list_iterate(itr, &client->voicetargets) {
		if (targetId == list_get_entry(itr, voicetarget_t, node)->id) {
			int i;
			vt = list_get_entry(itr, voicetarget_t, node);
			for (i = 0; i < TARGET_MAX_CHANNELS; i++) {
				if (vt->channels[i].channel == -1) {
					vt->channels[i].channel = channelId;
					vt->channels[i].linked = linked;
					vt->channels[i].children = children;
					Log_debug("Adding channel ID %d to voicetarget ID %d", channelId, targetId);
					return;
				}
			}
		}
	}
}

void Voicetarget_add_id(client_t *client, int targetId)
{
	voicetarget_t *newtarget;
	int i;

	Voicetarget_del_id(client, targetId);
	newtarget = (voicetarget_t*)Memory_safeCalloc(1, sizeof(voicetarget_t));
	for (i = 0; i < TARGET_MAX_CHANNELS; i++)
		newtarget->channels[i].channel = -1;
	for (i = 0; i < TARGET_MAX_SESSIONS; i++)
		newtarget->sessions[i] = -1;
	newtarget->id = targetId;
	list_add_tail(&newtarget->node, &client->voicetargets);
}

void Voicetarget_del_id(client_t *client, int targetId)
{
	struct dlist *itr, *save;
	list_iterate_safe(itr, save, &client->voicetargets) {
		if (targetId == list_get_entry(itr, voicetarget_t, node)->id) {
			list_del(&list_get_entry(itr, voicetarget_t, node)->node);
			free(list_get_entry(itr, voicetarget_t, node));
			Log_debug("Removing voicetarget ID %d", targetId);
		}
	}
}

voicetarget_t *Voicetarget_get_id(client_t *client, int targetId)
{
	struct dlist *itr;
	list_iterate(itr, &client->voicetargets) {
		if (targetId == list_get_entry(itr, voicetarget_t, node)->id) {
			return list_get_entry(itr, voicetarget_t, node);
		}
	}
	return NULL;
}


void Voicetarget_free_all(client_t *client)
{
	struct dlist *itr, *save;

	list_iterate_safe(itr, save, &client->voicetargets) {
		list_del(&list_get_entry(itr, voicetarget_t, node)->node);
		free(list_get_entry(itr, voicetarget_t, node));
	}
}
