/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

// include protobuf messages
#include "auth.pb.h"
#include "friends.pb.h"
#include "hello.pb.h"
#include "servers.pb.h"
#include "storage.pb.h"

namespace terminal
{
// base traits class
template<int AMessageType, typename AProtoClass>
class MessageTraits
{
public:
	typedef AProtoClass ProtoClass;

	typedef std::shared_ptr<ProtoClass> ProtoPtr;

	static const int MessageType = AMessageType;
	
public:
	static ProtoPtr Create()
	{
		return std::make_shared<AProtoClass>();
	}
};
}

namespace terminal
{
namespace msg
{
#include "MessageDefinition.autogen.h"
}
}