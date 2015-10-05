/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "TcpServer.h"
#include "TcpServerFactory.h"

#ifdef min
#undef min
#endif

#include <botan/auto_rng.h>

#include <botan/tls_server.h>
#include <botan/tls_session_manager.h>

#ifdef COMPILING_NET_TCP_SERVER
#define TCP_SERVER_EXPORT DLL_EXPORT
#else
#define TCP_SERVER_EXPORT DLL_IMPORT
#endif

namespace net
{
class TLSServer;

class TLSServerStream : public TcpServerStream
{
private:
	fwRefContainer<TcpServerStream> m_baseStream;

	std::shared_ptr<Botan::TLS::Server> m_tlsServer;

	TLSServer* m_parentServer;

	Botan::AutoSeeded_RNG m_rng;

	std::unique_ptr<Botan::TLS::Session_Manager> m_sessionManager;

	std::unique_ptr<Botan::TLS::Policy> m_policy;

	bool m_closing;

public:
	TLSServerStream(TLSServer* server, fwRefContainer<TcpServerStream> baseStream);

	virtual PeerAddress GetPeerAddress() override;

	virtual void Write(const std::vector<uint8_t>& data) override;

	virtual void Close() override;

private:
	void WriteToClient(const uint8_t buf[], size_t length);

	void ReceivedData(const uint8_t buf[], size_t length);

	void ReceivedAlert(Botan::TLS::Alert alert, const uint8_t[], size_t);

	bool HandshakeComplete(const Botan::TLS::Session& session);

private:
	void Initialize();

	void CloseInternal();
};

class TCP_SERVER_EXPORT TLSServer : public TcpServer
{
private:
	fwRefContainer<TcpServer> m_baseServer;

	std::shared_ptr<Botan::Credentials_Manager> m_credentials;

	std::set<fwRefContainer<TLSServerStream>> m_connections;

public:
	TLSServer(fwRefContainer<TcpServer> baseServer);

	inline std::shared_ptr<Botan::Credentials_Manager> GetCredentials()
	{
		return m_credentials;
	}

	inline void InvokeConnectionCallback(TLSServerStream* stream)
	{
		if (GetConnectionCallback())
		{
			GetConnectionCallback()(stream);
		}
	}

	inline void CloseStream(TLSServerStream* stream)
	{
		m_connections.erase(stream);
	}
};
}