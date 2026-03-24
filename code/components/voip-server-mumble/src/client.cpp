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
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "memory.h"
#include "list.h"
#include "client.h"
#include "ssl.h"
#include "messages.h"
#include "messagehandler.h"
#include "conf.h"
#include "channel.h"
#include "version.h"
#include "voicetarget.h"
#include "ban.h"
#include "util.h"

#include "ChannelListener.h"

#include <EASTL/fixed_set.h>

extern char system_string[], version_string[];

static int Client_read(client_t *client);
static int Client_write(client_t *client);
int Client_send_udp(client_t *client, uint8_t *data, int len);
void Client_free(client_t *client);

declare_list(clients);
static int clientcount; /* = 0 */
static int maxBandwidth;
bool_t bOpus = true;

int iCodecAlpha, iCodecBeta;
bool_t bPreferAlpha;

extern int* udpsocks;
extern bool_t hasv4;

void Client_init()
{
	maxBandwidth = getIntConf(MAX_BANDWIDTH) / 8; /* From bits/s -> bytes/s */
}

int Client_count()
{
	return clientcount;
}

int Client_getfds(struct pollfd *pollfds)
{
	/*struct dlist *itr;
	int i = 0;
	list_iterate(itr, &clients) {
		client_t *c;
		c = list_get_entry(itr, client_t, node);
		pollfds[i].fd = c->tcpfd;
		pollfds[i].events = POLLIN | POLLHUP | POLLERR;
		if (c->txsize > 0 || c->readBlockedOnWrite) /* Data waiting to be sent? * /
			pollfds[i].events |= POLLOUT;
		i++;
	}
	return i;*/
	return 0;
}

// #TODO: Have servers create ETW traces of the two player_muted functions being
// used. In the worst case we are facing N calls to strncmp (previously strstr),
// and this is incredibly inefficient.
bool Client_is_player_muted(const char *namePrefix, size_t len)
{
	struct dlist *itr, *save;
	list_iterate_safe(itr, save, &clients)
	{
		client_t* c;
		c = list_get_entry(itr, client_t, node);
		if (c->username && strncmp(c->username, namePrefix, len) == 0)
		{
			return c->mute;
		}
	}

	return false;
}

void Client_set_player_muted(const char *namePrefix, size_t len, bool muted)
{
	struct dlist *itr, *save;
	list_iterate_safe(itr, save, &clients)
	{
		client_t* c;
		c = list_get_entry(itr, client_t, node);
		if (c->username && strncmp(c->username, namePrefix, len) == 0) {
			c->mute = muted;
			break;
		}
	}
}

void Client_janitor()
{
	struct dlist *itr, *save;
	int bwTop = maxBandwidth + maxBandwidth / 4;
	list_iterate_safe(itr, save, &clients) {
		client_t *c;
		c = list_get_entry(itr, client_t, node);
		Log_debug("Client %s BW available %d", c->username, c->availableBandwidth);
		c->availableBandwidth += maxBandwidth;
		if (c->availableBandwidth > bwTop)
			c->availableBandwidth = bwTop;

		if (Timer_isElapsed(&c->lastActivity, 1000000LL * INACTIVITY_TIMEOUT)) {
			/* No activity from client - assume it is lost and close. */
			Log_info_client(c, "Timeout, closing.");
			c->stream->Close();
			//Client_free(c);
		}
	}
	Ban_pruneBanned();
}

void Client_codec_add(client_t *client, int codec)
{
	codec_t *cd = (codec_t*)Memory_safeMalloc(1, sizeof(codec_t));
	init_list_entry(&cd->node);
	cd->codec = codec;
	list_add_tail(&cd->node, &client->codecs);
}

void Client_codec_free(client_t *client)
{
	struct dlist *itr, *save;
	list_iterate_safe(itr, save, &client->codecs) {
		list_del(&list_get_entry(itr, codec_t, node)->node);
		free(list_get_entry(itr, codec_t, node));
	}
}

codec_t *Client_codec_iterate(client_t *client, codec_t **codec_itr)
{
	codec_t *cd = *codec_itr;

	if (list_empty(&client->codecs))
		return NULL;

	if (cd == NULL) {
		cd = list_get_entry(list_get_first(&client->codecs), codec_t, node);
	} else {
		if (list_get_next(&cd->node) == &client->codecs)
			cd = NULL;
		else
			cd = list_get_entry(list_get_next(&cd->node), codec_t, node);
	}
	*codec_itr = cd;
	return cd;
}

void Client_token_add(client_t *client, char *token_string)
{
	token_t *token;

	if (client->tokencount >= MAX_TOKENS)
		return;
	token = (token_t*)Memory_safeMalloc(1, sizeof(token_t));
	init_list_entry(&token->node);
	token->token = strdup(token_string);
	if (token->token == NULL)
		Log_fatal("Out of memory");
	list_add_tail(&token->node, &client->tokens);
	client->tokencount++;
}

bool_t Client_token_match(client_t *client, char const *str)
{
	token_t *token;
	struct dlist *itr;

	if (list_empty(&client->tokens))
		return false;
	list_iterate(itr, &client->tokens) {
		token = list_get_entry(itr, token_t, node);
		if (strnicmp(token->token, str, MAX_TOKENSIZE) == 0)
			return true;
	}
	return false;
}

void Client_token_free(client_t *client)
{
	struct dlist *itr, *save;
	token_t *token;

	list_iterate_safe(itr, save, &client->tokens) {
		token = list_get_entry(itr, token_t, node);
		list_del(&token->node);
		free(token->token);
		free(token);
	}
	client->tokencount = 0;
}


#define OPUS_WARN_USING "<strong>WARNING:</strong> Your client doesn't support the Opus codec the server is using, you won't be able to talk or hear anyone. Please upgrade your Mumble client."
#define OPUS_WARN_SWITCHING "<strong>WARNING:</strong> Your client doesn't support the Opus codec the server is switching to, you won't be able to talk or hear anyone. Please upgrade your Mumble client."
void recheckCodecVersions(client_t *connectingClient)
{
	client_t *client_itr = NULL;
	int max = 0, version, current_version;
	int users = 0, opus = 0;
	message_t *sendmsg;
	struct dlist codec_list, *itr, *save;
	codec_t *codec_itr, *cd;
	bool_t found;
	bool_t enableOpus;

	init_list_entry(&codec_list);

	while (Client_iterate(&client_itr) != NULL) {
		codec_itr = NULL;
		if (client_itr->codec_count == 0 && !client_itr->bOpus)
			continue;
		while (Client_codec_iterate(client_itr, &codec_itr) != NULL) {
			found = false;
			list_iterate(itr, &codec_list) {
				cd = list_get_entry(itr, codec_t, node);
				if (cd->codec == codec_itr->codec) {
					cd->count++;
					found = true;
				}
			}
			if (!found) {
				cd = (codec_t*)Memory_safeMalloc(1, sizeof(codec_t));
				memset(cd, 0, sizeof(codec_t));
				init_list_entry(&cd->node);
				cd->codec = codec_itr->codec;
				cd->count = 1;
				list_add_tail(&cd->node, &codec_list);
			}
		}
		users++;
		if (client_itr->bOpus)
			opus++;
	}
	if (users == 0)
		return;

	enableOpus = ((opus * 100 / users) >= getIntConf(OPUS_THRESHOLD));

	list_iterate(itr, &codec_list) {
		cd = list_get_entry(itr, codec_t, node);
		if (cd->count > max) {
			max = cd->count;
			version = cd->codec;
		}
	}
	list_iterate_safe(itr, save, &codec_list) {
		list_del(&list_get_entry(itr, codec_t, node)->node);
		free(list_get_entry(itr, codec_t, node));
	}

	current_version = bPreferAlpha ? iCodecAlpha : iCodecBeta;
	if (current_version != version) {
		// If we don't already use the compat bitstream version set
		// it as alpha and announce it. If another codec now got the
		// majority set it as the opposite of the currently valid bPreferAlpha
		// and announce it.
		if (version == (uint32_t)0x8000000b)
			bPreferAlpha = true;
		else
			bPreferAlpha = !bPreferAlpha;

		if (bPreferAlpha)
			iCodecAlpha = version;
		else
			iCodecBeta = version;
	} else if (bOpus && enableOpus) {
		if (connectingClient && !connectingClient->bOpus)
			Client_textmessage(connectingClient, OPUS_WARN_USING);
		return;
	}

	sendmsg = Msg_create(CodecVersion);
	sendmsg->payload.codecVersion->set_alpha(iCodecAlpha);
	sendmsg->payload.codecVersion->set_beta(iCodecBeta);
	sendmsg->payload.codecVersion->set_prefer_alpha(bPreferAlpha);
	sendmsg->payload.codecVersion->set_opus(enableOpus);

	Client_send_message_except(NULL, sendmsg);

	if (enableOpus && !bOpus) {
		client_itr = NULL;
		while (Client_iterate(&client_itr) != NULL) {
			if ((client_itr->authenticated || client_itr == connectingClient) &&
				!client_itr->bOpus) {
				Client_textmessage(client_itr, OPUS_WARN_SWITCHING);
			}
		}
		Log_info("OPUS codec %s", bOpus ? "enabled" : "disabled");
	}

	bOpus = enableOpus;
}

static int findFreeSessionId()
{
	int id;
	client_t *itr = NULL;

	for (id = 1; id < INT_MAX; id++) {
		itr = NULL;
		while ((itr = Client_iterate(&itr)) != NULL) {
			if (itr->sessionId == id)
				break;
		}
		if (itr == NULL) /* Found free id */
			return id;
	}
	return -1;
}

int Client_add(fwRefContainer<net::TcpServerStream> stream, client_t** client)
{
	client_t* newclient;
	message_t *sendmsg;
	char* addressString = NULL;

	/*if (Ban_isBannedAddr(remote)) {
		addressString = Util_addressToString(remote);
		Log_info("Address %s banned. Disconnecting", addressString);
		free(addressString);
		return -1;
	}*/

	newclient = new client_t();

	*client = newclient;

	newclient->numFailedCrypt = 0;

	newclient->stream = stream;
	//memcpy(&newclient->remote_tcp, remote, sizeof(struct sockaddr_storage));
	newclient->remote_tcp = stream->GetPeerAddress();
	/*newclient->ssl = SSLi_newconnection(&newclient->tcpfd, &newclient->SSLready);
	if (newclient->ssl == NULL) {
		addressString = Util_addressToString(remote);
		Log_warn("SSL negotiation failed with %s on port %d", addressString, Util_addressToPort(remote));
		free(addressString);
		free(newclient);
		return -1;
	}*/
	newclient->availableBandwidth = maxBandwidth;
	Timer_init(&newclient->lastActivity);
	Timer_init(&newclient->connectTime);
	Timer_init(&newclient->idleTime);
	newclient->sessionId = findFreeSessionId();
	if (newclient->sessionId < 0)
		Log_fatal("Could not find a free session ID");

	init_list_entry(&newclient->txMsgQueue);
	init_list_entry(&newclient->chan_node);
	init_list_entry(&newclient->node);
	init_list_entry(&newclient->voicetargets);
	init_list_entry(&newclient->codecs);
	init_list_entry(&newclient->tokens);

	list_add_tail(&newclient->node, &clients);
	clientcount++;

	/* Send version message to client */
	sendmsg = Msg_create(Version);
	sendmsg->payload.version->set_version(PROTOCOL_VERSION);
	sendmsg->payload.version->set_release(UMURMUR_VERSION);
	sendmsg->payload.version->set_os("CitizenFX Core");
	sendmsg->payload.version->set_os_version("Integrated Mumble server for the CitizenFX Framework.");
	Client_send_message(newclient, sendmsg);

	return 0;
}

void Client_free(client_t *client)
{
	struct dlist *itr, *save;
	message_t *sendmsg;
	bool_t authenticatedLeft = client->authenticated;

	if (client->authenticated) {
		int leave_id;
		leave_id = Chan_userLeave(client);
		if (leave_id > 0) { /* Remove temp channel */
			sendmsg = Msg_create(ChannelRemove);
			sendmsg->payload.channelRemove->set_channel_id(leave_id);
			Client_send_message_except(client, sendmsg);
		}
		sendmsg = Msg_create(UserRemove);
		sendmsg->payload.userRemove->set_session(client->sessionId);
		Client_send_message_except(client, sendmsg);
	}
	list_iterate_safe(itr, save, &client->txMsgQueue) {
		list_del(&list_get_entry(itr, message_t, node)->node);
		Msg_free(list_get_entry(itr, message_t, node));
	}
	Client_codec_free(client);
	Voicetarget_free_all(client);
	Client_token_free(client);

	list_del(&client->node);
	/*if (client->ssl)
		SSLi_free(client->ssl);
	close(client->tcpfd);*/
	clientcount--;
	if (client->release)
		free(client->release);
	if (client->os)
		free(client->os);
	if (client->os_version)
		free(client->os_version);
	if (client->username)
		free(client->username);
	if (client->context)
		free(client->context);
	delete client;

	if (authenticatedLeft)
		recheckCodecVersions(NULL); /* Can use better codec now? */
}

int Client_getPlayerId(client_t* client, char* username)
{
	int playerId = -1;
	if(!username)
	{
		return -1;
	}
	if (strncmp(username, "[", 1) == 0) {
		char *endptr;
		playerId = strtol(username + 1, &endptr, 10);
		if (*endptr != ']') {
			playerId = -1;
		}
	}
	return playerId;
}

void Client_close(client_t *client)
{
	//SSLi_shutdown(client->ssl);
	client->shutdown_wait = true;
}

void Client_disconnect_all()
{
	struct dlist *itr, *save;

	list_iterate_safe(itr, save, &clients) {
		Client_free(list_get_entry(itr, client_t, node));
	}
}

int Client_read_fd(int fd)
{
	return -1;
}

int Client_read(client_t *client)
{
#if 0
	int rc;

	Timer_restart(&client->lastActivity);

	if (client->writeBlockedOnRead) {
		client->writeBlockedOnRead = false;
		Log_debug("Client_read: writeBlockedOnRead == true");
		return Client_write(client);
	}

	if (client->shutdown_wait) {
		Client_free(client);
		return 0;
	}
	if (!client->SSLready) {
		int rc;
		rc = SSLi_nonblockaccept(client->ssl, &client->SSLready);
		if (rc < 0) {
			Client_free(client);
			return -1;
		}
	}

	do {
		errno = 0;
		if (!client->msgsize)
			rc = SSLi_read(client->ssl, &client->rxbuf[client->rxcount], 6 - client->rxcount);
		else
			rc = SSLi_read(client->ssl, &client->rxbuf[client->rxcount], client->msgsize);
		if (rc > 0) {
			message_t *msg;
			client->rxcount += rc;
			if (!client->msgsize && client->rxcount >= 6) {
				uint32_t msgLen;
				memcpy(&msgLen, &client->rxbuf[2], sizeof(uint32_t));
				client->msgsize = ntohl(msgLen);
			}
			if (client->msgsize > BUFSIZE - 6) {
				/* XXX - figure out how to handle this. A large size here can represent two cases:
				 * 1. A valid size. The only message that is this big is UserState message with a big texture
				 * 2. An invalid size = protocol error, e.g. connecting with a 1.1.x client
				 */
				//		  Log_warn("Too big message received (%d bytes). Playing safe and disconnecting client %s:%d",
				//			   client->msgsize, inet_ntoa(client->remote_tcp.sin_addr), ntohs(client->remote_tcp.sin_port));
				Client_free(client);
				return -1;
				/* client->rxcount = client->msgsize = 0; */
			}
			else if (client->rxcount == client->msgsize + 6) { /* Got all of the message */
				msg = Msg_networkToMessage(client->rxbuf, client->msgsize + 6);
				/* pass messsage to handler */
				if (msg)
					Mh_handle_message(client, msg);
				client->rxcount = client->msgsize = 0;
			}
		} else /* rc <= 0 */ {
			if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_WANT_READ) {
				return 0;
			}
			else if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_WANT_WRITE) {
				client->readBlockedOnWrite = true;
				return 0;
			}
			else if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_ZERO_RETURN ||
				SSLi_get_error(client->ssl, rc) == 0) {
				Log_info_client(client, "Connection closed by peer");
				Client_close(client);
			}
			else {
				if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_SYSCALL) {
					if (errno == 0)
						Log_info_client(client, "Connection closed by peer");
					else
						Log_info_client(client,"Error: %s  - Closing connection (code %d)",
							strerror(errno));
				}
				else if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_CONNRESET) {
					Log_info_client(client, "Connection reset by peer");
				}
				else {
					Log_info_client(client, "SSL error: %d - Closing connection", SSLi_get_error(client->ssl, rc));
				}
				Client_free(client);
				return -1;
			}
		}
	} while (SSLi_data_pending(client->ssl));

	return 0;
#endif
	return 0;
}

int Client_write_fd(int fd)
{
	return -1;
}

int Client_write(client_t *client)
{
#if 0
	int rc;

	if (client->readBlockedOnWrite) {
		client->readBlockedOnWrite = false;
		Log_debug("Client_write: readBlockedOnWrite == true");
		return Client_read(client);
	}
	rc = SSLi_write(client->ssl, &client->txbuf[client->txcount], client->txsize - client->txcount);
	if (rc > 0) {
		client->txcount += rc;
		if (client->txcount == client->txsize)
			client->txsize = client->txcount = 0;
	}
	else if (rc < 0) {
		if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_WANT_READ) {
			client->writeBlockedOnRead = true;
			return 0;
		}
		else if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_WANT_WRITE) {
			return 0;
		}
		else {
			if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_SYSCALL) {
				Log_info_client(client, "Error: %s	- Closing connection", strerror(errno));
			}
			else if (SSLi_get_error(client->ssl, rc) == SSLI_ERROR_CONNRESET) {
				Log_info_client(client, "Connection reset by peer");
			}
			else {
				Log_info_client(client, "SSL error: %d - Closing connection.", SSLi_get_error(client->ssl, rc));
			}
			Client_free(client);
			return -1;
		}
	}
#endif

	if (client->txsize > 0)
	{
		std::vector<uint8_t> vec(client->txsize);
		memcpy(vec.data(), client->txbuf, vec.size());

		client->stream->Write(vec);
		client->txsize = client->txcount = 0;
	}

	if (client->txsize == 0 && !list_empty(&client->txMsgQueue)) {
		message_t *msg;
		msg = list_get_entry(list_get_first(&client->txMsgQueue), message_t, node);
		list_del(list_get_first(&client->txMsgQueue));
		client->txQueueCount--;
		Client_send_message(client, msg);
	}
	return 0;
}

int Client_send_message_ver(client_t *client, message_t *msg, uint32_t version)
{
	if ((version == 0) || (client->version >= version) ||
		((version & 0x80000000) && (client->version < (~version))))
		return Client_send_message(client, msg);
	else
		Msg_free(msg);
	return -1;
}

int Client_send_message(client_t *client, message_t *msg)
{
	if (!client->authenticated && msg->messageType != Version) {
		Msg_free(msg);
		return 0;
	}
	if (client->txsize != 0) {
		/* Queue message */
		if ((client->txQueueCount > 25 &&  msg->messageType == UDPTunnel) ||
			client->txQueueCount > 150) {
			Msg_free(msg);
			return -1;
		}
		client->txQueueCount++;
		list_add_tail(&msg->node, &client->txMsgQueue);
		Log_debug("Queueing message");
	} else {
		int len;
		len = Msg_messageToNetwork(msg, client->txbuf);
		doAssert(len < BUFSIZE);

		client->txsize = len;
		client->txcount = 0;
		Client_write(client);
		Msg_free(msg);
	}
	return 0;
}

client_t *Client_iterate(client_t **client_itr)
{
	client_t *c = *client_itr;

	if (list_empty(&clients))
		return NULL;

	if (c == NULL) {
		c = list_get_entry(list_get_first(&clients), client_t, node);
	} else {
		if (list_get_next(&c->node) == &clients)
			c = NULL;
		else
			c = list_get_entry(list_get_next(&c->node), client_t, node);
	}
	*client_itr = c;
	return c;
}

void Client_textmessage(client_t *client, char *text)
{
	char *message;
	uint32_t *tree_id;
	message_t *sendmsg = NULL;

	message = (char*)Memory_safeMalloc(1, strlen(text) + 1);
	sendmsg = Msg_create(TextMessage);
	sendmsg->payload.textMessage->set_message(message);
	sendmsg->payload.textMessage->add_tree_id(0);
	strcpy(message, text);
	Client_send_message(client, sendmsg);
	free(message);
}


int Client_send_message_except(client_t *client, message_t *msg)
{
	client_t *itr = NULL;
	int count = 0;

	Msg_inc_ref(msg); /* Make sure a reference is held during the whole iteration. */
	while (Client_iterate(&itr) != NULL) {
		if (itr != client) {
			if (count++ > 0)
				Msg_inc_ref(msg); /* One extra reference for each new copy */
			Log_debug("Msg %d to %s refcount %d",  msg->messageType, itr->username, msg->refcount);
			Client_send_message(itr, msg);
		}
	}
	Msg_free(msg); /* Free our reference to the message */

	if (count == 0)
		Msg_free(msg); /* If only 1 client is connected then no message is passed
						* to Client_send_message(). Free it here. */

	return 0;
}

int Client_send_message_except_ver(client_t *client, message_t *msg, uint32_t version)
{
	client_t *itr = NULL;
	int count = 0;

	Msg_inc_ref(msg); /* Make sure a reference is held during the whole iteration. */
	while (Client_iterate(&itr) != NULL) {
		if (itr != client) {
			if (count++ > 0)
				Msg_inc_ref(msg); /* One extra reference for each new copy */
			Log_debug("Msg %d to %s refcount %d",  msg->messageType, itr->username, msg->refcount);
			Client_send_message_ver(itr, msg, version);
		}
	}
	Msg_free(msg); /* Free our reference to the message */

	if (count == 0)
		Msg_free(msg); /* If only 1 client is connected then no message is passed
						* to Client_send_message(). Free it here. */

	return 0;
}

bool_t checkDecrypt(client_t *client, const uint8_t *encrypted, uint8_t *plain, unsigned int len)
{
	if (CryptState_isValid(&client->cryptState) &&
		CryptState_decrypt(&client->cryptState, encrypted, plain, len))
		return true;

	if (Timer_elapsed(&client->cryptState.tLastGood) > 5000000ULL) {
		if (Timer_elapsed(&client->cryptState.tLastRequest) > 5000000ULL) {
			message_t *sendmsg;
			Timer_restart(&client->cryptState.tLastRequest);

			sendmsg = Msg_create(CryptSetup);
			Log_info_client(client, "Requesting voice channel crypt resync");
			Client_send_message(client, sendmsg);
		}
	}
	return false;
}

#define UDP_PACKET_SIZE 1024
int Client_read_udp(int udpsock)
{
	int len;
	struct sockaddr_storage from;
	socklen_t fromlen = sizeof(struct sockaddr_storage);
	uint8_t key[KEY_LENGTH];
	client_t *itr;
	UDPMessageType_t msgType;
	uint8_t fromaddress[4 * sizeof(int32_t)];
	uint16_t fromport;
	char *clientAddressString = NULL;

#if defined(__LP64__)
	uint8_t encbuff[UDP_PACKET_SIZE + 8];
	uint8_t *encrypted = encbuff + 4;
#else
	uint8_t encrypted[UDP_PACKET_SIZE];
#endif
	uint8_t buffer[UDP_PACKET_SIZE];

	len = recvfrom(udpsock, (char*)encrypted, UDP_PACKET_SIZE, MSG_TRUNC, (struct sockaddr *)&from, &fromlen);

	memset(key, 0, KEY_LENGTH);

	fromport = Util_addressToPort(&from);

	if(from.ss_family == AF_INET) {
		memcpy(fromaddress, &((struct sockaddr_in*)&from)->sin_addr, sizeof(int32_t));
		memcpy(&key[0], &fromport, 2);
		memcpy(&key[2], fromaddress, sizeof(int32_t));
	} else {
		memcpy(fromaddress, &((struct sockaddr_in6*)&from)->sin6_addr, 4 * sizeof(int32_t));
		memcpy(&key[0], &fromport, 2);
		memcpy(&key[2], fromaddress, 4 * sizeof(int32_t));
	}

	if (len <= 0)
		return -1;
	else if (len < 5 || len > UDP_PACKET_SIZE) /* 4 bytes crypt header + type + session */
		return 0;

	/*
	 * Reply to ping packet
	 * The second and third uint32_t are the timestamp, which will be returned unmodified
	 */
	if (len == 12 && *encrypted == 0) {
		uint32_t *ping = (uint32_t *)encrypted;
		ping[0] = htonl((uint32_t)PROTOCOL_VERSION);
		ping[3] = htonl((uint32_t)clientcount);
		ping[4] = htonl((uint32_t)getIntConf(MUMBLE_MAX_CLIENTS));
		ping[5] = htonl((uint32_t)getIntConf(MAX_BANDWIDTH));

		sendto(udpsock, (char*)encrypted, 6 * sizeof(uint32_t), 0, (struct sockaddr *)&from, fromlen);
		return 0;
	}

	itr = NULL;

	while (Client_iterate(&itr) != NULL) {
		if (memcmp(itr->key, key, KEY_LENGTH) == 0) {
			if (!checkDecrypt(itr, encrypted, buffer, len))
				goto out;
			break;
		}
	}
	if (itr == NULL) { /* Unknown peer */
		struct sockaddr_storage itraddressstorage;
		uint8_t itraddress[4 * sizeof(int32_t)];
		int addresslength;

		while (Client_iterate(&itr) != NULL) {
			itraddressstorage = *(sockaddr_storage*)itr->remote_tcp.GetSocketAddress();
			if(itraddressstorage.ss_family == AF_INET) {
				memcpy(itraddress, &((struct sockaddr_in*)&from)->sin_addr, sizeof(int32_t));
				addresslength = sizeof(int32_t);
			} else {
				memcpy(itraddress, &((struct sockaddr_in6*)&from)->sin6_addr, 4 * sizeof(int32_t));
				addresslength = 4 * sizeof(int32_t);
			}

			if (memcmp(itraddress, fromaddress, addresslength) == 0) {
				if (checkDecrypt(itr, encrypted, buffer, len)) {
					memcpy(itr->key, key, KEY_LENGTH);
					char* clientAddressString = Util_clientAddressToString(itr);
					Log_info_client(itr, "New UDP connection from %s on port %d", clientAddressString, fromport);
					free(clientAddressString);
					memcpy(&itr->remote_udp, &from, sizeof(struct sockaddr_storage));
					break;
				}
			}
		} /* while */
	}

	if (itr == NULL) { /* Couldn't find this peer among connected clients */
		goto out;
	}

	itr->bUDP = true;
	len -= 4; /* Adjust for crypt header */
	msgType = (UDPMessageType_t)((buffer[0] >> 5) & 0x7);

	switch (msgType) {
		case UDPVoiceSpeex:
		case UDPVoiceCELTAlpha:
		case UDPVoiceCELTBeta:
			if (bOpus)
				break;
		case UDPVoiceOpus:
			Client_voiceMsg(itr, buffer, len);
			break;
		case UDPPing:
			Log_debug("UDP Ping reply len %d", len);
			Client_send_udp(itr, buffer, len);
			break;
		default:
			clientAddressString = Util_clientAddressToString(itr);
			Log_debug("Unknown UDP message type from %s port %d", clientAddressString, fromport);
			free(clientAddressString);
			break;
	}

out:
	return 0;
}

static inline void Client_send_voice(client_t *src, client_t *dst, uint8_t *data, int len, int poslen)
{
	if (IS_AUTH(dst) && dst != src && !dst->deaf && !dst->self_deaf) {
		if (poslen > 0 && /* Has positional data */
			src->context != NULL && dst->context != NULL && /* ...both source and destination has context */
			strcmp(src->context, dst->context) == 0) /* ...and the contexts match */
			Client_send_udp(dst, data, len);
		else
			Client_send_udp(dst, data, len - poslen);
	}
}

/* Handle decrypted voice message */
int Client_voiceMsg(client_t *client, uint8_t *data, int len)
{
	uint8_t buffer[UDP_PACKET_SIZE];
	pds_t *pdi = Pds_create(data + 1, len - 1);
	pds_t *pds = Pds_create(buffer + 1, UDP_PACKET_SIZE - 1);
	unsigned int type = data[0] & 0xe0;
	unsigned int target = data[0] & 0x1f;
	unsigned int poslen, counter, size;
	int offset, packetsize;
	voicetarget_t *vt;

	channel_t *ch = (channel_t *)client->channel;
	struct dlist *itr;

	eastl::fixed_set<client_t*, 256> listeningUsers;

	if (!client->authenticated || client->mute || client->self_mute || !ch || ch->silent)
		goto out;

	packetsize = 20 + 8 + 4 + len;
	if (client->availableBandwidth - packetsize < 0)
		goto out; /* Discard */
	client->availableBandwidth -= packetsize;

	Timer_restart(&client->idleTime);
	Timer_restart(&client->lastActivity);

	counter = Pds_get_numval(pdi); /* step past session id */
	if ((type >> 5) != UDPVoiceOpus) {
		do {
			counter = Pds_next8(pdi);
			offset = Pds_skip(pdi, counter & 0x7f);
		} while ((counter & 0x80) && offset > 0);
	} else {
		size = Pds_get_numval(pdi);
		Pds_skip(pdi, size & 0x1fff);
	}

	poslen = pdi->maxsize - pdi->offset; /* For stripping of positional info */

	Pds_add_numval(pds, client->sessionId);
	Pds_append_data_nosize(pds, data + 1, len - 1);

	if (target == 0x1f) { /* Loopback */
		buffer[0] = (uint8_t) type;
		Client_send_udp(client, buffer, pds->offset + 1);
	}
	else if (target == 0) { /* regular channel speech */
		buffer[0] = (uint8_t) type;

		if (ch == NULL)
			goto out;

		list_iterate(itr, &ch->clients) {
			client_t *c;
			c = list_get_entry(itr, client_t, chan_node);
			listeningUsers.insert(c);
		}

		for (unsigned int currentSession : ChannelListener::getListenersForChannel(ch))
		{
			client_t* pDst = NULL;

			while (Client_iterate(&pDst) != NULL)
			{
				if (pDst->sessionId == currentSession)
					break;
			}

			if (pDst)
			{
				listeningUsers.insert(pDst);
			}
		}
	} else if ((vt = Voicetarget_get_id(client, target)) != NULL) { /* Targeted whisper */
		int i;
		channel_t *ch;
		eastl::fixed_set<int, 512> targeted_channels;

		/* Channels */
		for (i = 0; i < TARGET_MAX_CHANNELS && vt->channels[i].channel != -1; i++) {
			buffer[0] = (uint8_t) (type | 1);
			Log_debug("Whisper channel %d", vt->channels[i]);
			ch = Chan_fromId(vt->channels[i].channel);
			if (ch == NULL)
				continue;
			targeted_channels.emplace(ch->id);
			list_iterate(itr, &ch->clients) {
				client_t *c;
				c = list_get_entry(itr, client_t, chan_node);
				listeningUsers.insert(c);
			}

			// Send audio to all users that are listening to the channel
			for (unsigned int currentSession : ChannelListener::getListenersForChannel(ch))
			{
				client_t* pDst = NULL;

				while (Client_iterate(&pDst) != NULL)
				{
					if (pDst->sessionId == currentSession)
						break;
				}

				if (pDst)
				{
					listeningUsers.insert(pDst);
				}
			}

			/* Channel links */
			if (vt->channels[i].linked && !list_empty(&ch->channel_links)) {
				struct dlist *ch_itr;
				list_iterate(ch_itr, &ch->channel_links) {
					channellist_t *chl;
					channel_t *ch_link;
					chl = list_get_entry(ch_itr, channellist_t, node);
					ch_link = chl->chan;

					// channel listener iteration
					for (unsigned int currentSession : ChannelListener::getListenersForChannel(ch_link))
					{
						client_t* pDst = NULL;

						while (Client_iterate(&pDst) != NULL)
						{
							if (pDst->sessionId == currentSession)
								break;
						}

						if (pDst && pDst->channel != ch && !ChannelListener::isListening(pDst, ch))
						{
							listeningUsers.insert(pDst);
						}
					}

					list_iterate(itr, &ch_link->clients) {
						client_t *c;
						c = list_get_entry(itr, client_t, chan_node);

						if (!ChannelListener::isListening(c, ch_link))
						{
							Log_debug("Linked voice from %s -> %s", ch->name, ch_link->name);
							listeningUsers.insert(c);
						}
					}
				}
			}
			/* children */
			if (vt->channels[i].children) {
				struct dlist chanlist, *ch_itr;
				init_list_entry(&chanlist);
				Chan_buildTreeList(ch, &chanlist);
				list_iterate(ch_itr, &chanlist) {
					channel_t *sub;
					sub = list_get_entry(ch_itr, channellist_t, node)->chan;
					list_iterate(itr, &sub->clients) {
						client_t *c;
						c = list_get_entry(itr, client_t, chan_node);
						Log_debug("Child voice from %s -> %s", ch->name, sub->name);
						listeningUsers.insert(c);
					}
				}
				Chan_freeTreeList(&chanlist);
			}
		}
		/* Sessions */
		for (i = 0; i < TARGET_MAX_SESSIONS && vt->sessions[i] != -1; i++) {
			client_t *c = NULL;
			buffer[0] = (uint8_t) (type | 2);
			Log_debug("Whisper session %d", vt->sessions[i]);
			while (Client_iterate(&c) != NULL) {
				channel_t* ch = (channel_t*)c->channel;
				if ((!ch || targeted_channels.find(ch->id) == targeted_channels.end()) && c->sessionId == vt->sessions[i]) {
					listeningUsers.insert(c);
					break;
				}
			}
		}
	}

	// Send the audio to all listening users
	for (client_t* c : listeningUsers)
	{
		buffer[0] = (uint8_t)(type | 3); // SpeechFlags::Listen
		Client_send_voice(client, c, buffer, pds->offset + 1, poslen);
	}

out:
	Pds_free(pds);
	Pds_free(pdi);

	return 0;
}

int Client_send_udp(client_t *client, uint8_t *data, int len)
{
	uint8_t *buf, *mbuf;

	int udpsock = 0;// (client->remote_udp.GetAddressFamily() == AF_INET) ? udpsocks[0] : udpsocks[(hasv4) ? 1 : 0];

	if (CryptState_isValid(&client->cryptState) &&
		client->bUDP) {
#if defined(__LP64__)
		buf = mbuf = (uint8_t*)Memory_safeMalloc(1, len + 4 + 16);
		buf += 4;
#else
		mbuf = buf = (uint8_t*)Memory_safeMalloc(1, len + 4);
#endif
		CryptState_encrypt(&client->cryptState, data, buf, len);

/*#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
			sendto(udpsock, buf, len + 4, 0, (struct sockaddr *)&client->remote_udp, client->remote_tcp.ss_len);
#else
			sendto(udpsock, (char*)buf, len + 4, 0, (struct sockaddr *)&client->remote_udp, sizeof(struct sockaddr_storage));
#endif*/

		client->interceptor->Send(client->remote_udp, buf, len + 4);

		free(mbuf);
	} else {
		message_t *msg;
		msg = Msg_CreateVoiceMsg(data, len);
		Client_send_message(client, msg);
	}
	return 0;
}
