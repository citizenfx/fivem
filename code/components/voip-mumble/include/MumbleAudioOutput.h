/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include <unordered_map>

#include <wrl.h>

#include <MumbleAudioSink.h>

#define XAUDIO2_HELPER_FUNCTIONS
#include <xaudio2.h>
#include <x3daudio.h>

#include <mmdeviceapi.h>
#include <opus.h>

#include <LabSound/extended/LabSound.h>

namespace WRL = Microsoft::WRL;

class MumbleClient;
class MumbleUser;

struct JitterBuffer_;
typedef struct JitterBuffer_ JitterBuffer;

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

	void HandleClientVoiceData(const MumbleUser& user, uint64_t sequence, const uint8_t* data, size_t size, bool hasTerminator);

	void HandleClientDisconnect(const MumbleUser& user);

	void SetAudioDevice(const std::string& deviceId);

	void SetVolume(float volume);

	void GetTalkers(std::vector<uint32_t>* talkers);

	void SetMatrix(float position[3], float front[3], float up[3]);

	void SetDistance(float distance);

	float GetDistance();

	std::shared_ptr<lab::AudioContext> GetAudioContext(const std::string& name);

	inline void SetClient(MumbleClient* client)
	{
		m_client = client;
	}

	friend struct XA2DestinationNode;

private:
	struct BaseAudioState;

	struct ClientAudioStateBase
	{
		ClientAudioStateBase();

		virtual ~ClientAudioStateBase();

		virtual bool PollAudio(int frameCount, BaseAudioState* root);

		void resizeBuffer(size_t size);

		uint64_t sequence = 0;
		bool isTalking;
		OpusDecoder* opus;
		JitterBuffer* jitter;

		std::mutex jitterLock;

		int iLastConsume = 0;
		int iBufferFilled = 0;
		int iBufferOffset = 0;
		float* pfBuffer = nullptr;
		int iBufferSize = 48000;
		bool bLastAlive = true;
		bool bHasTerminator = false;

		bool quiet = true;
		float fPowerMax = 0.0f;
		float fPowerMin = 0.0f;

		float fAverageAvailable = 0.0f;
		int iMissCount = 0;
		int iInterpCount = 0;

		std::list<std::unique_ptr<std::vector<uint8_t>>> qlFrames;
	};

	struct BaseAudioState : public std::enable_shared_from_this<BaseAudioState>
	{
	public:
		float volume;
		float position[3];
		float distance;
		float overrideVolume;
		bool isAudible = true;
		uint32_t lastTime;

		uint32_t lastPush = 0;

	private:
		std::shared_mutex innerStateMutex;

		std::shared_ptr<ClientAudioStateBase> innerState;

	public:
		BaseAudioState();

		virtual ~BaseAudioState();

		void HandleVoiceData(uint64_t sequence, const uint8_t* data, size_t size, bool hasTerminator);

		inline std::shared_ptr<ClientAudioStateBase> GetInnerState()
		{
			std::shared_lock _(innerStateMutex);
			return innerState;
		}

		inline void SetInnerState(std::shared_ptr<ClientAudioStateBase> val)
		{
			std::unique_lock _(innerStateMutex);
			innerState = val;
		}

		virtual bool Valid()
		{
			return true;
		}

		virtual void PushSound(int16_t* voiceBuffer, int len)
		{
		}

		virtual void PushPosition(MumbleAudioOutput* baseIo, float position[3])
		{
		}

		virtual bool IsTalking()
		{
			if (auto is = GetInnerState())
			{
				return is->isTalking;
			}

			return false;
		}

		virtual bool ShouldManagePoll()
		{
			return false;
		}

		virtual void AfterConstruct()
		{
		}
	};

	struct ExternalAudioState : public BaseAudioState
	{
		ExternalAudioState(fwRefContainer<IMumbleAudioSink> sink);

		virtual ~ExternalAudioState();

		virtual bool Valid() override;

		virtual void PushSound(int16_t* voiceBuffer, int len) override;

		virtual void PushPosition(MumbleAudioOutput* baseIo, float position[3]) override;

		virtual bool IsTalking() override;

		virtual void AfterConstruct() override;

		fwRefContainer<IMumbleAudioSink> sink;
	};

	struct ClientAudioState : public BaseAudioState, public IXAudio2VoiceCallback
	{
		/// <summary>
		/// @FIX(wyoming-papa-chicken): PushPosition happens on MainThrd,
		/// destroyVoice happens on the UV Loop, OnBufferEnd exists on XA2.
		/// </summary>
		std::mutex m_render;
		IXAudio2SourceVoice* voice;
		volatile bool shuttingDown;

		std::shared_ptr<lab::AudioContext> context;
		std::shared_ptr<lab::AudioSourceNode> inNode;

		ClientAudioState();

		virtual ~ClientAudioState();

		virtual bool Valid() override
		{
			return !context || context->destination();
		}

		virtual bool ShouldManagePoll()
		{
			return true;
		}

		virtual void PushSound(int16_t* voiceBuffer, int len) override;

		virtual void PushPosition(MumbleAudioOutput* baseIo, float position[3]) override;

		void PushSoundInternal(uint16_t* voiceBuffer, int len);

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

	std::unordered_map<uint32_t, std::shared_ptr<BaseAudioState>> m_clients;
	std::shared_mutex m_clientsMutex;

	std::thread m_thread;

	bool m_initialized;
	bool m_initializeSignaled = false;

	std::mutex m_initializeMutex;

	std::condition_variable m_initializeVar;

	MumbleClient* m_client;

	float m_volume;

	std::string m_deviceGuid;

	X3DAUDIO_HANDLE m_x3da;

	X3DAUDIO_LISTENER m_listener;

	DirectX::XMFLOAT3 m_lastPosition;

	uint32_t m_lastMatrixTime;

	int m_channelCount;

	float m_distance;

	decltype(&X3DAudioCalculate) m_x3daCalculate;
};
