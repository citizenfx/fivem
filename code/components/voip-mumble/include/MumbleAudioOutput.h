/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <concurrent_unordered_map.h>

#include <map>
#include <mutex>
#include <thread>

#include <wrl.h>

#define XAUDIO2_HELPER_FUNCTIONS
#include <xaudio2.h>
#include <x3daudio.h>

#include <mmdeviceapi.h>
#include <opus.h>

namespace WRL = Microsoft::WRL;

class MumbleClient;
class MumbleUser;

class MumbleAudioOutput
{
public:
	void Initialize();

	void ThreadFunc();

	void InitializeAudioDevice();

	void HandleClientConnect(const MumbleUser& user);

	void HandleClientDistance(const MumbleUser& user, float distance);

	void HandleClientPosition(const MumbleUser& user, float position[3]);

	void HandleClientVolumeOverride(const MumbleUser& user, float volume);

	void HandleClientVoiceData(const MumbleUser& user, uint64_t sequence, const uint8_t* data, size_t size);

	void HandleClientDisconnect(const MumbleUser& user);

	void SetAudioDevice(const std::string& deviceId);

	void SetVolume(float volume);

	void GetTalkers(std::vector<uint32_t>* talkers);

	void SetMatrix(float position[3], float front[3], float up[3]);

	void SetDistance(float distance);

	inline void SetClient(MumbleClient* client)
	{
		m_client = client;
	}

private:
	struct ClientAudioState : public IXAudio2VoiceCallback
	{
		IXAudio2SourceVoice* voice;
		uint64_t sequence;
		float volume;
		float position[3];
		float distance;
		float overrideVolume;
		bool isTalking;
		bool isAudible;
		OpusDecoder* opus;
		uint32_t lastTime;

		ClientAudioState();

		virtual ~ClientAudioState();

		void __stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
		void __stdcall OnVoiceProcessingPassEnd() override {}
		void __stdcall OnStreamEnd() override {}
		void __stdcall OnBufferStart(void*) override {}
		void __stdcall OnLoopEnd(void*) override {}
		void __stdcall OnVoiceError(void*, HRESULT) override {}

		void __stdcall OnBufferEnd(void* cxt) override;
	};

private:
	WRL::ComPtr<IXAudio2> m_xa2;
	IXAudio2MasteringVoice* m_masteringVoice;
	IXAudio2SubmixVoice* m_submixVoice;

	WRL::ComPtr<IMMDeviceEnumerator> m_mmDeviceEnumerator;

	concurrency::concurrent_unordered_map<uint32_t, std::shared_ptr<ClientAudioState>> m_clients;

	std::thread m_thread;

	bool m_initialized;

	std::mutex m_initializeMutex;

	std::condition_variable m_initializeVar;

	MumbleClient* m_client;

	float m_volume;

	std::string m_deviceGuid;

	X3DAUDIO_HANDLE m_x3da;

	X3DAUDIO_LISTENER m_listener;

	DirectX::XMFLOAT3 m_lastPosition;

	uint32_t m_lastMatrixTime;

	float m_distance;

	decltype(&X3DAudioCalculate) m_x3daCalculate;
};
