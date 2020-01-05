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
#include <libswresample/swresample.h>
};

MumbleAudioInput::MumbleAudioInput()
	: m_likelihood(MumbleVoiceLikelihood::ModerateLikelihood), m_ptt(false), m_mode(MumbleActivationMode::VoiceActivity), m_deviceId(""), m_audioLevel(0.0f),
	  m_avr(nullptr), m_opus(nullptr), m_apm(nullptr), m_isTalking(false)
{

}

void MumbleAudioInput::Initialize()
{
	m_startEvent = CreateEvent(0, 0, 0, 0);

	m_thread = std::thread(ThreadStart, this);

	m_resampledBytes = nullptr;

	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_sequence = 0;
	m_cachedLen = 0;
}

static webrtc::VoiceDetection::Likelihood ConvertLikelihood(MumbleVoiceLikelihood likelihood);

void MumbleAudioInput::SetActivationMode(MumbleActivationMode mode)
{
	m_mode = mode;
}

void MumbleAudioInput::SetActivationLikelihood(MumbleVoiceLikelihood likelihood)
{
	m_likelihood = likelihood;
}

void MumbleAudioInput::SetPTTButtonState(bool pressed)
{
	m_ptt = pressed;
}

void MumbleAudioInput::SetAudioDevice(const std::string& dsoundDeviceId)
{
	m_deviceId = dsoundDeviceId;
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

	// save settings that occurred during creation now, since they might change while we wait to activate
	MumbleVoiceLikelihood lastLikelihood = m_likelihood;
	std::string lastDevice = m_deviceId;

	// wait for the activation signal
	WaitForSingleObject(m_startEvent, INFINITE);

	// go!
	if (m_audioClient.Get())
	{
		m_audioClient->Start();
	}

	m_audioBuffer.resize((m_waveFormat.nSamplesPerSec / 100) * m_waveFormat.nBlockAlign);

	bool recreateDevice = false;

	while (true)
	{
		if (m_likelihood != lastLikelihood)
		{
			if (m_apm)
			{
				m_apm->voice_detection()->set_likelihood(ConvertLikelihood(m_likelihood));
			}

			lastLikelihood = m_likelihood;
		}

		if (lastDevice != m_deviceId)
		{
			recreateDevice = true;
			lastDevice = m_deviceId;
		}

		if (recreateDevice)
		{
			InitializeAudioDevice();

			if (m_audioCaptureClient.Get())
			{
				m_audioClient->Start();
			}

			recreateDevice = false;
		}

		WaitForSingleObject(m_startEvent, 2000);

		HRESULT hr = HandleIncomingAudio();

		if (FAILED(hr))
		{
			if (hr == AUDCLNT_E_DEVICE_INVALIDATED)
			{
				recreateDevice = true;

				continue;
			}

			continue;
		}
	}
}

void MumbleAudioInput::HandleData(const uint8_t* buffer, size_t numBytes)
{
	if (m_mode == MumbleActivationMode::Disabled)
	{
		m_isTalking = false;
		m_audioLevel = 0.0f;
		return;
	}

	if (m_mode == MumbleActivationMode::PushToTalk && !m_ptt)
	{
		m_isTalking = false;
		m_audioLevel = 0.0f;
		return;
	}

	int frameSize = 40;

	// split to a multiple of 20ms chunks
	int chunkLength = (m_waveFormat.nSamplesPerSec / (1000 / frameSize)) * (m_waveFormat.wBitsPerSample / 8) * m_waveFormat.nChannels;

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
		if (!m_avr)
		{
			continue;
		}

		// resample
		uint32_t numSamples = chunkLength / m_waveFormat.nBlockAlign;

		int outSamples = av_rescale_rnd(swr_get_delay(m_avr, m_waveFormat.nSamplesPerSec) + numSamples, (int)48000, m_waveFormat.nSamplesPerSec, AV_ROUND_UP);

		av_samples_alloc(&m_resampledBytes, NULL, 1, outSamples, AV_SAMPLE_FMT_S16, 0);
		outSamples = swr_convert(m_avr, &m_resampledBytes, outSamples, (const uint8_t **)&origin, numSamples);

		// increment origin
		origin += chunkLength;
		bytesLeft -= chunkLength;

		// skip if APM is NULL
		if (!m_apm)
		{
			av_freep(&m_resampledBytes);

			continue;
		}

		int numVoice = 0;

		for (int off = 0; off < frameSize; off += 10)
		{
			int frameStart = (off * 48 * sizeof(int16_t)); // 1ms = 48 samples

			// is this voice?
			webrtc::AudioFrame frame;
			frame.num_channels_ = 1;
			frame.sample_rate_hz_ = 48000;
			frame.samples_per_channel_ = 480;
			memcpy(frame.data_, &m_resampledBytes[frameStart], 480 * sizeof(int16_t));

			m_apm->ProcessStream(&frame);

			auto db = (float)-(m_apm->level_estimator()->RMS());
			m_audioLevel = XAudio2DecibelsToAmplitudeRatio(db);

			if (m_apm->voice_detection()->stream_has_voice())
			{
				numVoice++;
			}

			memcpy(&m_resampledBytes[frameStart], frame.data_, 480 * sizeof(int16_t));
		}

		if (m_mode == MumbleActivationMode::VoiceActivity && numVoice < 2)
		{
			m_isTalking = false;
			m_audioLevel = 0.0f;

			av_freep(&m_resampledBytes);

			continue;
		}

		m_isTalking = true;

		// encode
		int len = opus_encode(m_opus, (const int16_t*)m_resampledBytes, frameSize * 48, m_encodedBytes, sizeof(m_encodedBytes));

		av_freep(&m_resampledBytes);

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

	while (!m_opusPackets.empty())
	{
		auto packet = m_opusPackets.front();
		m_opusPackets.pop();

		char outBuf[16384];
		PacketDataStream buffer(outBuf, sizeof(outBuf));

		buffer.append((4 << 5) | (m_client->GetVoiceTarget() & 31));

		buffer << m_sequence;

		buffer << (packet.size() | ((m_opusPackets.empty()) ? (1 << 13) : 0));
		buffer.append(packet.c_str(), packet.size());

		m_sequence++;

		//buffer << uint64_t(1 << 13);

		// send placeholder position
		buffer << m_positionX;
		buffer << m_positionY;
		buffer << m_positionZ;

		// extension: send our voice distance
		buffer << m_voiceDistance;

		m_client->SendVoice(outBuf, buffer.size());
	}
}

HRESULT MumbleAudioInput::HandleIncomingAudio()
{
	if (!m_audioCaptureClient)
	{
		return S_FALSE;
	}

	uint32_t packetLength = 0;
	HRESULT err = m_audioCaptureClient->GetNextPacketSize(&packetLength);

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

			size_t size = numFramesAvailable * m_waveFormat.nBlockAlign;
			if (size > m_audioBuffer.size())
			{
				m_audioBuffer.resize(size);
			}

			memcpy(&m_audioBuffer[0], data, size);

			err = m_audioCaptureClient->ReleaseBuffer(numFramesAvailable);

			if (FAILED(err))
			{
				break;
			}

			HandleData(&m_audioBuffer[0], size);

			if (FAILED(m_audioCaptureClient->GetNextPacketSize(&packetLength)))
			{
				break;
			}
		}
	}

	return err;
}

static webrtc::VoiceDetection::Likelihood ConvertLikelihood(MumbleVoiceLikelihood likelihood)
{
	switch (likelihood)
	{
	case MumbleVoiceLikelihood::HighLikelihood:
		return webrtc::VoiceDetection::kHighLikelihood;

	case MumbleVoiceLikelihood::ModerateLikelihood:
		return webrtc::VoiceDetection::kModerateLikelihood;

	case MumbleVoiceLikelihood::LowLikelihood:
		return webrtc::VoiceDetection::kLowLikelihood;

	case MumbleVoiceLikelihood::VeryLowLikelihood:
		return webrtc::VoiceDetection::kVeryLowLikelihood;
	}

	return webrtc::VoiceDetection::kModerateLikelihood;
}

WRL::ComPtr<IMMDevice> GetMMDeviceFromGUID(bool input, const std::string& guid);

void DuckingOptOut(WRL::ComPtr<IMMDevice> device);

void MumbleAudioInput::InitializeAudioDevice()
{
	// destroy
	if (m_avr)
	{
		swr_free(&m_avr);
	}

	if (m_apm)
	{
		delete m_apm;
		m_apm = nullptr;
	}

	if (m_opus)
	{
		opus_encoder_destroy(m_opus);
		m_opus = nullptr;
	}

	// create
	HRESULT hr = 0;
	ComPtr<IMMDevice> device;

	if (m_deviceId.empty())
	{
		if (FAILED(hr = m_mmDeviceEnumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, device.ReleaseAndGetAddressOf())))
		{
			trace(__FUNCTION__ ": Obtaining default audio endpoint failed. HR = 0x%08x\n", hr);
			return;
		}
	}
	else
	{
		device = GetMMDeviceFromGUID(true, m_deviceId);

		if (!device)
		{
			trace(__FUNCTION__ ": Obtaining audio device for %s failed.\n", m_deviceId);
			return;
		}
	}

	DuckingOptOut(device);

	if (FAILED(hr = device->Activate(IID_IAudioClient, CLSCTX_INPROC_SERVER, nullptr, (void**)m_audioClient.ReleaseAndGetAddressOf())))
	{
		trace(__FUNCTION__ ": Activating IAudioClient for capture device failed. HR = %08x\n", hr);
		return;
	}

	WAVEFORMATEX* waveFormat;

	m_audioClient->GetMixFormat(&waveFormat);

	if (FAILED(hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 0, 0, waveFormat, nullptr)))
	{
		trace(__FUNCTION__ ": Initializing IAudioClient for capture device failed. HR = %08x\n", hr);
		return;
	}

	uint32_t bufferSize;

	m_audioClient->GetBufferSize(&bufferSize);

	if (FAILED(hr = m_audioClient->GetService(IID_IAudioCaptureClient, (void**)m_audioCaptureClient.ReleaseAndGetAddressOf())))
	{
		trace(__FUNCTION__ ": Initializing IAudioCaptureClient for capture device failed. HR = %08x\n", hr);
		return;
	}

	WAVEFORMATEXTENSIBLE* formatEx = (WAVEFORMATEXTENSIBLE*)waveFormat;

	SetInputFormat(waveFormat);

	m_avr = swr_alloc();

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
	AVSampleFormat sampleFormat = AV_SAMPLE_FMT_S16;

	if (waveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
		(waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE && formatEx->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
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

	m_avr = swr_alloc_set_opts(m_avr, AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, 48000, channelLayout, sampleFormat, sampleRate, 0, NULL);

	swr_init(m_avr);

	int error;
	m_opus = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &error);

	opus_encoder_ctl(m_opus, OPUS_SET_BITRATE(24000));

	// set event handle
	m_audioClient->SetEventHandle(m_startEvent);

	webrtc::Config config;
	config.Set < webrtc::ExtendedFilter >
		(new webrtc::ExtendedFilter(true));
	config.Set < webrtc::ExperimentalAgc >
		(new webrtc::ExperimentalAgc(true, 12));
	config.Set < webrtc::DelayAgnostic >
		(new webrtc::DelayAgnostic(true));

	m_apm = webrtc::AudioProcessing::Create(config);

	webrtc::ProcessingConfig pconfig;

	pconfig.streams[webrtc::ProcessingConfig::kInputStream] =
		webrtc::StreamConfig(48000, 1, false);
	pconfig.streams[webrtc::ProcessingConfig::kOutputStream] =
		webrtc::StreamConfig(48000, 1, false);
	pconfig.streams[webrtc::ProcessingConfig::kReverseInputStream] =
		webrtc::StreamConfig(48000, 1, false);
	pconfig.streams[webrtc::ProcessingConfig::kReverseOutputStream] =
		webrtc::StreamConfig(48000, 1, false);

	m_apm->Initialize(pconfig);

	m_apm->high_pass_filter()->Enable(true);
	m_apm->echo_cancellation()->Enable(false);
	m_apm->noise_suppression()->Enable(true);
	m_apm->level_estimator()->Enable(true);
	m_apm->voice_detection()->set_likelihood(ConvertLikelihood(m_likelihood));
	m_apm->voice_detection()->set_frame_size_ms(10);
	m_apm->voice_detection()->Enable(true);

	m_apm->gain_control()->set_mode(webrtc::GainControl::kAdaptiveDigital);
	m_apm->gain_control()->set_target_level_dbfs(3);
	m_apm->gain_control()->set_compression_gain_db(9);
	m_apm->gain_control()->enable_limiter(true);
	m_apm->gain_control()->Enable(true);

	trace(__FUNCTION__ ": Initialized audio capture device.\n");
}

void MumbleAudioInput::Enable()
{
	SetEvent(m_startEvent);
}

void MumbleAudioInput::ThreadStart(MumbleAudioInput* instance)
{
	HANDLE mmcssHandle;
	DWORD mmcssTaskIndex;

	mmcssHandle = AvSetMmThreadCharacteristics(L"Pro Audio", &mmcssTaskIndex);
	instance->ThreadFunc();
}
