/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <NetAddress.h>
#include <ppltasks.h>

struct MumbleConnectionInfo
{
	bool isConnected;
	net::PeerAddress address;
	std::string username;
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

class IMumbleClient : public fwRefCountable
{
public:
	virtual void Initialize() = 0;

	virtual concurrency::task<MumbleConnectionInfo*> ConnectAsync(const net::PeerAddress& address, const std::string& userName) = 0;

	virtual concurrency::task<void> DisconnectAsync() = 0;

	virtual MumbleConnectionInfo* GetConnectionInfo() = 0;

	virtual bool IsAnyoneTalking() = 0;

	virtual float GetInputAudioLevel() = 0;

	virtual void SetChannel(const std::string& channelName) = 0;

	virtual void GetTalkers(std::vector<std::string>* names) = 0;

	virtual void SetAudioDistance(float distance) = 0;

	virtual void SetActorPosition(float position[3]) = 0;

	virtual void SetListenerMatrix(float position[3], float front[3], float up[3]) = 0;

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
