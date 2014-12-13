/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleAudioInput.h"
#include <avrt.h>
#include <sstream>
#include <PacketDataStream.h>
#include <MumbleClientImpl.h>
#include <ksmedia.h>

#pragma comment(lib, "avrt.lib")

extern "C"
{
#include <libavresample/avresample.h>
};

void MumbleAudioInput::Initialize()
{
	m_startEvent = CreateEvent(0, 0, 0, 0);

	m_thread = std::thread(ThreadStart, this);

	m_resampledBytes = nullptr;

	m_sequence = 0;
	m_cachedLen = 0;
}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

void MumbleAudioInput::ThreadFunc()
{
	SetThreadName(-1, "[Mumble] Audio Input Thread");

	// initialize COM for the current thread
	CoInitialize(nullptr);

	HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, (void**)m_mmDeviceEnumerator.GetAddressOf());

	if (FAILED(hr))
	{
		return;
	}

	InitializeAudioDevice();

	// wait for the activation signal
	WaitForSingleObject(m_startEvent, INFINITE);

	// go!
	m_audioClient->Start();

	while (true)
	{
		WaitForSingleObject(m_startEvent, INFINITE);

		HRESULT hr = HandleIncomingAudio();

		if (FAILED(hr))
		{
			if (hr == AUDCLNT_E_DEVICE_INVALIDATED)
			{
				InitializeAudioDevice();

				m_audioClient->Start();

				continue;
			}

			continue;
		}
	}
}

void MumbleAudioInput::HandleData(const uint8_t* buffer, size_t numBytes)
{
	//FILE* f = fopen("Q:\\wav.pcm", "ab");
	//fwrite(buffer, 1, numBytes, f);
	//fclose(f);

	// split to a multiple of 40ms chunks
	int chunkLength = (m_waveFormat.nSamplesPerSec / (1000 / 40)) * (m_waveFormat.wBitsPerSample / 8) * m_waveFormat.nChannels;

	size_t bytesLeft = numBytes;
	const uint8_t* origin = buffer;
	bool freeBuffer = false;

	if (m_cachedLen > 0)
	{
		uint8_t* newBuffer = new uint8_t[m_cachedLen + numBytes];
		memcpy(&newBuffer[0], m_cachedBytes, m_cachedLen);
		memcpy(&newBuffer[m_cachedLen], buffer, numBytes);

		buffer = newBuffer;

		origin = buffer;
		bytesLeft = m_cachedLen + numBytes;

		freeBuffer = true;
	}

	while (bytesLeft >= chunkLength)
	{
		// resample
		uint32_t numSamples = chunkLength / (m_waveFormat.nBlockAlign / m_waveFormat.nChannels);

		int32_t outLineSize;
		uint32_t outSize = av_samples_get_buffer_size(&outLineSize, 2, numSamples / m_waveFormat.nChannels, AV_SAMPLE_FMT_S16, 0);

		uint32_t outSamples = avresample_get_delay(m_avr) + (numSamples / m_waveFormat.nChannels);

		outSamples = av_rescale_rnd(outSamples, 48000, m_waveFormat.nSamplesPerSec, AV_ROUND_UP);

		outSamples += avresample_available(m_avr);

		m_resampledBytes = (uint8_t*)av_realloc_array(m_resampledBytes, 1, outSize);

		outSamples = avresample_convert(m_avr, &m_resampledBytes, outLineSize, outSamples, (uint8_t**)&origin, chunkLength, numSamples / m_waveFormat.nChannels);

		outSize = outSamples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * 2;

		// increment origin
		origin += chunkLength;
		bytesLeft -= chunkLength;

		// encode
		int len = opus_encode(m_opus, (const int16_t*)m_resampledBytes, 960 * 2, m_encodedBytes, sizeof(m_encodedBytes));

		if (len < 0)
		{
			trace("opus error %d\n", len);
			return;
		}

		// store packet
		EnqueueOpusPacket(std::string((char*)m_encodedBytes, len));
	}

	if (bytesLeft > 0)
	{
		int clen = std::min(sizeof(m_cachedBytes), bytesLeft);
		memcpy(m_cachedBytes, origin, clen);

		m_cachedLen = clen;
	}
	else
	{
		m_cachedLen = 0;
	}

	if (freeBuffer)
	{
		delete[] buffer;
	}

	SendQueuedOpusPackets();
}

void MumbleAudioInput::EnqueueOpusPacket(std::string packet)
{
	m_opusPackets.push(packet);
}

void MumbleAudioInput::SendQueuedOpusPackets()
{
	if (m_opusPackets.empty())
	{
		return;
	}

	char outBuf[16384];
	PacketDataStream buffer(outBuf, sizeof(outBuf));

	buffer.append((4 << 5));

	buffer << m_sequence;

	while (!m_opusPackets.empty())
	{
		auto packet = m_opusPackets.front();
		m_opusPackets.pop();

		//buffer.append(packet.size() | ((m_opusPackets.empty()) ? (1 << 7) : 0));
		buffer << packet.size();
		buffer.append(packet.c_str(), packet.size());

		m_sequence++;
	}

	trace("sent opus packets @ %d %lld\n", GetTickCount(), m_sequence);

	m_client->Send(MumbleMessageType::UDPTunnel, outBuf, buffer.size());
}

HRESULT MumbleAudioInput::HandleIncomingAudio()
{
	uint32_t packetLength = 1;
	//HRESULT err = m_audioCaptureClient->GetNextPacketSize(&packetLength);

	HRESULT err = S_OK;

	if (SUCCEEDED(err))
	{
		while (packetLength > 0)
		{
			uint8_t* data;
			uint32_t numFramesAvailable;
			DWORD flags;

			err = m_audioCaptureClient->GetBuffer(&data, &numFramesAvailable, &flags, nullptr, nullptr);

			if (FAILED(err))
			{
				break;
			}

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
			{
				data = nullptr;
			}

			HandleData(data, numFramesAvailable * m_waveFormat.nBlockAlign);

			err = m_audioCaptureClient->ReleaseBuffer(numFramesAvailable);

			if (FAILED(err))
			{
				break;
			}

			/*err = m_audioCaptureClient->GetNextPacketSize(&packetLength);

			if (FAILED(err))
			{
				break;
			}*/

			break;
		}
	}

	return err;
}

void MumbleAudioInput::InitializeAudioDevice()
{
	ComPtr<IMMDevice> device;

	if (FAILED(m_mmDeviceEnumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, device.ReleaseAndGetAddressOf())))
	{
		return;
	}

	if (FAILED(device->Activate(IID_IAudioClient, CLSCTX_INPROC_SERVER, nullptr, (void**)m_audioClient.ReleaseAndGetAddressOf())))
	{
		return;
	}

	WAVEFORMATEX* waveFormat;

	m_audioClient->GetMixFormat(&waveFormat);

	if (FAILED(m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 20 * 10000, 0, waveFormat, nullptr)))
	{
		return;
	}

	uint32_t bufferSize;

	m_audioClient->GetBufferSize(&bufferSize);

	if (FAILED(m_audioClient->GetService(IID_IAudioCaptureClient, (void**)m_audioCaptureClient.ReleaseAndGetAddressOf())))
	{
		return;
	}

	WAVEFORMATEXTENSIBLE* formatEx = (WAVEFORMATEXTENSIBLE*)waveFormat;

	SetInputFormat(waveFormat);

	m_avr = avresample_alloc_context();

	// channel layout
	int channelLayout = AV_CH_LAYOUT_MONO;

	if (m_waveFormat.nChannels == 2)
	{
		channelLayout = AV_CH_LAYOUT_STEREO_DOWNMIX;
	}
	else if (m_waveFormat.nChannels == 4)
	{
		channelLayout = AV_CH_LAYOUT_QUAD;
	}

	// sample format
	int sampleFormat = AV_SAMPLE_FMT_S16;

	if (formatEx->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
	{
		sampleFormat = AV_SAMPLE_FMT_FLT;
	}
	else if (m_waveFormat.wBitsPerSample == 8)
	{
		sampleFormat = AV_SAMPLE_FMT_U8;
	}
	else if (m_waveFormat.wBitsPerSample == 32)
	{
		sampleFormat = AV_SAMPLE_FMT_S32;
	}

	int sampleRate = m_waveFormat.nSamplesPerSec;

	av_opt_set_int(m_avr, "in_channel_layout", channelLayout, 0);
	av_opt_set_int(m_avr, "in_sample_fmt", sampleFormat, 0);
	av_opt_set_int(m_avr, "in_sample_rate", sampleRate, 0);
	av_opt_set_int(m_avr, "out_channel_layout", AV_CH_LAYOUT_STEREO_DOWNMIX, 0);
	av_opt_set_int(m_avr, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_int(m_avr, "out_sample_rate", 48000, 0);

	avresample_open(m_avr);

	int error;
	m_opus = opus_encoder_create(48000, 2, OPUS_APPLICATION_VOIP, &error);

	opus_encoder_ctl(m_opus, OPUS_SET_BITRATE(32000));

	// set event handle
	m_audioClient->SetEventHandle(m_startEvent);
}

void MumbleAudioInput::Enable()
{
	SetEvent(m_startEvent);
}

void MumbleAudioInput::ThreadStart(MumbleAudioInput* instance)
{
	HANDLE mmcssHandle;
	DWORD mmcssTaskIndex;

	mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
	instance->ThreadFunc();
}