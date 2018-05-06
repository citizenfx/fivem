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

#include "log.h"
#include "memory.h"
#include "list.h"
#include "client.h"
#include "messages.h"
#include "messagehandler.h"
#include "crypt.h"
#include "channel.h"
#include "conf.h"
#include "voicetarget.h"
#include "ban.h"

#define MAX_TEXT 512
#define MAX_USERNAME 128

#define NO_CELT_MESSAGE "<strong>WARNING:</strong> Your client doesn't support the CELT codec, you won't be able to talk to or hear most clients. Please make sure your client was built with CELT support."


extern channel_t *defaultChan;
extern int iCodecAlpha, iCodecBeta;
extern bool_t bPreferAlpha, bOpus;

static bool_t fake_celt_support;

static void sendServerReject(client_t *client, const char *reason, MumbleProto::Reject::RejectType type)
{
	message_t *msg = Msg_create(Reject);
	msg->payload.reject->set_reason(reason);
	msg->payload.reject->set_type(type);
	Client_send_message(client, msg);

	Log_info_client(client, "Server reject reason: %s", reason);
}

static void sendPermissionDenied(client_t *client, const char *reason)
{
	message_t *msg = Msg_create(PermissionDenied);
	msg->payload.permissionDenied->set_type(MumbleProto::PermissionDenied_DenyType_Text);
	msg->payload.permissionDenied->set_reason(strdup(reason));
	Client_send_message(client, msg);
}

static void addTokens(client_t *client, message_t *msg)
{
	int i;
	if (client->tokencount + msg->payload.authenticate->tokens_size() < MAX_TOKENS) {
		/* Check lengths first */
		for (i = 0; i < msg->payload.authenticate->tokens_size(); i++) {
			if (strlen(msg->payload.authenticate->tokens(i).c_str()) > MAX_TOKENSIZE - 1) {
				sendPermissionDenied(client, "Too long token");
				return;
			}
		}

		for (i = 0; i < msg->payload.authenticate->tokens_size(); i++) {
			Log_debug("Adding token '%s' to client '%s'", msg->payload.authenticate->tokens(i).c_str(), client->username);
			Client_token_add(client, (char*)msg->payload.authenticate->tokens(i).c_str());
		}
	}
	else
		sendPermissionDenied(client, "Too many tokens");
}

void Mh_handle_message(client_t *client, message_t *msg)
{
	message_t *sendmsg = NULL;
	channel_t *ch_itr = NULL;
	client_t *client_itr, *target;

	if (!client->authenticated && !(msg->messageType == Authenticate ||
									msg->messageType == Version)) {
		goto out;
	}

	switch (msg->messageType) {
	case UDPTunnel:
	case Ping:
	case CryptSetup:
	case VoiceTarget:
	case UserStats:
	case PermissionQuery:
		break;
	default:
		Timer_restart(&client->idleTime);
	}

	switch (msg->messageType) {
	case Authenticate:
		Log_debug("Authenticate message received");

		if (IS_AUTH(client) || !msg->payload.authenticate->username().c_str()) {
			/* Authenticate message might be sent when a tokens are changed by the user.*/
			Client_token_free(client); /* Clear the token list */
			if (msg->payload.authenticate->tokens_size() > 0) {
				Log_debug("Tokens in auth message from '%s'. n_tokens = %d", client->username,
				          msg->payload.authenticate->tokens_size());
				addTokens(client, msg);
			}
			break;
		}

		/*if (SSLi_getSHA1Hash(client->ssl, client->hash) && Ban_isBanned(client)) {
			char hexhash[41];
			SSLi_hash2hex(client->hash, hexhash);
			Log_info("Client with hash '%s' is banned. Disconnecting", hexhash);
			goto disconnect;
		}*/

		client->authenticated = true;

		client_itr = NULL;
		while (Client_iterate(&client_itr) != NULL) {
			if (!IS_AUTH(client_itr))
				continue;
			if (client_itr->username && strncmp(client_itr->username, msg->payload.authenticate->username().c_str(), MAX_USERNAME) == 0) {
				char buf[64];
				sprintf(buf, "Username already in use");
				Log_debug("Username already in use");
				sendServerReject(client, buf, MumbleProto::Reject_RejectType_UsernameInUse);
				goto disconnect;
			}
		}
		if (strlen(getStrConf(PASSPHRASE)) > 0) {
			if (!msg->payload.authenticate->password().c_str() ||
				(msg->payload.authenticate->password().c_str() &&
				 strncmp(getStrConf(PASSPHRASE), msg->payload.authenticate->password().c_str(), MAX_TEXT) != 0)) {
				char buf[64];
				sprintf(buf, "Wrong server password");
				sendServerReject(client, buf, MumbleProto::Reject_RejectType_WrongServerPW);
				Log_debug("Wrong server password: '%s'", msg->payload.authenticate->has_password() ?
						  msg->payload.authenticate->password().c_str() : "(null)");
				goto disconnect;
			}
		}
		if (strlen(msg->payload.authenticate->username().c_str()) == 0 ||
			strlen(msg->payload.authenticate->username().c_str()) >= MAX_USERNAME) { /* XXX - other invalid names? */
			char buf[64];
			sprintf(buf, "Invalid username");
			Log_debug("Invalid username");
			sendServerReject(client, buf, MumbleProto::Reject_RejectType_InvalidUsername);
			goto disconnect;
		}

		if (Client_count() >= getIntConf(MAX_CLIENTS)) {
			char buf[64];
			snprintf(buf, 64, "Server is full (max %d users)", getIntConf(MAX_CLIENTS));
			sendServerReject(client, buf, MumbleProto::Reject_RejectType_ServerFull);
			goto disconnect;
		}

		/* Name */
		client->username = strdup(msg->payload.authenticate->username().c_str());

		/* Tokens */
		if (msg->payload.authenticate->tokens_size() > 0)
			addTokens(client, msg);

		/* Check if admin PW among tokens */
		if (strlen(getStrConf(ADMIN_PASSPHRASE)) > 0 &&
		    Client_token_match(client, getStrConf(ADMIN_PASSPHRASE))) {
			client->isAdmin = true;
			Log_info_client(client, "User provided admin password");
		}

		/* Setup UDP encryption */
		CryptState_init(&client->cryptState);
		CryptState_genKey(&client->cryptState);
		sendmsg = Msg_create(CryptSetup);
		sendmsg->payload.cryptSetup->set_key(client->cryptState.raw_key, AES_BLOCK_SIZE);
		sendmsg->payload.cryptSetup->set_server_nonce(client->cryptState.encrypt_iv, AES_BLOCK_SIZE);
		sendmsg->payload.cryptSetup->set_client_nonce(client->cryptState.decrypt_iv, AES_BLOCK_SIZE);
		Client_send_message(client, sendmsg);

		/* Channel stuff */
		Chan_userJoin(defaultChan, client); /* Join default channel */

		/* Codec version */
		Log_debug("Client %d has %d CELT codecs", client->sessionId,
				  msg->payload.authenticate->celt_versions_size());
		if (msg->payload.authenticate->celt_versions_size() > 0) {
			int i;
			codec_t *codec_itr;
			client->codec_count = msg->payload.authenticate->celt_versions_size();

			for (i = 0; i < client->codec_count; i++)
			Client_codec_add(client, msg->payload.authenticate->celt_versions(i));
			codec_itr = NULL;
			while (Client_codec_iterate(client, &codec_itr) != NULL)
				Log_debug("Client %d CELT codec ver 0x%x", client->sessionId, codec_itr->codec);

		} else {
			Client_codec_add(client, (int32_t)0x8000000b);
			client->codec_count = 1;
			fake_celt_support = true;
		}
		if (msg->payload.authenticate->opus())
			client->bOpus = true;

		recheckCodecVersions(client);

		sendmsg = Msg_create(CodecVersion);
		sendmsg->payload.codecVersion->set_alpha(iCodecAlpha);
		sendmsg->payload.codecVersion->set_beta(iCodecBeta);
		sendmsg->payload.codecVersion->set_prefer_alpha(bPreferAlpha);
		sendmsg->payload.codecVersion->set_opus(bOpus);
		Client_send_message(client, sendmsg);

		if (!bOpus && client->bOpus && fake_celt_support) {
			Client_textmessage(client, NO_CELT_MESSAGE);
		}

		/* Iterate channels and send channel info */
		ch_itr = NULL;
		while (Chan_iterate(&ch_itr) != NULL) {
			sendmsg = Msg_create(ChannelState);
			sendmsg->payload.channelState->set_channel_id(ch_itr->id);
			if (ch_itr->id != 0) {
				sendmsg->payload.channelState->set_parent(ch_itr->parent->id);
			}
			sendmsg->payload.channelState->set_name(ch_itr->name);
			if (ch_itr->desc)
				sendmsg->payload.channelState->set_description(ch_itr->desc);
			if (ch_itr->position != 0) {
				sendmsg->payload.channelState->set_position(ch_itr->position);
			}
			Log_debug("Send channel info: %s", sendmsg->payload.channelState->name().c_str());
			Client_send_message(client, sendmsg);
		}

		/* Iterate channels and send channel links info */
		ch_itr = NULL;
		while (Chan_iterate(&ch_itr) != NULL) {
			if (ch_itr->linkcount > 0) { /* Has links */
				struct dlist *itr;

				sendmsg = Msg_create(ChannelState);
				sendmsg->payload.channelState->set_channel_id(ch_itr->id);

				list_iterate(itr, &ch_itr->channel_links) { /* Iterate links */
					channellist_t *chl;
					channel_t *ch;
					chl = list_get_entry(itr, channellist_t, node);
					ch = chl->chan;
					sendmsg->payload.channelState->add_links(ch->id);
				}
				Client_send_message(client, sendmsg);
			}
		}

		/* Send user state for connecting user to other users */
		sendmsg = Msg_create(UserState);
		sendmsg->payload.userState->set_session(client->sessionId);
		sendmsg->payload.userState->set_name(client->username);
		sendmsg->payload.userState->set_channel_id(((channel_t *)client->channel)->id);

		if (defaultChan->silent) {
			sendmsg->payload.userState->set_suppress(true);
		}

		Client_send_message_except(client, sendmsg);

		client_itr = NULL;
		while (Client_iterate(&client_itr) != NULL) {
			if (!IS_AUTH(client_itr))
				continue;
			sendmsg = Msg_create(UserState);
			sendmsg->payload.userState->set_session(client_itr->sessionId);
			sendmsg->payload.userState->set_name(client_itr->username);
			sendmsg->payload.userState->set_channel_id(((channel_t *)client_itr->channel)->id);
			sendmsg->payload.userState->set_suppress(((channel_t *)client_itr->channel)->silent);

			client_itr->isSuppressed = ((channel_t *)client_itr->channel)->silent;

			if (client_itr->self_deaf) {
				sendmsg->payload.userState->set_self_deaf(true);
			}
			if (client_itr->self_mute) {
				sendmsg->payload.userState->set_self_mute(true);
			}
			if (client_itr->deaf) {
				sendmsg->payload.userState->set_deaf(true);
			}
			if (client_itr->mute) {
				sendmsg->payload.userState->set_mute(true);
			}
			if (client_itr->recording) {
				sendmsg->payload.userState->set_recording(true);
			}
			Client_send_message(client, sendmsg);
		}

		/* Sync message */
		sendmsg = Msg_create(ServerSync);
		sendmsg->payload.serverSync->set_session(client->sessionId);
		sendmsg->payload.serverSync->set_welcome_text(getStrConf(WELCOMETEXT));
		sendmsg->payload.serverSync->set_max_bandwidth(getIntConf(MAX_BANDWIDTH));
		Client_send_message(client, sendmsg);

		/* Server config message */
		sendmsg = Msg_create(ServerConfig);
		sendmsg->payload.serverConfig->set_allow_html(true); /* Support this? */
		sendmsg->payload.serverConfig->set_message_length(MAX_TEXT); /* Hardcoded */
		sendmsg->payload.serverConfig->set_image_message_length(0); /* XXX */
		Client_send_message(client, sendmsg);

		Log_info_client(client, "User %s authenticated", client->username);
		break;

	case Ping:
		if (msg->payload.ping->has_good())
			client->cryptState.uiRemoteGood = msg->payload.ping->good();
		if (msg->payload.ping->has_late())
			client->cryptState.uiRemoteLate = msg->payload.ping->late();
		if (msg->payload.ping->has_lost())
			client->cryptState.uiRemoteLost = msg->payload.ping->lost();
		if (msg->payload.ping->has_resync())
			client->cryptState.uiRemoteResync = msg->payload.ping->resync();

		Log_debug("Ping <-: %d %d %d %d",
				  client->cryptState.uiRemoteGood, client->cryptState.uiRemoteLate,
				  client->cryptState.uiRemoteLost, client->cryptState.uiRemoteResync
			);

		client->UDPPingAvg = msg->payload.ping->udp_ping_avg();
		client->UDPPingVar = msg->payload.ping->udp_ping_var();
		client->TCPPingAvg = msg->payload.ping->tcp_ping_avg();
		client->TCPPingVar = msg->payload.ping->tcp_ping_var();
		client->UDPPackets = msg->payload.ping->udp_packets();
		client->TCPPackets = msg->payload.ping->tcp_packets();

		sendmsg = Msg_create(Ping);

		sendmsg->payload.ping->set_timestamp(msg->payload.ping->timestamp());
		sendmsg->payload.ping->set_good(client->cryptState.uiGood);
		sendmsg->payload.ping->set_late(client->cryptState.uiLate);
		sendmsg->payload.ping->set_lost(client->cryptState.uiLost);
		sendmsg->payload.ping->set_resync(client->cryptState.uiResync);

		Client_send_message(client, sendmsg);
		Log_debug("Ping ->: %d %d %d %d",
				  client->cryptState.uiGood, client->cryptState.uiLate,
				  client->cryptState.uiLost, client->cryptState.uiResync);

		break;
	case CryptSetup:
		Log_debug("Voice channel crypt resync requested");
		if (!msg->payload.cryptSetup->has_client_nonce()) {
			sendmsg = Msg_create(CryptSetup);
			sendmsg->payload.cryptSetup->set_server_nonce(client->cryptState.decrypt_iv, AES_BLOCK_SIZE);
			Client_send_message(client, sendmsg);
		} else {
			memcpy(client->cryptState.decrypt_iv, msg->payload.cryptSetup->client_nonce().data(), AES_BLOCK_SIZE);
			client->cryptState.uiResync++;
		}
		break;
	case UserState:
		target = NULL;
		/* Only allow state changes for for the self user unless an admin is issuing */
		if (msg->payload.userState->has_session() &&
		    msg->payload.userState->session() != client->sessionId && !client->isAdmin) {
			sendPermissionDenied(client, "Permission denied");
			break;
		}
		if (msg->payload.userState->has_session() && msg->payload.userState->session() != client->sessionId) {
			while (Client_iterate(&target) != NULL) {
				if (target->sessionId == msg->payload.userState->session())
					break;
			}
			if (target == NULL) {
				Log_warn("Client with sessionID %d not found", msg->payload.userState->session());
				break;
			}
		}

		if (msg->payload.userState->has_user_id() || msg->payload.userState->has_suppress() ||
		    msg->payload.userState->has_priority_speaker() || msg->payload.userState->has_texture()) {
			sendPermissionDenied(client, "Not supported by uMurmur");
			break;
		}

		if (target == NULL)
			target = client;

		msg->payload.userState->set_session(target->sessionId);
		msg->payload.userState->set_actor(client->sessionId);

		if (msg->payload.userState->has_deaf()) {
			target->deaf = msg->payload.userState->deaf();
			if (target->deaf) {
				msg->payload.userState->set_mute(true);
			}
		}
		if (msg->payload.userState->has_mute()) {
			target->mute = msg->payload.userState->mute();
			if (!target->mute) {
				msg->payload.userState->set_deaf(false);
				target->deaf = false;
			}
		}
		if (msg->payload.userState->has_self_deaf()) {
			client->self_deaf = msg->payload.userState->self_deaf();
			if (client->self_deaf) {
				msg->payload.userState->set_self_mute(true);
			}
		}
		if (msg->payload.userState->has_self_mute()) {
			client->self_mute = msg->payload.userState->self_mute();
			if (!client->self_mute) {
				msg->payload.userState->set_self_deaf(false);
				client->self_deaf = false;
			}
		}
		if (msg->payload.userState->has_recording() &&
			msg->payload.userState->recording() != client->recording) {
			client->recording = msg->payload.userState->recording();
			char *message;
			uint32_t *tree_id;

			message = (char*)Memory_safeMalloc(1, strlen(client->username) + 32);
			sendmsg = Msg_create(TextMessage);
			sendmsg->payload.textMessage->add_tree_id(0);
			if (client->recording)
				sprintf(message, "User %s started recording", client->username);
			else
				sprintf(message, "User %s stopped recording", client->username);
			sendmsg->payload.textMessage->set_message(message);
			Client_send_message_except_ver(NULL, sendmsg, ~0x010203);
			free(message);
			sendmsg = NULL;
		}
		if (msg->payload.userState->has_channel_id()) {
			int leave_id;

			channelJoinResult_t result = Chan_userJoin_id_test(msg->payload.userState->channel_id(), target);

			if (result.CHJOIN_NOENTER || result.CHJOIN_NOTFOUND)
				break;

			if (result.CHJOIN_WRONGPW) {
				if (target == client && !client->isAdmin) {
					sendPermissionDenied(client, "Wrong channel password");
					break;
				}
				/* Tricky one: if user hasn't the password, but is moved to the channel by admin then let
				 * the user in. Also let admin user in regardless of channel password.
				 * Take no action on other errors.
				 */
				else if (!client->isAdmin)
					break;
			}

			leave_id = Chan_userJoin_id(msg->payload.userState->channel_id(), target);
			if (leave_id > 0) {
				Log_debug("Removing channel ID %d", leave_id);
				sendmsg = Msg_create(ChannelRemove);
				sendmsg->payload.channelRemove->set_channel_id(leave_id);
			}

			if (result.CHJOIN_SILENT) {
				if (!target->isSuppressed) {
				msg->payload.userState->set_suppress(true);
				target->isSuppressed = true;
				}
			}
			else if (target->isSuppressed) {
				msg->payload.userState->set_suppress(false);
				target->isSuppressed = false;
			}
		}
		if (msg->payload.userState->has_plugin_context()) {
			if (client->context)
				free(client->context);
			client->context = (char*)Memory_safeMalloc(1, msg->payload.userState->plugin_context().size());
			memcpy(client->context, msg->payload.userState->plugin_context().data(),
				   msg->payload.userState->plugin_context().size());

			break; /* Don't inform other users about this state */
		}
		/* Re-use message */
		Msg_inc_ref(msg);

		Client_send_message_except(NULL, msg);

		/* Need to send remove channel message _after_ UserState message */
		if (sendmsg != NULL)
			Client_send_message_except(NULL, sendmsg);
		break;

	case TextMessage:
		if (!getBoolConf(ALLOW_TEXTMESSAGE))
			break;
		msg->payload.textMessage->set_actor(client->sessionId);

		/* XXX - HTML is allowed and can't be turned off */
		if (msg->payload.textMessage->tree_id_size() > 0) {
			sendPermissionDenied(client, "Tree message not supported");
			break;
		}

		if (msg->payload.textMessage->channel_id_size() > 0) { /* To channel */
			int i;
			channel_t *ch_itr;
			for (i = 0; i < msg->payload.textMessage->channel_id_size(); i++) {
				ch_itr = NULL;
				do {
					Chan_iterate(&ch_itr);
				} while (ch_itr != NULL && ch_itr->id != msg->payload.textMessage->channel_id(i));
				if (ch_itr != NULL) {
					struct dlist *itr;
					list_iterate(itr, &ch_itr->clients) {
						client_t *c;
						c = list_get_entry(itr, client_t, chan_node);
						if (c != client && !c->deaf && !c->self_deaf) {
							Msg_inc_ref(msg);
							Client_send_message(c, msg);
							Log_debug("Text message to session ID %d", c->sessionId);
						}
					}
				}
			} /* for */
		}
		if (msg->payload.textMessage->session_size() > 0) { /* To user */
			int i;
			client_t *itr;
			for (i = 0; i < msg->payload.textMessage->session_size(); i++) {
				itr = NULL;
				while (Client_iterate(&itr) != NULL) {
					if (!IS_AUTH(itr))
						continue;
					if (itr->sessionId == msg->payload.textMessage->session(i)) {
						if (!itr->deaf && !itr->self_deaf) {
							Msg_inc_ref(msg);
							Client_send_message(itr, msg);
							Log_debug("Text message to session ID %d", itr->sessionId);
						}
						break;
					}
				}
				if (itr == NULL)
					Log_warn("TextMessage: Session ID %d not found", msg->payload.textMessage->session(i));
			} /* for */
		}
		break;

	case VoiceTarget:
	{
		int i, j, count, targetId = msg->payload.voiceTarget->id();
		const MumbleProto::VoiceTarget_Target *target;

		if (!targetId || targetId >= 0x1f)
			break;
		Voicetarget_add_id(client, targetId);
		count = msg->payload.voiceTarget->targets_size();
		if (!count)
			break;
		for (i = 0; i < count; i++) {
			target = &msg->payload.voiceTarget->targets(i);
			for (j = 0; j < target->session_size(); j++)
				Voicetarget_add_session(client, targetId, target->session(j));
			if (target->has_channel_id()) {
				bool_t linked = false, children = false;
				if (target->has_links())
					linked = target->links();
				if (target->has_children())
					children = target->children();
				Voicetarget_add_channel(client, targetId, target->channel_id(), linked, children);
			}
		}
		break;
	}
	case Version:
		Log_debug("Version message received");
		if (msg->payload.version->has_version()) {
			client->version = msg->payload.version->version();
			Log_debug("Client version 0x%x", client->version);
		}
		if (msg->payload.version->has_release()) {
			if (client->release) free(client->release);
			client->release = strdup(msg->payload.version->release().c_str());
			Log_debug("Client release %s", client->release);
		}
		if (msg->payload.version->has_os()) {
			if (client->os) free(client->os);
			client->os = strdup(msg->payload.version->os().c_str());
			Log_debug("Client OS %s", client->os);
		}
		if (msg->payload.version->has_os_version()) {
			if (client->os_version) free(client->os_version);
			client->os_version = strdup(msg->payload.version->os_version().c_str());
			Log_debug("Client OS version %s", client->os_version);
		}
		break;
	case PermissionQuery:
		Msg_inc_ref(msg); /* Re-use message */

		if (client->isAdmin)
			msg->payload.permissionQuery->set_permissions(PERM_ADMIN);
		else
			msg->payload.permissionQuery->set_permissions(PERM_DEFAULT);

		if (!getBoolConf(ALLOW_TEXTMESSAGE))
			msg->payload.permissionQuery->set_permissions(msg->payload.permissionQuery->permissions() & ~PERM_TEXTMESSAGE);
		if (!getBoolConf(ENABLE_BAN))
			msg->payload.permissionQuery->set_permissions(msg->payload.permissionQuery->permissions() & ~PERM_BAN);

		Client_send_message(client, msg);
		break;
	case UDPTunnel:
		client->bUDP = false;
		Client_voiceMsg(client, (uint8_t*)msg->payload.UDPTunnel->packet().data(), msg->payload.UDPTunnel->packet().size());
	    break;
	case ChannelState:
	{
		channel_t *ch_itr, *parent, *newchan;
		int leave_id;
		/* Don't allow any changes to existing channels */
		if (msg->payload.channelState->has_channel_id()) {
			sendPermissionDenied(client, "Not supported by uMurmur");
			break;
		}
		/* Must have parent */
		if (!msg->payload.channelState->has_parent()) {
			sendPermissionDenied(client, "Not supported by uMurmur");
			break;
		}
		/* Must have name */
		if (!msg->payload.channelState->has_name()) {
			sendPermissionDenied(client, "Not supported by uMurmur");
			break;
		}
		/* Must be temporary channel */
		if (msg->payload.channelState->temporary() != true) {
			sendPermissionDenied(client, "Only temporary channels are supported by uMurmur");
			break;
		}
		/* Check channel name is OK */
		if (strlen(msg->payload.channelState->name().c_str()) > MAX_TEXT) {
			sendPermissionDenied(client, "Channel name too long");
			break;
		}

		parent = Chan_fromId(msg->payload.channelState->parent());
		if (parent == NULL)
			break;
		ch_itr = NULL;
		while (Chan_iterate_siblings(parent, &ch_itr) != NULL) {
			if (strcmp(ch_itr->name, msg->payload.channelState->name().c_str()) == 0) {
				sendPermissionDenied(client, "Channel already exists");
				break;
			}
		}
		if (ch_itr != NULL)
			break;

		/* Disallow temporary channels as siblings to temporary channels */
		if (parent->temporary) {
			sendPermissionDenied(client, "Parent channel is temporary channel");
			break;
		}

		/* XXX - Murmur looks for "\\w" and sends perm denied if not found.
		 * I don't know why so I don't do that here...
		 */

		/* Create the channel */
		newchan = Chan_createChannel(msg->payload.channelState->name().c_str(),
									 msg->payload.channelState->description().c_str());
		newchan->temporary = true;
		if (msg->payload.channelState->has_position())
			newchan->position = msg->payload.channelState->position();
		Chan_addChannel(parent, newchan);
		msg->payload.channelState->set_channel_id(newchan->id);
		Msg_inc_ref(msg);
		Client_send_message_except(NULL, msg);

		/* Join the creating user */
		sendmsg = Msg_create(UserState);
		sendmsg->payload.userState->set_session(client->sessionId);
		sendmsg->payload.userState->set_channel_id(newchan->id);

		if (client->isSuppressed) {
			sendmsg->payload.userState->set_suppress(false);
			client->isSuppressed = false;
		}

		Client_send_message_except(NULL, sendmsg);

		leave_id = Chan_userJoin(newchan, client);
		if (leave_id > 0) {
			Log_debug("Removing channel ID %d", leave_id);
			sendmsg = Msg_create(ChannelRemove);
			sendmsg->payload.channelRemove->set_channel_id(leave_id);
			Client_send_message_except(NULL, sendmsg);
		}
	}
	break;

	case UserStats:
	{
		client_t *target = NULL;
		codec_t *codec_itr = NULL;
		int i;
		bool_t details = true;

		if (msg->payload.userStats->has_stats_only())
			details = !msg->payload.userStats->stats_only();

		if (!msg->payload.userStats->has_session())
			sendPermissionDenied(client, "Not supported by uMurmur");
		while (Client_iterate(&target) != NULL) {
			if (!IS_AUTH(target))
				continue;
			if (target->sessionId == msg->payload.userStats->session())
				break;
		}
		if (!target) /* Not found */
			break;

		/*
		 * Differences from Murmur:
		 * o Ignoring certificates intentionally
		 * o Ignoring channel local determining
		 */

		sendmsg = Msg_create(UserStats);
		sendmsg->payload.userStats->set_session(msg->payload.userStats->session());
		sendmsg->payload.userStats->mutable_from_client()->set_good(target->cryptState.uiGood);
		sendmsg->payload.userStats->mutable_from_client()->set_late(target->cryptState.uiLate);
		sendmsg->payload.userStats->mutable_from_client()->set_lost(target->cryptState.uiLost);
		sendmsg->payload.userStats->mutable_from_client()->set_resync(target->cryptState.uiResync);

		sendmsg->payload.userStats->mutable_from_server()->set_good(target->cryptState.uiRemoteGood);
		sendmsg->payload.userStats->mutable_from_server()->set_late(target->cryptState.uiRemoteLate);
		sendmsg->payload.userStats->mutable_from_server()->set_lost(target->cryptState.uiRemoteLost);
		sendmsg->payload.userStats->mutable_from_server()->set_resync(target->cryptState.uiRemoteResync);

		sendmsg->payload.userStats->set_udp_packets(target->UDPPackets);
		sendmsg->payload.userStats->set_udp_ping_avg(target->UDPPingAvg);
		sendmsg->payload.userStats->set_udp_ping_var(target->UDPPingVar);

		sendmsg->payload.userStats->set_tcp_ping_avg(target->TCPPingAvg);
		sendmsg->payload.userStats->set_tcp_ping_var(target->TCPPingVar);
		sendmsg->payload.userStats->set_tcp_packets(target->TCPPackets);

		if (details) {

			sendmsg->payload.userStats->mutable_version()->set_version(target->version);
			if (target->release)
				sendmsg->payload.userStats->mutable_version()->set_release(target->release);
			if (target->os)
				sendmsg->payload.userStats->mutable_version()->set_os(target->os);
			if (target->os_version)
				sendmsg->payload.userStats->mutable_version()->set_os_version(target->os_version);

			i = 0;
			while (Client_codec_iterate(target, &codec_itr) != NULL)
				sendmsg->payload.userStats->add_celt_versions(codec_itr->codec);

			sendmsg->payload.userStats->set_opus(target->bOpus);

			/* Address */
			/*if (getBoolConf(SHOW_ADDRESSES)) {
				sendmsg->payload.userStats->address.data
					= Memory_safeMalloc(16, sizeof(uint8_t));
				memset(sendmsg->payload.userStats->address.data, 0, 16);
				/* ipv4 representation as ipv6 address. Supposedly correct. * /
				memset(&sendmsg->payload.userStats->address.data[10], 0xff, 2); /* IPv4 * /
				if(target->remote_tcp.ss_family == AF_INET)
					memcpy(&sendmsg->payload.userStats->address.data[12], &((struct sockaddr_in*)&target->remote_tcp)->sin_addr, 4);
				else
					memcpy(&sendmsg->payload.userStats->address.data[0], &((struct sockaddr_in6*)&target->remote_tcp)->sin6_addr, 16);
				sendmsg->payload.userStats->address.len = 16;
			} else {
				sendmsg->payload.userStats->has_address = false;
			}*/
		}
		/* BW */
		sendmsg->payload.userStats->set_bandwidth(target->availableBandwidth);

		/* Onlinesecs */
		sendmsg->payload.userStats->set_onlinesecs(Timer_elapsed(&target->connectTime) / 1000000LL);
		/* Idlesecs */
		sendmsg->payload.userStats->set_idlesecs(Timer_elapsed(&target->idleTime) / 1000000LL);
		Client_send_message(client, sendmsg);
	}
	break;
	case UserRemove:
		target = NULL;
		/* Only admin can issue this */
		if (!client->isAdmin) {
			sendPermissionDenied(client, "Permission denied");
			break;
		}
		while (Client_iterate(&target) != NULL) {
			if (target->sessionId == msg->payload.userRemove->session())
				break;
		}
		if (target == NULL) {
			Log_warn("Client with sessionId %d not found", msg->payload.userRemove->session());
			break;
		}
		msg->payload.userRemove->set_session(target->sessionId);
		msg->payload.userRemove->set_actor(client->sessionId);

		if (msg->payload.userRemove->has_ban() && msg->payload.userRemove->ban()) {
			if (!getBoolConf(ENABLE_BAN))
				sendPermissionDenied(client, "Permission denied");
			else
				Ban_UserBan(target, (char*)msg->payload.userRemove->reason().c_str());
		} else {
			Log_info_client(target, "User kicked. Reason: '%s'",
			                strlen(msg->payload.userRemove->reason().c_str()) == 0 ? "N/A" : msg->payload.userRemove->reason().c_str());
		}
		/* Re-use message */
		Msg_inc_ref(msg);

		Client_send_message_except(NULL, msg);
		Client_close(target);
		break;
	case BanList:
		/* Only admin can issue this */
		if (!client->isAdmin) {
			sendPermissionDenied(client, "Permission denied");
			break;
		}
		if (!getBoolConf(ENABLE_BAN)) {
			sendPermissionDenied(client, "Permission denied");
			break;
		}
		if (msg->payload.banList->has_query() && msg->payload.banList->query()) {
			/* Create banlist message and add banentrys */
			sendmsg = Ban_getBanList();
			Client_send_message(client, sendmsg);
		} else {
			/* Clear banlist and set the new one */
			Ban_clearBanList();
			Ban_putBanList(msg, msg->payload.banList->bans_size());
		}
		break;

		/* Permission denied for all these messages. Not implemented. */
	case ChannelRemove:
	case ContextAction:
	case ContextActionAdd:
	case ACL_:
	case UserList:
	case QueryUsers:
		sendPermissionDenied(client, "Not supported by uMurmur");
		break;

	default:
		Log_warn("Message %d not handled", msg->messageType);
		break;
	}
out:
	Msg_free(msg);
	return;

disconnect:
	Msg_free(msg);
	Client_close(client);
}

