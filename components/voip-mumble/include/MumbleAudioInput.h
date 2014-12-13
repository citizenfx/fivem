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
#include <libavresample/avresample.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
};
#include <opus.h>

#include <queue>

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

	AVAudioResampleContext* m_avr;

	uint8_t* m_resampledBytes;

	uint32_t m_cachedLen;

	uint8_t m_cachedBytes[512 * 1024];

	uint8_t m_encodedBytes[65536];

	OpusEncoder* m_opus;

	std::queue<std::string> m_opusPackets;

	uint64_t m_sequence;

	MumbleClient* m_client;

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
	void Initialize();

	void Enable();

	inline void SetClient(MumbleClient* client) { m_client = client; }
};