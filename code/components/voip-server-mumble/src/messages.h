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
#ifndef MESSAGES_H_89768
#define MESSAGES_H_89768

#include <stdint.h>
#include "Mumble.pb.h"
#include "list.h"
#include "types.h"

#define PROTVER_MAJOR 1
#define PROTVER_MINOR 4
#define PROTVER_PATCH 0
#define PROTOCOL_VERSION ((PROTVER_MAJOR << 16) | (PROTVER_MINOR << 8) | (PROTVER_PATCH))

#define PERM_NONE 0x0
#define PERM_WRITE 0x1
#define PERM_TRAVERSE 0x2
#define PERM_ENTER 0x4
#define PERM_SPEAK 0x8
#define PERM_MUTEDEAFEN 0x10
#define PERM_MOVE 0x20
#define PERM_MAKECHANNEL 0x40
#define PERM_LINKCHANNEL 0x80
#define PERM_WHISPER 0x100
#define PERM_TEXTMESSAGE 0x200
#define PERM_MAKETEMPCHANNEL 0x400
#define PERM_LISTEN 0x800
// Root channel only
#define PERM_KICK 0x10000
#define PERM_BAN 0x20000
#define PERM_REGISTER 0x40000
#define PERM_SELFREGISTER 0x80000	
#define PERM_CACHED 0x8000000
#define PERM_ALL 0xf0fff

#define PERM_DEFAULT (PERM_TRAVERSE | PERM_ENTER | PERM_SPEAK | PERM_WHISPER | PERM_TEXTMESSAGE | PERM_MAKETEMPCHANNEL | PERM_LISTEN)
#define PERM_ADMIN (PERM_DEFAULT | PERM_MUTEDEAFEN | PERM_MOVE | PERM_KICK | PERM_BAN)

typedef enum {
	Version,
	UDPTunnel,
	Authenticate,
	Ping,
	Reject,
	ServerSync,
	ChannelRemove,
	ChannelState,
	UserRemove,
	UserState,
	BanList, /* 10 */
	TextMessage,
	PermissionDenied,
	ACL_,
	QueryUsers,
	CryptSetup,
	ContextActionAdd,
	ContextAction,
	UserList,
	VoiceTarget,
	PermissionQuery, /* 20 */
	CodecVersion,
	UserStats,
	RequestBlob,
	ServerConfig
} messageType_t;

typedef enum {
	UDPVoiceCELTAlpha,
	UDPPing,
	UDPVoiceSpeex,
	UDPVoiceCELTBeta,
	UDPVoiceOpus,
} UDPMessageType_t;


typedef union payload {
	MumbleProto::Version *version;
	MumbleProto::UDPTunnel *UDPTunnel;
	MumbleProto::Authenticate *authenticate;
	MumbleProto::Ping *ping;
	MumbleProto::Reject *reject;
	MumbleProto::ServerSync *serverSync;
	MumbleProto::ChannelRemove *channelRemove;
	MumbleProto::ChannelState *channelState;
	MumbleProto::UserRemove *userRemove;
	MumbleProto::UserState *userState;
	MumbleProto::BanList *banList;	
	MumbleProto::TextMessage *textMessage;
	MumbleProto::PermissionDenied *permissionDenied;
	/* ChanACL not supported */
	/* ACL not supported */
	MumbleProto::QueryUsers *queryUsers;
	MumbleProto::CryptSetup *cryptSetup;
	/* ContextActionAdd not supported */
	/* ContextAction not supported */
	MumbleProto::UserList::User *userList_user;
	MumbleProto::UserList *userList;
	MumbleProto::VoiceTarget::Target *voiceTarget_target;
	MumbleProto::VoiceTarget *voiceTarget;
	MumbleProto::PermissionQuery *permissionQuery;
	MumbleProto::CodecVersion *codecVersion;
	MumbleProto::UserStats *userStats;
	MumbleProto::ServerConfig *serverConfig;
} payload_t;

typedef struct message {
	messageType_t messageType;
	int refcount;
	struct dlist node;
	bool_t unpacked;
	payload_t payload;
} message_t;


int Msg_messageToNetwork(message_t *msg, uint8_t *buffer);
message_t *Msg_networkToMessage(uint8_t *data, int size);
void Msg_free(message_t *msg);
void Msg_inc_ref(message_t *msg);

message_t *Msg_CreateVoiceMsg(uint8_t *data, int size);
message_t *Msg_create(messageType_t messageType);
void Msg_banList_addEntry(message_t *msg, int index, uint8_t *address, uint32_t mask,
                          char *name, char *hash, char *reason, char *start, uint32_t duration);
void Msg_banList_getEntry(message_t *msg, int index, uint8_t **address, uint32_t *mask,
                          char **name, char **hash, char **reason, char **start, uint32_t *duration);
message_t *Msg_banList_create(int n_bans);


#endif
