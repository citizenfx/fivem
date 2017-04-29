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
#include <terminal/internal/L1MessageBuilder.h>
#include <terminal/internal/TcpStreamSocket.h>

#include <terminal/internal/User1.h>
#include <terminal/internal/Utils1.h>

#include <network/uri.hpp>

namespace terminal
{
Client::Client()
{
	m_dispatcher = new L1ServiceDispatcher();

	m_parser = new L1LegacyMessageParser();
	m_parser->SetDispatcher(m_dispatcher);

	m_connection = new L1FramedTcpConnection();
	m_connection->SetParser(m_parser);

	m_builder = new L1MessageBuilder();
	m_builder->SetConnection(m_connection);
	m_builder->SetDispatcher(m_dispatcher);
}

concurrency::task<Result<ConnectRemoteDetail>> Client::ConnectRemote(const char* uri)
{
	// parse the host URI
	std::error_code ec;
	network::uri serviceUri = network::make_uri(std::string(uri), ec);

	// if the uri couldn't be parsed, return an error
	if (!static_cast<bool>(ec))
	{
		boost::string_ref scheme = serviceUri.scheme().get_value_or("layer1");

		if (scheme == "layer1")
		{
			boost::string_ref hostname = serviceUri.host().get_value_or("localhost");
			int port = serviceUri.port<int>().get_value_or(3036);

			concurrency::task_completion_event<Result<ConnectRemoteDetail>> completionSource;

			m_socket = new TcpStreamSocket();
			m_socket->Connect(hostname.to_string().c_str(), port).then([=] (Result<void> result)
			{
				if (result.HasSucceeded())
				{
					m_connection->BindSocket(m_socket);

					ConnectRemoteDetail detail(1);

					completionSource.set(Result<ConnectRemoteDetail>(detail));
				}
				else
				{
					completionSource.set(result);
				}
			});

			return concurrency::task<Result<ConnectRemoteDetail>>(completionSource);
		}
		else
		{
			return concurrency::task_from_result(Result<ConnectRemoteDetail>(Result<void>(ErrorCode::InvalidScheme)));
		}
	}
	else
	{
		return concurrency::task_from_result(Result<ConnectRemoteDetail>(Result<void>(ErrorCode::InvalidUri)));
	}
}

Result<void*> Client::GetUserService(uint64_t interfaceIdentifier)
{
	if (m_connection->HasSocket())
	{
		Result<void*> result(ErrorCode::UnknownInterface, nullptr);

		TryCreateService<User1, IUser1>(interfaceIdentifier, &result);

		return result;
	}

	return Result<void>((m_connection->HasSocket()) ? ErrorCode::UnknownInterface : ErrorCode::NotConnected);
}

Result<void*> Client::GetUtilsService(uint64_t interfaceIdentifier)
{
	if (m_connection->HasSocket())
	{
		Result<void*> result(ErrorCode::UnknownInterface, nullptr);

		TryCreateService<Utils1, IUtils1>(interfaceIdentifier, &result);

		return result;
	}

	return Result<void>((m_connection->HasSocket()) ? ErrorCode::UnknownInterface : ErrorCode::NotConnected);
}

REGISTER_INTERFACE(Client);
}