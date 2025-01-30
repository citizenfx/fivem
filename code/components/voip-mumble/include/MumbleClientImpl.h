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

#include <concurrent_queue.h>

#include <thread>

#include <uvw.hpp>
#include <botan/block_cipher.h>

namespace net
{
	class UvLoopHolder;
}

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


constexpr uint8_t AES_BLOCK_SIZE = 16;

class MumbleCrypto : public fwRefCountable
{
public:
	uint32_t m_remoteGood;
	uint32_t m_remoteLate;
	uint32_t m_remoteLost;
	uint32_t m_remoteResync;

	uint32_t m_localGood;
	uint32_t m_localLate;
	uint32_t m_localLost;
	uint32_t m_localResync;

	void Encrypt(const uint8_t* plain, uint8_t* cipher, size_t length);
	bool Decrypt(const uint8_t* cipher, uint8_t* plain, size_t length);
	std::string GetClientNonce();

	bool SetServerNonce(const std::string& serverNonce);
	bool SetKey(const std::string& key, const std::string& clientNonce, const std::string& serverNonce);
	bool IsInitialized() const;


	MumbleCrypto()
	{
		memset(m_key.data(), 0, AES_BLOCK_SIZE);
		memset(m_clientNonce.data(), 0, AES_BLOCK_SIZE);
		memset(m_serverNonce.data(), 0, AES_BLOCK_SIZE);
	}
private:
	std::array<uint8_t, 16> m_key;
	std::array<uint8_t, 16> m_clientNonce;
	std::array<uint8_t, 16> m_serverNonce;

	bool m_init;

	uint8_t m_decryptHistory[0x100];

	std::unique_ptr<Botan::BlockCipher> m_cipher;

private:
	void OCBEncrypt(const unsigned char *plain, unsigned char *encrypted, unsigned int len, const unsigned char *nonce, unsigned char *tag);
	void OCBDecrypt(const unsigned char *encrypted, unsigned char *plain, unsigned int len, const unsigned char *nonce, unsigned char *tag);
};

class MumbleClient : public IMumbleClient, public Botan::TLS::Callbacks
{
public:
	virtual void Initialize() override;

	virtual concurrency::task<MumbleConnectionInfo*> ConnectAsync(const net::PeerAddress& address, const std::string& userName) override;

	virtual concurrency::task<void> DisconnectAsync() override;

	virtual void RunFrame() override;

	virtual MumbleConnectionInfo* GetConnectionInfo() override;

	virtual void UpdateVoiceTarget(int idx, const VoiceTargetConfig& config) override;

	virtual void SetVoiceTarget(int idx) override;

	virtual bool IsAnyoneTalking() override;

	virtual float GetInputAudioLevel() override;

	virtual void SetChannel(const std::string& channelName) override;

	virtual void AddListenChannel(const std::string& channelName) override;

	virtual void RemoveListenChannel(const std::string& channelName) override;

	virtual bool DoesChannelExist(const std::string& channelName) override;

	virtual std::shared_ptr<lab::AudioContext> GetAudioContext(const std::string& name) override;

	virtual void SetClientVolumeOverride(const std::wstring& clientName, float volume) override;

	virtual void SetClientVolumeOverrideByServerId(uint32_t serverId, float volume) override;

	virtual std::wstring GetPlayerNameFromServerId(uint32_t serverId) override;

	virtual std::string GetVoiceChannelFromServerId(uint32_t serverId) override;

	virtual void GetTalkers(std::vector<std::string>* referenceIds) override;

	virtual void SetPositionHook(const TPositionHook& hook) override;

	virtual void SetAudioDistance(float distance) override;

	virtual void SetAudioInputDistance(float distance) override;

	virtual void SetAudioOutputDistance(float distance) override;

	virtual float GetAudioDistance() override;

	virtual void SetActorPosition(float position[3]) override;

	virtual void SetListenerMatrix(float position[3], float front[3], float up[3]) override;

	virtual void SetActivationMode(MumbleActivationMode mode) override;

	virtual void SetPTTButtonState(bool pressed) override;

	virtual void SetOutputVolume(float volume) override;

	virtual void SetActivationLikelihood(MumbleVoiceLikelihood likelihood) override;

	virtual void SetInputDevice(const std::string& dsoundDeviceId) override;

	virtual void SetOutputDevice(const std::string& dsoundDeviceId) override;

private:
	fwRefContainer<net::UvLoopHolder> m_loop;

	std::shared_ptr<uvw::TimerHandle> m_connectTimer;

	std::shared_ptr<uvw::TimerHandle> m_idleTimer;

	std::shared_ptr<uvw::TCPHandle> m_tcp;

	std::shared_ptr<uvw::UDPHandle> m_udp;

	MumbleConnectionInfo m_connectionInfo;

	MumbleDataHandler m_handler;

	MumbleAudioInput m_audioInput;

	MumbleAudioOutput m_audioOutput;

	concurrency::task_completion_event<MumbleConnectionInfo*> m_completionEvent;

	concurrency::concurrent_queue<std::tuple<uint32_t, std::array<float, 3>>> m_positionUpdates;

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

	uint16_t m_inFlightTcpPings = 0;

	bool m_hasUdp = false;

	bool m_udpTimedOut = false;

	// the time in milliseconds since the player joined the mumble server
	// This is used in for `Ping` packets to determine if we should allow the client to swap from UDP -> TCP
	// By default we wont swap back to TCP for the first 20 seconds of the clients connection (only after we have UDP)
	std::chrono::milliseconds m_timeSinceJoin;

	std::chrono::milliseconds m_nextPing;

	TPositionHook m_positionHook;

	std::string m_curManualChannel;

	std::string m_lastManualChannel;

	std::set<std::string> m_curChannelListens;

	std::set<std::string> m_lastChannelListens;

	std::map<int, VoiceTargetConfig> m_pendingVoiceTargetUpdates;

public:
	MumbleCrypto m_crypto;

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

public:
	void MarkConnected();

private:

	void WriteToSocket(const uint8_t buf[], size_t length);

	void OnAlert(Botan::TLS::Alert alert, const uint8_t[], size_t);

	void OnReceive(const uint8_t buf[], size_t length);

	bool OnHandshake(const Botan::TLS::Session& session);

	void OnActivated();

	void Send(const char* buf, size_t size);
};
