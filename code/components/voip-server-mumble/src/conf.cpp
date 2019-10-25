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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "conf.h"
#include "log.h"

#define DEFAULT_WELCOME "Welcome to uMurmur!"
#define DEFAULT_MAX_CLIENTS 512
#define DEFAULT_MAX_BANDWIDTH 76000
#define DEFAULT_BINDPORT 64738
#define DEFAULT_BAN_LENGTH (60*60)
#define DEFAULT_OPUS_THRESHOLD 100

void Conf_init(const char *conffile)
{
/*	config_init(&configuration);
	if (conffile == NULL)
		conffile = defaultconfig;
	if (config_read_file(&configuration, conffile) != CONFIG_TRUE) {
		Log_fatal("Error in config file %s line %d: %s", conffile,
			config_error_line(&configuration), config_error_text(&configuration));
	}*/
}

bool_t Conf_ok(const char *conffile)
{
	bool_t rc = true;
	/*config_init(&configuration);
	if (conffile == NULL)
		conffile = defaultconfig;
	if (config_read_file(&configuration, conffile) != CONFIG_TRUE) {
		fprintf(stderr, "Error in config file %s line %d: %s\n", conffile,
			config_error_line(&configuration), config_error_text(&configuration));
		rc = false;
	}
	config_destroy(&configuration);*/
	return rc;
}

void Conf_deinit()
{
	//config_destroy(&configuration);
}

const char *getStrConf(param_t param)
{
	switch (param) {
		case CERTIFICATE:
			return "/etc/umurmur/certificate.crt";
		case KEY:
			return "/etc/umurmur/private_key.key";
		case CAPATH:
			return NULL;
		case PASSPHRASE:
			return "";
		case ADMIN_PASSPHRASE:
			return "";
		case WELCOMETEXT:
			return DEFAULT_WELCOME;
		case DEFAULT_CHANNEL:
			return "";
		case BANFILE:
			return NULL;
		default:
			doAssert(false);
			break;
	}
	return NULL;
}

int getIntConf(param_t param)
{
	switch (param) {
		case BAN_LENGTH:
			return DEFAULT_BAN_LENGTH;
		case MAX_BANDWIDTH:
			return DEFAULT_MAX_BANDWIDTH;
		case MAX_CLIENTS:
			return DEFAULT_MAX_CLIENTS;
		case OPUS_THRESHOLD:
			return DEFAULT_OPUS_THRESHOLD;
		default:
			doAssert(false);
	}

	assert(false);
	return 0;
}

bool_t getBoolConf(param_t param)
{
	switch (param) {
		case ALLOW_TEXTMESSAGE:
			return true;
		case ENABLE_BAN:
			return false;
		case SYNC_BANFILE:
			return false;
		case SHOW_ADDRESSES:
			return true;
		default:
			doAssert(false);
	}

	assert(false);
	return false;
}

int Conf_getNextChannel(conf_channel_t *chdesc, int index)
{
	if (index > 0)
	{
		return -1;
	}

	chdesc->name = "Root";
	chdesc->description = "The root.";
	chdesc->parent = NULL;
	chdesc->password = NULL;
	chdesc->noenter = false;
	chdesc->position = 0;
	chdesc->silent = false;
// 	config_setting_t *setting = NULL;
// 	int maxconfig = 64, ret = 0;
// 	char configstr[maxconfig];
// 
// 	ret = snprintf(configstr, maxconfig, "channels.[%d].name", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL)
// 		return -1; /* Required */
// 	chdesc->name =  config_setting_get_string(setting);
// 
// 	ret = snprintf(configstr, maxconfig, "channels.[%d].parent", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL)
// 		return -1; /* Required */
// 	chdesc->parent = config_setting_get_string(setting);
// 
// 	ret = snprintf(configstr, maxconfig, "channels.[%d].description", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL) /* Optional */
// 		chdesc->description = NULL;
// 	else
// 		chdesc->description = config_setting_get_string(setting);
// 
// 	ret = snprintf(configstr, maxconfig, "channels.[%d].password", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL) /* Optional */
// 		chdesc->password = NULL;
// 	else
// 		chdesc->password = config_setting_get_string(setting);
// 
// 	ret = snprintf(configstr, maxconfig, "channels.[%d].noenter", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL) /* Optional */
// 		chdesc->noenter = false;
// 	else
// 		chdesc->noenter = config_setting_get_bool(setting);
// 
// 	ret = snprintf(configstr, maxconfig, "channels.[%d].silent", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL) /* Optional */
// 		chdesc->silent = false;
// 	else
// 		chdesc->silent = config_setting_get_bool(setting);
// 
// 	ret = snprintf(configstr, maxconfig, "channels.[%d].position", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL) /* Optional */
// 		chdesc->position = 0;
// 	else
// 		chdesc->position = config_setting_get_int(setting);

	return 0;
}

int Conf_getNextChannelLink(conf_channel_link_t *chlink, int index)
{
// 	config_setting_t *setting = NULL;
// 	int maxconfig = 64, ret = 0;
// 	char configstr[maxconfig];
// 
// 	ret = snprintf(configstr, maxconfig, "channel_links.[%d].source", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL)
// 		return -1;
// 	chlink->source = config_setting_get_string(setting);
// 
// 	ret = snprintf(configstr, maxconfig, "channel_links.[%d].destination", index);
// 	setting = config_lookup(&configuration, configstr);
// 	if (ret >= maxconfig || ret < 0 || setting == NULL)
// 		return -1;
// 	chlink->destination = config_setting_get_string(setting);

	return -1;
}
