/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <array>
#include <optional>

#include <NetAddress.h>
#include <ppltasks.h>

namespace lab
{
	class AudioContext;
}

struct MumbleConnectionInfo
{
	bool isConnected;
	bool isConnecting;
	net::PeerAddress address;
	std::string username;

	inline MumbleConnectionInfo()
		: isConnected(false), isConnecting(false)
	{

	}
};

enum class MumbleActivationMode
{
	Disabled,
	VoiceActivity,
	PushToTalk
};

enum class MumbleVoiceLikelihood
{
	VeryLowLikelihood,
	LowLikelihood,
	ModerateLikelihood,
	HighLikelihood
};

struct VoiceTargetConfig
{
	struct Target
	{
		std::vector<std::wstring> users;
		std::string channel;
		// ACL is not supported in umurmur, so does not count
		bool links;
		bool children;

		inline Target()
			: links(false), children(false)
		{

		}
	};

	std::vector<Target> targets;
};

class IMumbleClient : public fwRefCountable
{
public:
	using TPositionHook = std::function<std::optional<std::array<float, 3>>(const std::string&)>;

public:
	virtual void Initialize() = 0;

	virtual concurrency::task<MumbleConnectionInfo*> ConnectAsync(const net::PeerAddress& address, const std::string& userName) = 0;

	virtual concurrency::task<void> DisconnectAsync() = 0;

	virtual void RunFrame() = 0;

	virtual MumbleConnectionInfo* GetConnectionInfo() = 0;

	virtual bool IsAnyoneTalking() = 0;

	virtual float GetInputAudioLevel() = 0;

	virtual void SetChannel(const std::string& channelName) = 0;

	virtual void SetClientVolumeOverride(const std::wstring& clientName, float volume) = 0;

	virtual void SetClientVolumeOverrideByServerId(uint32_t serverId, float volume) = 0;

	virtual std::wstring GetPlayerNameFromServerId(uint32_t serverId) = 0;

	virtual uint32_t GetVoiceChannelFromServerId(uint32_t serverId) = 0;

	virtual void GetTalkers(std::vector<std::string>* names) = 0;

	virtual void SetPositionHook(const TPositionHook& hook) = 0;

	virtual void SetAudioDistance(float distance) = 0;

	virtual float GetAudioDistance() = 0;

	virtual void SetActorPosition(float position[3]) = 0;

	virtual void SetListenerMatrix(float position[3], float front[3], float up[3]) = 0;

	virtual void UpdateVoiceTarget(int idx, const VoiceTargetConfig& config) = 0;

	virtual void SetVoiceTarget(int idx) = 0;

	virtual void AddListenChannel(const std::string& channelName) = 0;

	virtual void RemoveListenChannel(const std::string& channelName) = 0;

	virtual std::shared_ptr<lab::AudioContext> GetAudioContext(const std::string& name) = 0;

	// settings
	virtual void SetActivationMode(MumbleActivationMode mode) = 0;

	virtual void SetPTTButtonState(bool pressed) = 0;

	virtual void SetOutputVolume(float volume) = 0;

	virtual void SetActivationLikelihood(MumbleVoiceLikelihood likelihood) = 0;

	virtual void SetInputDevice(const std::string& dsoundDeviceId) = 0;

	virtual void SetOutputDevice(const std::string& dsoundDeviceId) = 0;
};

fwRefContainer<IMumbleClient>
	#ifdef COMPILING_VOIP_MUMBLE
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	CreateMumbleClient();
