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

	bool recreateDevice = false;

	while (true)
	{
		if (m_likelihood != lastLikelihood)
		{
			m_apm->voice_detection()->set_likelihood(ConvertLikelihood(m_likelihood));
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

		WaitForSingleObject(m_startEvent, INFINITE);

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

	// split to a multiple of 40ms chunks
	int chunkLength = (m_waveFormat.nSamplesPerSec / (1000 / 10)) * (m_waveFormat.wBitsPerSample / 8) * m_waveFormat.nChannels;

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
		uint32_t numSamples = chunkLength / m_waveFormat.nBlockAlign;

		int outSamples = av_rescale_rnd(swr_get_delay(m_avr, m_waveFormat.nSamplesPerSec) + numSamples, (int)48000, m_waveFormat.nSamplesPerSec, AV_ROUND_UP);

		av_samples_alloc(&m_resampledBytes, NULL, m_waveFormat.nChannels, outSamples, AV_SAMPLE_FMT_S16, 0);
		outSamples = swr_convert(m_avr, &m_resampledBytes, outSamples, (const uint8_t **)&origin, numSamples);

		// increment origin
		origin += chunkLength;
		bytesLeft -= chunkLength;

		// is this voice?
		webrtc::AudioFrame frame;
		frame.num_channels_ = 1;
		frame.sample_rate_hz_ = 48000;
		frame.samples_per_channel_ = 480;
		memcpy(frame.data_, m_resampledBytes, 480 * sizeof(int16_t));

		m_apm->ProcessStream(&frame);

		auto db = (float)-(m_apm->level_estimator()->RMS());
		m_audioLevel = XAudio2DecibelsToAmplitudeRatio(db);

		if (m_mode == MumbleActivationMode::VoiceActivity && !m_apm->voice_detection()->stream_has_voice())
		{
			m_isTalking = false;
			m_audioLevel = 0.0f;
			continue;
		}

		m_isTalking = true;

		memcpy(m_resampledBytes, frame.data_, 480 * sizeof(int16_t));

		/*if (fvad_process(m_fvad, (const int16_t*)m_resampledBytes, 480) != 1)
		{
			continue;
		}*/

		// encode
		int len = opus_encode(m_opus, (const int16_t*)m_resampledBytes, 480, m_encodedBytes, sizeof(m_encodedBytes));

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
		buffer << (packet.size());
		buffer.append(packet.c_str(), packet.size());

		m_sequence++;
	}

	//buffer << uint64_t(1 << 13);

	// send placeholder position
	buffer << m_positionX;
	buffer << m_positionY;
	buffer << m_positionZ;

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

			break;
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

	if (FAILED(hr = device->Activate(IID_IAudioClient, CLSCTX_INPROC_SERVER, nullptr, (void**)m_audioClient.ReleaseAndGetAddressOf())))
	{
		trace(__FUNCTION__ ": Activating IAudioClient for capture device failed. HR = %08x\n", hr);
		return;
	}

	WAVEFORMATEX* waveFormat;

	m_audioClient->GetMixFormat(&waveFormat);

	if (FAILED(hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 20 * 10000, 0, waveFormat, nullptr)))
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

	m_avr = swr_alloc_set_opts(m_avr, AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, 48000, channelLayout, sampleFormat, sampleRate, 0, NULL);

	swr_init(m_avr);

	int error;
	m_opus = opus_encoder_create(48000, 1, OPUS_APPLICATION_VOIP, &error);

	opus_encoder_ctl(m_opus, OPUS_SET_BITRATE(32000));

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

	mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
	instance->ThreadFunc();
}
