/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/Client.h>
#include <terminal/internal/L1ServiceDispatcher.h>
#include <terminal/internal/L1LegacyMessageParser.h>
#include <terminal/internal/L1FramedTcpConnection.h>
#include <terminal/internal/TcpStreamSocket.h>

namespace terminal
{
concurrency::task<Result<ConnectRemoteDetail>> Client::ConnectRemote(const char* hostname, uint16_t port)
{
	// TODO: register this before connecting
	m_dispatcher = new L1ServiceDispatcher();

	m_parser = new L1LegacyMessageParser();
	m_parser->SetDispatcher(m_dispatcher);

	m_connection = new L1FramedTcpConnection();
	m_connection->SetParser(m_parser);

	concurrency::task_completion_event<Result<ConnectRemoteDetail>> completionSource;

	m_socket = new TcpStreamSocket();
	m_socket->Connect(hostname, port).then([=] (Result<void> result)
	{
		if (result.HasSucceeded())
		{
			m_connection->BindSocket(m_socket);

			ConnectRemoteDetail detail(1);

			completionSource.set(Result<ConnectRemoteDetail>(result, detail));
		}
		else
		{
			completionSource.set(result);
		}
	});

	return concurrency::task<Result<ConnectRemoteDetail>>(completionSource);
}

void* Client::GetUserService(uint64_t interfaceIdentifier)
{
	return nullptr;
}

REGISTER_INTERFACE(Client);
}