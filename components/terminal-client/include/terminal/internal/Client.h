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
#include <terminal/internal/L1MessageBuilder.h>

namespace terminal
{
class Client : public IClient
{
private:
	fwRefContainer<Connection> m_connection;

	fwRefContainer<Dispatcher> m_dispatcher;

	fwRefContainer<Parser> m_parser;

	fwRefContainer<StreamSocket> m_socket;

	fwRefContainer<L1MessageBuilder> m_builder; // yes, this isn't an abstract type - so?

	concurrency::concurrent_unordered_map<uint64_t, fwRefContainer<fwRefCountable>> m_interfaces;

private:
	template<typename TImpl, typename TInterface>
	void TryCreateService(uint64_t interfaceIdentifier, Result<void*>* result)
	{
		if (interfaceIdentifier == TInterface::InterfaceID)
		{
			auto it = m_interfaces.find(interfaceIdentifier);

			if (it == m_interfaces.end())
			{
				fwRefContainer<TImpl> user = new TImpl(this);
				m_interfaces[interfaceIdentifier] = user;

				*result = Result<void*>(static_cast<TInterface*>(user.GetRef()));
			}
			else
			{
				*result = Result<void*>(static_cast<TInterface*>(static_cast<TImpl*>(it->second.GetRef())));
			}
		}
	}

public:
	Client();

	virtual concurrency::task<Result<ConnectRemoteDetail>> ConnectRemote(const char* uri) override;

	virtual Result<void*> GetUserService(uint64_t interfaceIdentifier) override;

	virtual Result<void*> GetUtilsService(uint64_t interfaceIdentifier) override;

public:
	inline fwRefContainer<L1MessageBuilder> GetBuilder()
	{
		return m_builder;
	}
};
}