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

class TLSServerStream : public TcpServerStream, public Botan::TLS::Callbacks
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

	void Initialize();

public:
	// Botan::TLS::Callbacks
	virtual inline void tls_emit_data(const uint8_t data[], size_t size) override
	{
		return WriteToClient(data, size);
	}

	virtual inline void tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size) override
	{
		return ReceivedData(data, size);
	}

	virtual inline void tls_alert(Botan::TLS::Alert alert) override
	{
		return ReceivedAlert(alert, nullptr, 0);
	}

	virtual inline bool tls_session_established(const Botan::TLS::Session& session) override
	{
		return HandshakeComplete(session);
	}

	virtual inline std::string tls_server_choose_app_protocol(const std::vector<std::string>& client_protos) override
	{
		return "";
	}

	virtual inline void tls_verify_cert_chain(
		const std::vector<Botan::X509_Certificate>& cert_chain,
		const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp_responses,
		const std::vector<Botan::Certificate_Store*>& trusted_roots,
		Botan::Usage_Type usage,
		const std::string& hostname,
		const Botan::TLS::Policy& policy) override
	{
		try
		{
			Botan::TLS::Callbacks::tls_verify_cert_chain(cert_chain, ocsp_responses, trusted_roots, usage, hostname, policy);
		}
		catch (std::exception& e)
		{
			trace("%s\n", e.what());
		}
	}

private:
	void WriteToClient(const uint8_t buf[], size_t length);

	void ReceivedData(const uint8_t buf[], size_t length);

	void ReceivedAlert(Botan::TLS::Alert alert, const uint8_t[], size_t);

	bool HandshakeComplete(const Botan::TLS::Session& session);

private:
	void CloseInternal();
};

class TCP_SERVER_EXPORT TLSServer : public TcpServer
{
private:
	fwRefContainer<TcpServer> m_baseServer;

	std::shared_ptr<Botan::Credentials_Manager> m_credentials;

	std::set<fwRefContainer<TLSServerStream>> m_connections;

public:
	TLSServer(fwRefContainer<TcpServer> baseServer, const std::string& certificatePath, const std::string& keyPath);

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