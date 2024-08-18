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
#include <botan/system_rng.h>

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

#ifdef IS_FXSERVER
	Botan::AutoSeeded_RNG m_rng;
#else
	Botan::System_RNG m_rng;
#endif

	std::unique_ptr<Botan::TLS::Session_Manager> m_sessionManager;

	std::unique_ptr<Botan::TLS::Policy> m_policy;

	std::string m_protocol;

	bool m_closing;

private:
	template<typename TContainer>
	inline void DoWrite(TContainer data, TCompleteCallback&& onComplete)
	{
		fwRefContainer<TLSServerStream> thisRef = this;

		ScheduleCallback([thisRef, data = std::forward<TContainer>(data), onComplete = std::move(onComplete)]() mutable
		{
			auto tlsServer = thisRef->m_tlsServer;

			if (tlsServer && tlsServer->is_active())
			{
				try
				{
					thisRef->m_nextOnComplete = std::move(onComplete);
					tlsServer->send(data);
				}
				catch (const std::exception& e)
				{
					trace("tls send: %s\n", e.what());

					thisRef->Close();
				}
			}
		}, true);
	}

public:
	TLSServerStream(TLSServer* server, fwRefContainer<TcpServerStream> baseStream);

	virtual PeerAddress GetPeerAddress() override;

	virtual void Write(std::string&& data, TCompleteCallback&& onComplete) override;

	virtual void Write(std::vector<uint8_t>&& data, TCompleteCallback&& onComplete) override;

	virtual void Write(const std::string& data, TCompleteCallback&& onComplete) override;

	virtual void Write(const std::vector<uint8_t>& data, TCompleteCallback&& onComplete) override;

	virtual void Write(std::unique_ptr<char[]> data, size_t len, TCompleteCallback&& onComplete) override
	{
		fwRefContainer<TLSServerStream> thisRef = this;

		ScheduleCallback([thisRef, data = std::move(data), len, onComplete = std::move(onComplete)]() mutable
		{
			auto tlsServer = thisRef->m_tlsServer;

			if (tlsServer && tlsServer->is_active())
			{
				try
				{
					thisRef->m_nextOnComplete = std::move(onComplete);
					tlsServer->send(reinterpret_cast<const uint8_t*>(data.get()), len);
				}
				catch (const std::exception & e)
				{
					trace("tls send: %s\n", e.what());

					thisRef->Close();
				}
			}
		}, true);
	}

	virtual void Close() override;

	void Initialize();

public:
	// Botan::TLS::Callbacks
	virtual void tls_emit_data(const uint8_t data[], size_t size) override
	{
		return WriteToClient(data, size);
	}

	virtual void tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size) override
	{
		return ReceivedData(data, size);
	}

	virtual void tls_alert(Botan::TLS::Alert alert) override
	{
		return ReceivedAlert(alert, nullptr, 0);
	}

	virtual void tls_session_activated() override
	{
		HandshakeComplete();
	}

	virtual bool tls_session_established(const Botan::TLS::Session& session) override
	{
		//return HandshakeComplete(session);
		return true;
	}

	virtual std::string tls_server_choose_app_protocol(const std::vector<std::string>& client_protos) override;

	virtual void tls_verify_cert_chain(
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

	virtual void ScheduleCallback(TScheduledCallback&& callback, bool performInline) override;

	void StartConnectionTimeout(std::chrono::duration<uint64_t, std::milli> timeout) override;

private:
	void WriteToClient(const uint8_t buf[], size_t length);

	void ReceivedData(const uint8_t buf[], size_t length);

	void ReceivedAlert(Botan::TLS::Alert alert, const uint8_t[], size_t);

	bool HandshakeComplete();

private:
	void CloseInternal();

	TCompleteCallback m_nextOnComplete;
};

class TCP_SERVER_EXPORT TLSServer : public TcpServer
{
private:
	fwRefContainer<TcpServer> m_baseServer;

	std::shared_ptr<Botan::Credentials_Manager> m_credentials;

	std::set<fwRefContainer<TLSServerStream>> m_connections;

	std::mutex m_connectionsMutex;

	std::vector<std::string> m_protocols;

	std::map<std::string, fwRefContainer<TcpServer>> m_protocolServers;

public:
	TLSServer(fwRefContainer<TcpServer> baseServer, const std::string& certificatePath, const std::string& keyPath, bool autoGenerate = false);

private:
	void Initialize(fwRefContainer<TcpServer> baseServer, std::shared_ptr<Botan::Credentials_Manager> credentialManager);

public:
	fwRefContainer<TcpServer> GetProtocolServer(const std::string& protocolName);

	inline std::shared_ptr<Botan::Credentials_Manager> GetCredentials()
	{
		return m_credentials;
	}

	inline std::vector<std::string> GetProtocolList()
	{
		return m_protocols;
	}

	inline void SetProtocolList(const std::vector<std::string>& protocols)
	{
		m_protocols = protocols;
	}

	void InvokeConnectionCallback(TLSServerStream* stream, const std::string& protocol);

	inline void CloseStream(TLSServerStream* stream)
	{
		std::lock_guard<std::mutex> _(m_connectionsMutex);
		m_connections.erase(stream);
	}
};
}
