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
#ifndef CLIENT_H_45786678
#define CLIENT_H_45786678

#include <TcpServer.h>

#include <stdint.h>
#include <sys/types.h>
#include <errno.h>              /* errno */

#include "list.h"
#include "types.h"
#include "messages.h"
#include "crypt.h"
#include "timer.h"
#include "pds.h"
#include "ssl.h"

#include <NetAddress.h>

#define BUFSIZE 8192
#define UDP_BUFSIZE 512
#define INACTIVITY_TIMEOUT 60 /* Seconds */
#define MAX_CODECS 10
#define MAX_TOKENSIZE 64
#define MAX_TOKENS 32
#define KEY_LENGTH sizeof(uint16_t) + 4 * sizeof(uint32_t)

#define IS_AUTH(_a_) ((_a_)->authenticated)

#ifdef __linux__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

typedef struct {
	fwRefContainer<net::TcpServerStream> stream;
	bool_t SSLready;
	bool_t shutdown_wait;
	cryptState_t cryptState;
	bool_t readBlockedOnWrite, writeBlockedOnRead;
	net::PeerAddress remote_tcp;
	net::PeerAddress remote_udp;
	std::deque<uint8_t> rcvbuf;
	uint8_t rxbuf[BUFSIZE], txbuf[BUFSIZE];
	uint32_t rxcount, msgsize, drainleft, txcount, txsize;
	int sessionId;
	uint8_t key[KEY_LENGTH];
	char *username;
	bool_t bUDP, authenticated, deaf, mute, self_deaf, self_mute, recording, bOpus;
	char *os, *release, *os_version;
	uint32_t version;
	int codec_count;
	struct dlist codecs;
	int availableBandwidth;
	etimer_t lastActivity, connectTime, idleTime;
	struct dlist node;
	struct dlist txMsgQueue;
	int txQueueCount;
	void *channel; /* Ugly... */
	char *context;
	struct dlist chan_node;
	struct dlist voicetargets;
	struct dlist tokens;
	int tokencount;
	uint8_t hash[20];
	bool_t isAdmin;
	bool_t isSuppressed;
	float UDPPingAvg, UDPPingVar, TCPPingAvg, TCPPingVar;
	uint32_t UDPPackets, TCPPackets;
} client_t;

typedef struct {
	int codec, count;
	struct dlist node;
} codec_t;

typedef struct {
	char *token;
	struct dlist node;
} token_t;

void Client_init();
int Client_getfds(struct pollfd *pollfds);
void Client_janitor();
int Client_add(fwRefContainer<net::TcpServerStream> stream, client_t** client);
int Client_read_fd(int fd);
int Client_write_fd(int fd);
int Client_send_message(client_t *client, message_t *msg);
int Client_send_message_ver(client_t *client, message_t *msg, uint32_t version);
int Client_send_message_except_ver(client_t *client, message_t *msg, uint32_t version);
int Client_count(void);
void Client_close(client_t *client);
client_t *Client_iterate(client_t **client);
int Client_send_message_except(client_t *client, message_t *msg);
int Client_read_udp(int udpsock);
void Client_disconnect_all();
int Client_voiceMsg(client_t *client, uint8_t *data, int len);
void recheckCodecVersions(client_t *connectingClient);
void Client_codec_add(client_t *client, int codec);
void Client_codec_free(client_t *client);
codec_t *Client_codec_iterate(client_t *client, codec_t **codec_itr);
void Client_textmessage(client_t *client, char *text);
bool_t Client_token_match(client_t *client, char const *str);
void Client_token_free(client_t *client);
void Client_token_add(client_t *client, char *token_string);
void Client_free(client_t *client);

#endif
