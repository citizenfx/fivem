/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <thread>
#include <wrl.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
extern "C"
{
#include <libswresample/swresample.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
};

#include <webrtc/modules/audio_processing/include/audio_processing.h>
#include <webrtc/modules/interface/module_common_types.h>
#include <webrtc/system_wrappers/include/trace.h>

#include <opus.h>

#include <queue>

#include "MumbleClient.h"

using namespace Microsoft::WRL;

class MumbleClient;

class MumbleAudioInput
{
private:
	std::thread m_thread;

	ComPtr<IMMDeviceEnumerator> m_mmDeviceEnumerator;

	ComPtr<IAudioClient> m_audioClient;

	ComPtr<IAudioCaptureClient> m_audioCaptureClient;

	WAVEFORMATEX m_waveFormat;

	HANDLE m_startEvent;

	SwrContext* m_avr;

	webrtc::AudioProcessing* m_apm;

	uint8_t* m_resampledBytes;

	uint32_t m_cachedLen;

	uint8_t m_cachedBytes[512 * 1024];

	uint8_t m_encodedBytes[65536];

	OpusEncoder* m_opus;

	std::queue<std::string> m_opusPackets;

	uint64_t m_sequence;

	MumbleClient* m_client;

	MumbleActivationMode m_mode;

	MumbleVoiceLikelihood m_likelihood;

	bool m_isTalking;

	bool m_ptt;

	std::string m_deviceId;

	float m_positionX;
	float m_positionY;
	float m_positionZ;

	float m_voiceDistance;

	float m_audioLevel;

	std::vector<uint8_t> m_audioBuffer;

private:
	static void ThreadStart(MumbleAudioInput* instance);

	void ThreadFunc();

	void InitializeAudioDevice();

	HRESULT HandleIncomingAudio();

	void HandleData(const uint8_t* buffer, size_t numBytes);

	void EnqueueOpusPacket(std::string packet);

	void SendQueuedOpusPackets();

	inline void SetInputFormat(WAVEFORMATEX* waveFormat) { m_waveFormat = *waveFormat; }

public:
	MumbleAudioInput();

	void Initialize();

	void Enable();

	void SetActivationMode(MumbleActivationMode mode);

	void SetActivationLikelihood(MumbleVoiceLikelihood likelihood);

	void SetPTTButtonState(bool pressed);

	void SetAudioDevice(const std::string& dsoundDeviceId);

	inline bool IsTalking()
	{
		return m_isTalking;
	}

	inline float GetAudioLevel()
	{
		return m_audioLevel;
	}

	inline void SetPosition(float position[3])
	{
		m_positionX = position[0];
		m_positionY = position[1];
		m_positionZ = position[2];
	}

	inline void SetDistance(float distance)
	{
		m_voiceDistance = distance;
	}

	inline void SetClient(MumbleClient* client) { m_client = client; }
};
