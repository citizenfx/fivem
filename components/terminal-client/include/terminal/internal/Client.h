/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Connection.h>
#include <terminal/internal/Dispatcher.h>
#include <terminal/internal/Parser.h>
#include <terminal/internal/Socket.h>

namespace terminal
{
class Client : public IClient
{
private:
	fwRefContainer<Connection> m_connection;

	fwRefContainer<Dispatcher> m_dispatcher;

	fwRefContainer<Parser> m_parser;

	fwRefContainer<StreamSocket> m_socket;

public:
	virtual concurrency::task<Result<ConnectRemoteDetail>> ConnectRemote(const char* hostname, uint16_t port) override;

	virtual void* GetUserService(uint64_t interfaceIdentifier) override;
};
}