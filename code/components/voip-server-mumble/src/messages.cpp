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
#include <stdio.h>
#include <string.h>

#include "messages.h"
#include "client.h"
#include "pds.h"
#include "log.h"
#include "memory.h"

#define PREAMBLE_SIZE 6

static message_t *Msg_create_nopayload(messageType_t messageType);

static void Msg_addPreamble(uint8_t *buffer, uint16_t type, uint32_t len)
{
	buffer[1] = (type) & 0xff;
	buffer[0] = (type >> 8) & 0xff;

	buffer[5] = (len) & 0xff;
	buffer[4] = (len >> 8) & 0xff;
	buffer[3] = (len >> 16) & 0xff;
	buffer[2] = (len >> 24) & 0xff;
}

static void Msg_getPreamble(uint8_t *buffer, int *type, int *len)
{
	uint16_t msgType;
	uint32_t msgLen;

	msgType = buffer[1] | (buffer[0] << 8);
	msgLen = buffer[5] | (buffer[4] << 8) | (buffer[3] << 16) | (buffer[2] << 24);
	*type = (int)msgType;
	*len = (int)msgLen;
}

#define MAX_MSGSIZE (BUFSIZE - PREAMBLE_SIZE)
int Msg_messageToNetwork(message_t *msg, uint8_t *buffer)
{
	int len;
	uint8_t *bufptr = buffer + PREAMBLE_SIZE;

	Log_debug("To net: msg type %d", msg->messageType);
	switch (msg->messageType) {
	case Version:
		len = msg->payload.version->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
		}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.version->SerializeToArray(bufptr, len);
		break;
	case UDPTunnel: /* Non-standard handling of tunneled voice traffic. */
		if (msg->payload.UDPTunnel->packet().size() > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
		}
		len = msg->payload.UDPTunnel->packet().size();
		Msg_addPreamble(buffer, msg->messageType, msg->payload.UDPTunnel->packet().size());
		memcpy(bufptr, msg->payload.UDPTunnel->packet().data(), msg->payload.UDPTunnel->packet().size());
		break;
	case Authenticate:
		len = msg->payload.authenticate->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.authenticate->SerializeToArray(bufptr, len);
		break;
	case Ping:
		len = msg->payload.ping->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.ping->SerializeToArray(bufptr, len);
		break;
	case Reject:
		len = msg->payload.reject->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.reject->SerializeToArray(bufptr, len);
		break;
	case ServerSync:
		len = msg->payload.serverSync->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.serverSync->SerializeToArray(bufptr, len);
		break;
	case TextMessage:
		len = msg->payload.textMessage->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.textMessage->SerializeToArray(bufptr, len);
		break;
	case PermissionDenied:
		len = msg->payload.permissionDenied->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.permissionDenied->SerializeToArray(bufptr, len);
		break;
	case CryptSetup:
		len = msg->payload.cryptSetup->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.cryptSetup->SerializeToArray(bufptr, len);
		break;
	case UserList:
		len = msg->payload.userList->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.userList->SerializeToArray(bufptr, len);
		break;
	case UserState:
		len = msg->payload.userState->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.userState->SerializeToArray(bufptr, len);
		break;
	case UserRemove:
		len = msg->payload.userRemove->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.userRemove->SerializeToArray(bufptr, len);
		break;
	case ChannelState:
		len = msg->payload.channelState->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.channelState->SerializeToArray(bufptr, len);
		break;
	case VoiceTarget:
		len = msg->payload.voiceTarget->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.voiceTarget->SerializeToArray(bufptr, len);
		break;
	case CodecVersion:
		len = msg->payload.codecVersion->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.codecVersion->SerializeToArray(bufptr, len);
		break;
	case PermissionQuery:
		len = msg->payload.permissionQuery->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.permissionQuery->SerializeToArray(bufptr, len);
		break;
	case ChannelRemove:
		len = msg->payload.channelRemove->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.channelRemove->SerializeToArray(bufptr, len);
		break;
	case UserStats:
	{
		len = msg->payload.userStats->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.userStats->SerializeToArray(bufptr, len);
		break;
	}
	case ServerConfig:
		len = msg->payload.serverConfig->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		msg->payload.serverConfig->SerializeToArray(bufptr, len);
		break;

	case BanList:
		len = msg->payload.banList->ByteSize();
		if (len > MAX_MSGSIZE) {
			Log_warn("Too big tx message. Discarding");
			break;
			}
		Msg_addPreamble(buffer, msg->messageType, len);
		Log_debug("Msg_MessageToNetwork: BanList size %d", len);
		msg->payload.banList->SerializeToArray(bufptr, len);
		break;
	default:
		Log_warn("Msg_MessageToNetwork: Unsupported message %d", msg->messageType);
		return 0;
	}
	return len + PREAMBLE_SIZE;
}

static message_t *Msg_create_nopayload(messageType_t messageType)
{
	message_t *msg = (message_t*)Memory_safeMalloc(1, sizeof(message_t));

	memset(msg, 0, sizeof(message_t));
	msg->refcount = 1;
	msg->messageType = messageType;
	init_list_entry(&msg->node);
	return msg;
}

message_t *Msg_create(messageType_t messageType)
{
	message_t *msg = Msg_create_nopayload(messageType);
	int i;

	switch (messageType) {
	case Version:
		msg->payload.version = new MumbleProto::Version();

		break;
	case UDPTunnel:
		msg->payload.UDPTunnel = new MumbleProto::UDPTunnel();
		
		break;
	case Authenticate:
		msg->payload.authenticate = new MumbleProto::Authenticate();
		
		break;
	case Ping:
		msg->payload.ping = new MumbleProto::Ping();
		
		break;
	case Reject:
		msg->payload.reject = new MumbleProto::Reject();
		
		break;
	case ServerSync:
		msg->payload.serverSync = new MumbleProto::ServerSync();
		
		break;
	case TextMessage:
		msg->payload.textMessage = new MumbleProto::TextMessage();
		
		break;
	case PermissionDenied:
		msg->payload.permissionDenied = new MumbleProto::PermissionDenied();
		
		break;
	case CryptSetup:
		msg->payload.cryptSetup = new MumbleProto::CryptSetup();
		
		break;
	case UserList:
		msg->payload.userList = new MumbleProto::UserList();
		
		break;
	case UserState:
		msg->payload.userState = new MumbleProto::UserState();
		
		break;
	case ChannelState:
		msg->payload.channelState = new MumbleProto::ChannelState();
		
		break;
	case UserRemove:
		msg->payload.userRemove = new MumbleProto::UserRemove();
		
		break;
	case VoiceTarget:
		msg->payload.voiceTarget = new MumbleProto::VoiceTarget();
		
		break;
	case CodecVersion:
		msg->payload.codecVersion = new MumbleProto::CodecVersion();
		
		break;
	case PermissionQuery:
		msg->payload.permissionQuery = new MumbleProto::PermissionQuery();
		
		break;
	case ChannelRemove:
		msg->payload.channelRemove = new MumbleProto::ChannelRemove();
		
		break;
	case UserStats:
		msg->payload.userStats = new MumbleProto::UserStats();

		break;
	case ServerConfig:
		msg->payload.serverConfig = new MumbleProto::ServerConfig();
		
		break;

	default:
		Log_warn("Msg_create: Unsupported message %d", msg->messageType);
		break;
	}

	return msg;
}

message_t *Msg_banList_create(int n_bans)
{
	message_t *msg = Msg_create_nopayload(BanList);
	int i;

	msg->payload.banList = new MumbleProto::BanList();
	return msg;
}

void Msg_banList_addEntry(message_t *msg, int index, uint8_t *address, uint32_t mask,
                          char *name, char *hash, char *reason, char *start, uint32_t duration)
{
	MumbleProto::BanList::BanEntry *entry = msg->payload.banList->add_bans();

	entry->set_address(address, 16);
	entry->set_mask(mask);
	entry->set_name(name);
	entry->set_hash(hash);
	entry->set_reason(reason);
	entry->set_start(start);

	if (duration > 0) {
		entry->set_duration(duration);
	}
	Log_debug("Msg_banList_addEntry: %s %s %s %s %s",
		entry->name().c_str(), entry->hash().c_str(), entry->address().c_str(), entry->reason().c_str(), entry->start().c_str());
}

void Msg_banList_getEntry(message_t *msg, int index, uint8_t **address, uint32_t *mask,
                          char **name, char **hash, char **reason, char **start, uint32_t *duration)
{
	const MumbleProto::BanList_BanEntry *entry = &msg->payload.banList->bans(index);

	*address = (uint8_t*)entry->address().c_str();
	*mask = entry->mask();
	*name = (char*)entry->name().c_str();
	*hash = (char*)entry->hash().c_str();
	*reason = (char*)entry->reason().c_str();
	*start = (char*)entry->start().c_str();
	if (entry->has_duration())
		*duration = entry->duration();
	else
		*duration = 0;
}


void Msg_inc_ref(message_t *msg)
{
	msg->refcount++;
}

void Msg_free(message_t *msg)
{
	int i;

	if (msg->refcount) msg->refcount--;
	if (msg->refcount > 0)
		return;

	switch (msg->messageType) {
	case Version:
		delete msg->payload.version;
		break;
	case UDPTunnel:
		delete msg->payload.UDPTunnel;
		break;
	case Authenticate:
		delete msg->payload.authenticate;
		break;
	case Ping:
		delete msg->payload.ping;
		break;
	case Reject:
		delete msg->payload.reject;
		break;
	case ServerSync:
		delete msg->payload.serverSync;
		break;
	case TextMessage:
		delete msg->payload.textMessage;
		break;
	case PermissionDenied:
		delete msg->payload.permissionDenied;
		break;
	case CryptSetup:
		delete msg->payload.cryptSetup;
		break;
	case UserList:
		delete msg->payload.userList;
		break;
	case UserState:
		delete msg->payload.userState;
		break;
	case ChannelState:
		delete msg->payload.channelState;
		break;
	case UserRemove:
		delete msg->payload.userRemove;
		break;
	case VoiceTarget:
		delete msg->payload.voiceTarget;
		break;
	case CodecVersion:
		delete msg->payload.codecVersion;
		break;
	case PermissionQuery:
		delete msg->payload.permissionQuery;
		break;
	case ChannelRemove:
		delete msg->payload.channelRemove;
		break;
	case UserStats:
		delete msg->payload.userStats;
		break;
	case ServerConfig:
		delete msg->payload.serverConfig;
		break;
	case BanList:
		delete msg->payload.banList;
		break;

	default:
		Log_warn("Msg_free: Unsupported message %d", msg->messageType);
		break;
	}
	free(msg);
}

message_t *Msg_CreateVoiceMsg(uint8_t *data, int size)
{
	message_t *msg = NULL;

	msg = Msg_create_nopayload(UDPTunnel);
	msg->unpacked = false;
	msg->payload.UDPTunnel = new MumbleProto::UDPTunnel();
	msg->payload.UDPTunnel->set_packet(data, size);
	return msg;
}

message_t *Msg_networkToMessage(uint8_t *data, int size)
{
	message_t *msg = NULL;
	uint8_t *msgData = &data[6];
	int messageType, msgLen;

	Msg_getPreamble(data, &messageType, &msgLen);

	Log_debug("Message type %d size %d", messageType, msgLen);
	//dumpmsg(data, size);

	switch (messageType) {
	case Version:
	{
		msg = Msg_create_nopayload(Version);
		msg->unpacked = true;
		msg->payload.version = new MumbleProto::Version();
		if (!msg->payload.version->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case UDPTunnel: /* Non-standard handling of tunneled voice data */
	{
		msg = Msg_CreateVoiceMsg(msgData, msgLen);
		break;
	}
	case Authenticate:
	{
		msg = Msg_create_nopayload(Authenticate);
		msg->unpacked = true;
		msg->payload.authenticate = new MumbleProto::Authenticate();
		if (!msg->payload.authenticate->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case Ping:
	{
		msg = Msg_create_nopayload(Ping);
		msg->unpacked = true;
		msg->payload.ping = new MumbleProto::Ping();
		if (!msg->payload.ping->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case Reject:
	{
		msg = Msg_create_nopayload(Reject);
		msg->unpacked = true;
		msg->payload.reject = new MumbleProto::Reject();
		if (!msg->payload.reject->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case ServerSync:
	{
		msg = Msg_create_nopayload(ServerSync);
		msg->unpacked = true;
		msg->payload.serverSync = new MumbleProto::ServerSync();
		if (!msg->payload.serverSync->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case TextMessage:
	{
		msg = Msg_create_nopayload(TextMessage);
		msg->unpacked = true;
		msg->payload.textMessage = new MumbleProto::TextMessage();
		if (!msg->payload.textMessage->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case PermissionDenied:
	{
		msg = Msg_create_nopayload(PermissionDenied);
		msg->unpacked = true;
		msg->payload.permissionDenied = new MumbleProto::PermissionDenied();
		if (!msg->payload.permissionDenied->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case CryptSetup:
	{
		msg = Msg_create_nopayload(CryptSetup);
		msg->unpacked = true;
		msg->payload.cryptSetup = new MumbleProto::CryptSetup();
		if (!msg->payload.cryptSetup->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case UserList:
	{
		msg = Msg_create_nopayload(UserList);
		msg->unpacked = true;
		msg->payload.userList = new MumbleProto::UserList();
		if (!msg->payload.userList->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case UserState:
	{
		msg = Msg_create_nopayload(UserState);
		msg->unpacked = true;
		msg->payload.userState = new MumbleProto::UserState();
		if (!msg->payload.userState->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case ChannelState:
	{
		msg = Msg_create_nopayload(ChannelState);
		msg->unpacked = true;
		msg->payload.channelState = new MumbleProto::ChannelState();
		if (!msg->payload.channelState->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case VoiceTarget:
	{
		msg = Msg_create_nopayload(VoiceTarget);
		msg->unpacked = true;
		msg->payload.voiceTarget = new MumbleProto::VoiceTarget();
		if (!msg->payload.voiceTarget->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case CodecVersion:
	{
		msg = Msg_create_nopayload(CodecVersion);
		msg->unpacked = true;
		msg->payload.codecVersion = new MumbleProto::CodecVersion();
		if (!msg->payload.codecVersion->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case PermissionQuery:
	{
		msg = Msg_create_nopayload(PermissionQuery);
		msg->unpacked = true;
		msg->payload.permissionQuery = new MumbleProto::PermissionQuery();
		if (!msg->payload.permissionQuery->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case ChannelRemove:
	{
		msg = Msg_create_nopayload(ChannelRemove);
		msg->unpacked = true;
		msg->payload.channelRemove = new MumbleProto::ChannelRemove();
		if (!msg->payload.channelRemove->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case UserStats:
	{
		msg = Msg_create_nopayload(UserStats);
		msg->unpacked = true;
		msg->payload.userStats = new MumbleProto::UserStats();
		if (!msg->payload.userStats->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case UserRemove:
	{
		msg = Msg_create_nopayload(UserRemove);
		msg->unpacked = true;
		msg->payload.userRemove = new MumbleProto::UserRemove();
		if (!msg->payload.userRemove->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}
	case BanList:
	{
		msg = Msg_create_nopayload(BanList);
		msg->unpacked = true;
		msg->payload.banList = new MumbleProto::BanList();
		if (!msg->payload.banList->ParseFromArray(msgData, msgLen))
			goto err_out;
		break;
	}

	default:
		Log_warn("Msg_networkToMessage: Unsupported message %d", messageType);
		break;
	}
	return msg;

err_out:
	Msg_free(msg);
	return NULL;
}
