#pragma once

#undef min
#undef max

#include <botan/auto_rng.h>
#include <botan/tls_client.h>

#include <MumbleClient.h>
#include <MumbleDataHandler.h>
#include <WS2tcpip.h>

#include <MumbleClientState.h>

#include <thread>

enum class ClientTask
{
	Idle,
	BeginConnect,
	EndConnect,
	RecvData,

};

class MumbleCredentialsManager : public Botan::Credentials_Manager
{
public:
	virtual void verify_certificate_chain(const std::string& type, const std::string& hostname, const std::vector<Botan::X509_Certificate>& cert_chain)
	{
		// no-op
	}
};

class MumbleTLSPolicy : public Botan::TLS::Policy
{
public:
	virtual bool acceptable_protocol_version(Botan::TLS::Protocol_Version version) const override
	{
		if (version != Botan::TLS::Protocol_Version::SSL_V3)
		{
			return true;
		}

		return false;
	}
};

class MumbleClient : public IMumbleClient
{
public:
	virtual void Initialize();

	virtual concurrency::task<MumbleConnectionInfo*> ConnectAsync(const char* hostname, uint16_t port, const wchar_t* userName);

	virtual concurrency::task<void> DisconnectAsync();

private:
	SOCKET m_socket;

	std::thread m_mumbleThread;

	HANDLE m_beginConnectEvent;

	HANDLE m_socketConnectEvent;
	HANDLE m_socketReadEvent;

	HANDLE m_idleEvent;

	MumbleConnectionInfo m_connectionInfo;

	MumbleDataHandler m_handler;

	concurrency::task_completion_event<MumbleConnectionInfo*> m_completionEvent;

	Botan::AutoSeeded_RNG m_rng;

	MumbleTLSPolicy m_policy;

	std::unique_ptr<MumbleCredentialsManager> m_credentials;

	std::shared_ptr<Botan::TLS::Client> m_tlsClient;

	std::unique_ptr<Botan::TLS::Session_Manager_In_Memory> m_sessionManager;

	MumbleClientState m_state;

public:
	static fwRefContainer<MumbleClient> GetCurrent();

	inline MumbleClientState& GetState() { return m_state; }

	template<typename TPacket>
	void Send(MumbleMessageType type, TPacket& packet)
	{
		auto string = packet.SerializeAsString();

		Send(type, string.c_str(), string.size());
	}

	void Send(MumbleMessageType type, const char* buf, size_t size);

private:
	static void ThreadFunc(MumbleClient* client);

	bool CreateSocket();

	void WriteToSocket(const uint8_t buf[], size_t length);

	void OnAlert(Botan::TLS::Alert alert, const uint8_t[], size_t);

	void OnReceive(const uint8_t buf[], size_t length);

	bool OnHandshake(const Botan::TLS::Session& session);

	ClientTask WaitForTask();

	void ThreadFuncImpl();

	void Send(const char* buf, size_t size);
};