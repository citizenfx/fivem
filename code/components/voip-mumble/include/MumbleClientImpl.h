/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#undef min
#undef max

#include <botan/auto_rng.h>
#include <botan/tls_client.h>

#include <MumbleClient.h>
#include <MumbleDataHandler.h>
#include <WS2tcpip.h>

#include <MumbleClientState.h>
#include <MumbleAudioInput.h>
#include <MumbleAudioOutput.h>

#include <thread>

enum class ClientTask
{
	Idle,
	BeginConnect,
	EndConnect,
	RecvData,
	UDPRead,
	Unknown
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
		return true;
	}

	virtual bool acceptable_ciphersuite(const Botan::TLS::Ciphersuite& suite) const override
	{
		return Botan::TLS::Policy::acceptable_ciphersuite(suite);
	}

	virtual std::vector<std::string> allowed_key_exchange_methods() const override
	{
		return {
			//"SRP_SHA",
			//"ECDHE_PSK",
			//"DHE_PSK",
			//"PSK",
			"CECPQ1",
			"ECDH",
			"DH",
			"RSA",
		};
	}
};

class MumbleCrypto : public fwRefCountable
{
public:
	virtual void Encrypt(const uint8_t* plain, uint8_t* cipher, size_t length) = 0;

	virtual bool Decrypt(const uint8_t* cipher, uint8_t* plain, size_t length) = 0;
};

class MumbleClient : public IMumbleClient, public Botan::TLS::Callbacks
{
public:
	virtual void Initialize() override;

	virtual concurrency::task<MumbleConnectionInfo*> ConnectAsync(const net::PeerAddress& address, const std::string& userName) override;

	virtual concurrency::task<void> DisconnectAsync() override;

	virtual MumbleConnectionInfo* GetConnectionInfo() override;

	virtual void UpdateVoiceTarget(int idx, const VoiceTargetConfig& config) override;

	virtual void SetVoiceTarget(int idx) override;

	virtual bool IsAnyoneTalking() override;

	virtual float GetInputAudioLevel() override;

	virtual void SetChannel(const std::string& channelName) override;

	virtual void SetClientVolumeOverride(const std::string& clientName, float volume) override;

	virtual void GetTalkers(std::vector<std::string>* referenceIds) override;

	virtual void SetPositionHook(const TPositionHook& hook) override;

	virtual void SetAudioDistance(float distance) override;

	virtual void SetActorPosition(float position[3]) override;

	virtual void SetListenerMatrix(float position[3], float front[3], float up[3]) override;

	virtual void SetActivationMode(MumbleActivationMode mode) override;

	virtual void SetPTTButtonState(bool pressed) override;

	virtual void SetOutputVolume(float volume) override;

	virtual void SetActivationLikelihood(MumbleVoiceLikelihood likelihood) override;

	virtual void SetInputDevice(const std::string& dsoundDeviceId) override;

	virtual void SetOutputDevice(const std::string& dsoundDeviceId) override;

private:
	SOCKET m_socket;

	std::thread m_mumbleThread;

	HANDLE m_beginConnectEvent;

	HANDLE m_socketConnectEvent;
	HANDLE m_socketReadEvent;
	HANDLE m_udpReadEvent;

	HANDLE m_idleEvent;

	MumbleConnectionInfo m_connectionInfo;

	MumbleDataHandler m_handler;

	MumbleAudioInput m_audioInput;

	MumbleAudioOutput m_audioOutput;

	concurrency::task_completion_event<MumbleConnectionInfo*> m_completionEvent;

	Botan::AutoSeeded_RNG m_rng;

	MumbleTLSPolicy m_policy;

	std::unique_ptr<MumbleCredentialsManager> m_credentials;

	std::shared_ptr<Botan::TLS::Client> m_tlsClient;

	std::unique_ptr<Botan::TLS::Session_Manager_In_Memory> m_sessionManager;

	MumbleClientState m_state;

	std::recursive_mutex m_clientMutex;

	float m_tcpPingAverage;
	float m_tcpPingVariance;

	uint32_t m_tcpPingCount;
	uint32_t m_tcpPings[12];

	float m_udpPingAverage;
	float m_udpPingVariance;

	uint32_t m_udpPingCount;
	uint32_t m_udpPings[12];

	int m_voiceTarget;

	std::chrono::milliseconds m_lastUdp;

	SOCKET m_udpSocket;

	TPositionHook m_positionHook;

	fwRefContainer<MumbleCrypto> m_crypto;

	std::string m_curManualChannel;

public:
	static fwRefContainer<MumbleClient> GetCurrent();

	inline int GetVoiceTarget() { return m_voiceTarget; }

	inline MumbleClientState& GetState() { return m_state; }

	inline void EnableAudioInput() { m_audioInput.Enable(); }

	inline MumbleAudioOutput& GetOutput() { return m_audioOutput; }

	void HandlePing(const MumbleProto::Ping& ping);

	template<typename TPacket>
	void Send(MumbleMessageType type, TPacket& packet)
	{
		auto string = packet.SerializeAsString();

		Send(type, string.c_str(), string.size());
	}

	void Send(MumbleMessageType type, const char* buf, size_t size);

	void SendVoice(const char* buf, size_t size);

	void SendUDP(const char* buf, size_t size);

	void HandleVoice(const uint8_t* data, size_t size);

	void HandleUDP(const uint8_t* buf, size_t size);

	inline void SetCrypto(const fwRefContainer<MumbleCrypto>& crypto)
	{
		m_crypto = crypto;
	}

	// Botan::TLS::Callbacks
public:
	virtual inline void tls_emit_data(const uint8_t data[], size_t size) override
	{
		return WriteToSocket(data, size);
	}

	virtual inline void tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size) override
	{
		return OnReceive(data, size);
	}

	virtual inline void tls_alert(Botan::TLS::Alert alert) override
	{
		return OnAlert(alert, nullptr, 0);
	}

	virtual inline bool tls_session_established(const Botan::TLS::Session& session) override
	{
		return OnHandshake(session);
	}

	virtual inline void tls_session_activated() override
	{
		OnActivated();
	}

	virtual inline void tls_verify_cert_chain(
		const std::vector<Botan::X509_Certificate>& cert_chain,
		const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp_responses,
		const std::vector<Botan::Certificate_Store*>& trusted_roots,
		Botan::Usage_Type usage,
		const std::string& hostname,
		const Botan::TLS::Policy& policy) override
	{

	}

private:
	static void ThreadFunc(MumbleClient* client);

public:
	void MarkConnected();

private:

	void WriteToSocket(const uint8_t buf[], size_t length);

	void OnAlert(Botan::TLS::Alert alert, const uint8_t[], size_t);

	void OnReceive(const uint8_t buf[], size_t length);

	bool OnHandshake(const Botan::TLS::Session& session);

	void OnActivated();

	ClientTask WaitForTask();

	void ThreadFuncImpl();

	void Send(const char* buf, size_t size);
};
