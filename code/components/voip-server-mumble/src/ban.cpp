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
#include <time.h>
#include <string.h>
#include "log.h"
#include "memory.h"
#include "list.h"
#include "ban.h"
#include "conf.h"
#include "ssl.h"
#include "util.h"

static void Ban_saveBanFile(void);
static void Ban_readBanFile(void);


declare_list(banlist);
static int bancount; /* = 0 */
static int ban_duration;
static bool_t banlist_changed;

void Ban_init(void)
{
	ban_duration = getIntConf(BAN_LENGTH);
	/* Read ban file here */
	if (getStrConf(BANFILE) != NULL)
		Ban_readBanFile();
}

void Ban_deinit(void)
{
	/* Save banlist */
	if (getStrConf(BANFILE) != NULL)
		Ban_saveBanFile();

	Ban_clearBanList();
}

void Ban_UserBan(client_t *client, char *reason)
{
	ban_t *ban;
	char hexhash[41];

	ban = (ban_t*)Memory_safeCalloc(1, sizeof(ban_t));

	memcpy(ban->hash, client->hash, 20);

	ban->address = *(sockaddr_storage*)client->remote_tcp.GetSocketAddress();
	ban->mask = (ban->address.ss_family == AF_INET) ? 32 : 128;
	ban->reason = strdup(reason);
	ban->name = strdup(client->username);
	ban->time = time(NULL);
	ban->duration = ban_duration;
	list_add_tail(&ban->node, &banlist);
	bancount++;
	banlist_changed = true;
	if(getBoolConf(SYNC_BANFILE))
		Ban_saveBanFile();

	hexhash[0] = '\0';
	//SSLi_hash2hex(ban->hash, hexhash);

	char *clientAddressString = Util_clientAddressToString(client);

	Log_info_client(client, "User kickbanned. Reason: '%s' Hash: %s IP: %s Banned for: %d seconds",
		ban->reason, hexhash, clientAddressString, ban->duration);

	free(clientAddressString);
}


void Ban_pruneBanned()
{
	struct dlist *itr;
	ban_t *ban;

	list_iterate(itr, &banlist) {
		ban = list_get_entry(itr, ban_t, node);
#ifdef DEBUG
		char hexhash[41];
		SSLi_hash2hex(ban->hash, hexhash);
		char *addressString = Util_addressToString(&ban->address);
		Log_debug("BL: User %s Reason: '%s' Hash: %s IP: %s Time left: %d",
			ban->name, ban->reason, hexhash, addressString,
			ban->time + ban->duration - time(NULL));
		free(addressString);
#endif
		/* Duration of 0 = forever */
		if (ban->duration != 0 && ban->time + ban->duration - time(NULL) <= 0) {
			free(ban->name);
			free(ban->reason);
			list_del(&ban->node);
			free(ban);
			bancount--;
			banlist_changed = true;
			if(getBoolConf(SYNC_BANFILE))
				Ban_saveBanFile();
		}
	}
}

bool_t Ban_isBanned(client_t *client)
{
	struct dlist *itr;
	ban_t *ban;
	list_iterate(itr, &banlist) {
		ban = list_get_entry(itr, ban_t, node);
		if (memcmp(ban->hash, client->hash, 20) == 0)
			return true;
	}
	return false;

}

bool_t Ban_isBannedAddr(struct sockaddr_storage *address)
{
	struct dlist *itr;
	ban_t *ban;

	list_iterate(itr, &banlist) {
		ban = list_get_entry(itr, ban_t, node);
		if (ban->address.ss_family == address->ss_family) {
			if (address->ss_family == AF_INET) {
				uint32_t a1, a2, mask;
				mask = (ban->mask == 32) ? UINT32_MAX : (1u << ban->mask) - 1;
				a1 = (uint32_t)((struct sockaddr_in *)&ban->address)->sin_addr.s_addr & mask;
				a2 = (uint32_t)((struct sockaddr_in *)address)->sin_addr.s_addr & mask;
				if (a1 == a2)
					return true;
			} else {
				uint64_t mask[2];
				uint64_t *a1 = (uint64_t *) &((struct sockaddr_in6 *)&ban->address)->sin6_addr.s6_addr;
				uint64_t *a2 = (uint64_t *) &((struct sockaddr_in6 *)address)->sin6_addr.s6_addr;

				if (ban->mask == 128)
					mask[0] = mask[1] = 0xffffffffffffffffULL;
				else if (ban->mask > 64) {
					mask[0] = 0xffffffffffffffffULL;
					mask[1] = SWAPPED(~((1ULL << (128 - ban->mask)) - 1));
				} else {
					mask[0] = SWAPPED(~((1ULL << (64 - ban->mask)) - 1));
					mask[1] = 0ULL;
				}
				if ((a1[0] & mask[0]) == (a2[0] & mask[0]) &&
				    (a1[1] & mask[1]) == (a2[1] & mask[1]))
				    return true;
			}
		}
	}

	return false;
}

int Ban_getBanCount(void)
{
	return bancount;
}

message_t *Ban_getBanList(void)
{
	int i = 0;
	struct dlist *itr;
	ban_t *ban;
	message_t *msg;
	struct tm timespec;
	char timestr[32];
	char hexhash[41];
	uint8_t address[16];

	msg = Msg_banList_create(bancount);
	list_iterate(itr, &banlist) {
		ban = list_get_entry(itr, ban_t, node);
		//gmtime_r(&ban->time, &timespec);
		strftime(timestr, 32, "%Y-%m-%dT%H:%M:%SZ", &timespec);
		//SSLi_hash2hex(ban->hash, hexhash);
		memset(address, 0, 16);

		if(ban->address.ss_family == AF_INET) {
			memcpy(&address[12], &((struct sockaddr_in *)&ban->address)->sin_addr, 4);
			memset(&address[10], 0xff, 2);
			Msg_banList_addEntry(msg, i++, address, ban->mask + 96, ban->name, hexhash, ban->reason, timestr, ban->duration);
		} else {
			memcpy(&address, &((struct sockaddr_in6 *)&ban->address)->sin6_addr, 16);
			Msg_banList_addEntry(msg, i++, address, ban->mask, ban->name, hexhash, ban->reason, timestr, ban->duration);
		}

	}
	return msg;
}

void Ban_clearBanList(void)
{
	ban_t *ban;
	struct dlist *itr, *save;
	list_iterate_safe(itr, save, &banlist) {
		ban = list_get_entry(itr, ban_t, node);
		free(ban->name);
		free(ban->reason);
		list_del(&ban->node);
		free(ban);
		bancount--;
	}
}

void Ban_putBanList(message_t *msg, int n_bans)
{
	int i = 0;
	struct tm timespec;
	ban_t *ban;
	char *hexhash, *name, *reason, *start;
	uint32_t duration, mask;
	uint8_t *address;
	uint8_t mappedBytes[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff};
	char *tz;

	for (i = 0; i < n_bans; i++) {
		Msg_banList_getEntry(msg, i, &address, &mask, &name, &hexhash, &reason, &start, &duration);
		ban = (ban_t*)Memory_safeMalloc(1, sizeof(ban_t));
		//SSLi_hex2hash(hexhash, ban->hash);

		if(memcmp(address, mappedBytes, 12) == 0) {
			memcpy(&((struct sockaddr_in *)&ban->address)->sin_addr, &address[12], 4);
			ban->address.ss_family = AF_INET;
			if (mask > 32) {
				mask = 32;
			}
		} else {
			memcpy(&((struct sockaddr_in6 *)&ban->address)->sin6_addr, address, 16);
			ban->address.ss_family = AF_INET6;
		}

		ban->mask = mask;
		ban->reason = strdup(reason);
		ban->name = strdup(name);

		/*
		 * Parse the timestring. We need to set TZ to UTC so that mktime() knows that the info in
		 * struct tm indeed is given in UTC. Otherwise it will use the current locale. There's
		 * apparently no other way to do this...
		 */
		memset(&timespec, 0, sizeof(struct tm));
		/*strptime(start, "%Y-%m-%dT%H:%M:%S", &timespec);
		tz = getenv("TZ");
		setenv("TZ", "UTC", 1);
		tzset();
		ban->time = mktime(&timespec);
		if (tz)
			setenv("TZ", tz, 1);
		else
			unsetenv("TZ");
		tzset();*/

		ban->duration = duration;
		list_add_tail(&ban->node, &banlist);
		bancount++;
	}
	banlist_changed = true;
	if(getBoolConf(SYNC_BANFILE))
		Ban_saveBanFile();
}

static void Ban_saveBanFile(void)
{
	struct dlist *itr;
	ban_t *ban;
	char hexhash[41];
	FILE *file;

	if (!banlist_changed)
		return;
	file = fopen(getStrConf(BANFILE), "w");
	if (file == NULL) {
		Log_warn("Could not save banlist to file %s: %s", getStrConf(BANFILE), strerror(errno));
		return;
	}
	list_iterate(itr, &banlist) {
		ban = list_get_entry(itr, ban_t, node);
		//SSLi_hash2hex(ban->hash, hexhash);

		char *addressString = Util_addressToString(&ban->address);
		fprintf(file, "%s,%s,%d,%ld,%d,%s,%s\n", hexhash, addressString,ban->mask, (long int)ban->time, ban->duration, ban->name, ban->reason);
		free(addressString);
	}
	fclose(file);
	banlist_changed = false;
	Log_info("Banlist file '%s': %d entries written", getStrConf(BANFILE), bancount);
}

static void Ban_readBanFile(void)
{
	ban_t *ban;
	char line[1024], *hexhash, *address, *name, *reason;
	uint32_t mask, duration;
	time_t time;
	char *p;
	FILE *file;

	file = fopen(getStrConf(BANFILE), "r");
	if (file == NULL) {
		Log_warn("Could not read banlist file %s: %s", getStrConf(BANFILE), strerror(errno));
		return;
	}
	while (fgets(line, 1024, file) != NULL) {
		p = strtok(line, ",");
		hexhash = p;
		p = strtok(NULL, ",");
		if (p == NULL) break;
		address = p;
		p = strtok(NULL, ",");
		if (p == NULL) break;
		mask = strtoul(p, NULL, 0);
		p = strtok(NULL, ",");
		if (p == NULL) break;
		time = strtoul(p, NULL, 0);
		p = strtok(NULL, ",");
		if (p == NULL) break;
		duration = strtoul(p, NULL, 0);
		p = strtok(NULL, ",");
		if (p == NULL) break;
		name = p;
		p = strtok(NULL, "\n");
		if (p == NULL) break;
		reason = p;

		ban = (ban_t*)Memory_safeMalloc(1, sizeof(ban_t));
		memset(ban, 0, sizeof(ban_t));
		//SSLi_hex2hash(hexhash, ban->hash);
		if (inet_pton(AF_INET, address, &ban->address) == 0) {
			if (inet_pton(AF_INET6, address, &ban->address) == 0) {
				Log_warn("Address \"%s\" is illegal!", address);
			} else {
				ban->address.ss_family = AF_INET6;
			}
		} else {
			ban->address.ss_family = AF_INET;
		}
		ban->name = strdup(name);
		ban->reason = strdup(reason);
		if (ban->name == NULL || ban->reason == NULL)
			Log_fatal("Out of memory");
		ban->time = time;
		ban->duration = duration;
		ban->mask = mask;
		list_add_tail(&ban->node, &banlist);
		bancount++;
		Log_debug("Banfile: H = '%s' A = '%s' M = %d U = '%s' R = '%s'", hexhash, address, ban->mask, ban->name, ban->reason);
	}
	fclose(file);
	Log_info("Banlist file '%s': %d entries read", getStrConf(BANFILE), bancount);
}
