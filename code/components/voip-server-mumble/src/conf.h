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
#ifndef CONF_H_24564356
#define CONF_H_24564356

#include "messages.h"
#include "config.h"

typedef enum param {
	CERTIFICATE,
	KEY,
	PASSPHRASE,
	CAPATH,
	BINDPORT,
	BINDPORT6,
	BINDADDR,
	BINDADDR6,
	WELCOMETEXT,
	MAX_BANDWIDTH,
	MAX_CLIENTS,
	DEFAULT_CHANNEL,
	USERNAME,
	GROUPNAME,
	LOGFILE,
	ADMIN_PASSPHRASE,
	BAN_LENGTH,
	ALLOW_TEXTMESSAGE,
	ENABLE_BAN,
	BANFILE,
	SYNC_BANFILE,
	OPUS_THRESHOLD,
	SHOW_ADDRESSES,
} param_t;

typedef struct {
	const char *parent;
	const char *name;
	const char *description;
	const char *password;
	bool_t noenter, silent;
	int position;
} conf_channel_t;

typedef struct {
	const char *source;
	const char *destination;
} conf_channel_link_t;

void Conf_init(const char *conffile);
void Conf_deinit();
bool_t Conf_ok(const char *conffile);

const char *getStrConf(param_t param);
int getIntConf(param_t param);
bool_t getBoolConf(param_t param);
int Conf_getNextChannel(conf_channel_t *chdesc, int index);
int Conf_getNextChannelLink(conf_channel_link_t *chlink, int index);

#endif
