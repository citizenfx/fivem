/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleAudioOutput.h"
#include <avrt.h>
#include <sstream>
#include <PacketDataStream.h>
#include <MumbleClientImpl.h>
#include <MumbleClientState.h>

#include <Error.h>

#define INITGUID
#include <guiddef.h>
DEFINE_GUID(DEVINTERFACE_AUDIO_RENDER, 0xe6327cad, 0xdcec, 0x4949, 0xae, 0x8a, 0x99, 0x1e, 0x97, 0x6a, 0x79, 0xd2);
#undef INITGUID

#pragma comment(lib, "xaudio2.lib")

// XA2 Win7 API
#define DECLSPEC_UUID_WRAPPER(x) __declspec(uuid(#x))

#define DEFINE_IID(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
            interface DECLSPEC_UUID_WRAPPER(l##-##w1##-##w2##-##b1##b2##-##b3##b4##b5##b6##b7##b8) interfaceName; \
            EXTERN_C const GUID DECLSPEC_SELECTANY IID_##interfaceName = __uuidof(interfaceName)

DEFINE_IID(IXAudio2Legacy, 8bcf1f58, 9fe7, 4583, 8a, c6, e2, ad, c4, 65, c8, bb);

DECLARE_INTERFACE_(IXAudio2Legacy, IUnknown)
{
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, __deref_out void** ppvInterface) PURE;

	STDMETHOD_(ULONG, AddRef) (THIS) PURE;

	STDMETHOD_(ULONG, Release) (THIS) PURE;

	STDMETHOD(GetDeviceCount) (THIS_ __out UINT32* pCount) PURE;

	STDMETHOD(GetDeviceDetails) (THIS_ UINT32 Index, __out void* pDeviceDetails) PURE;

	STDMETHOD(Initialize) (THIS_ UINT32 Flags X2DEFAULT(0),
		XAUDIO2_PROCESSOR XAudio2Processor X2DEFAULT(XAUDIO2_DEFAULT_PROCESSOR)) PURE;

	STDMETHOD(RegisterForCallbacks) (__in IXAudio2EngineCallback* pCallback) PURE;

	STDMETHOD_(void, UnregisterForCallbacks) (__in IXAudio2EngineCallback* pCallback) PURE;

	STDMETHOD(CreateSourceVoice) (THIS_ __deref_out IXAudio2SourceVoice** ppSourceVoice,
		__in const WAVEFORMATEX* pSourceFormat,
		UINT32 Flags X2DEFAULT(0),
		float MaxFrequencyRatio X2DEFAULT(XAUDIO2_DEFAULT_FREQ_RATIO),
		__in_opt IXAudio2VoiceCallback* pCallback X2DEFAULT(NULL),
		__in_opt const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL),
		__in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;

	STDMETHOD(CreateSubmixVoice) (THIS_ __deref_out IXAudio2SubmixVoice** ppSubmixVoice,
		UINT32 InputChannels, UINT32 InputSampleRate,
		UINT32 Flags X2DEFAULT(0), UINT32 ProcessingStage X2DEFAULT(0),
		__in_opt const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL),
		__in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;


	STDMETHOD(CreateMasteringVoice) (THIS_ __deref_out IXAudio2MasteringVoice** ppMasteringVoice,
		UINT32 InputChannels X2DEFAULT(XAUDIO2_DEFAULT_CHANNELS),
		UINT32 InputSampleRate X2DEFAULT(XAUDIO2_DEFAULT_SAMPLERATE),
		UINT32 Flags X2DEFAULT(0), UINT32 DeviceIndex X2DEFAULT(0),
		__in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;

	STDMETHOD(StartEngine) (THIS) PURE;

	STDMETHOD_(void, StopEngine) (THIS) PURE;

	STDMETHOD(CommitChanges) (THIS_ UINT32 OperationSet) PURE;

	STDMETHOD_(void, GetPerformanceData) (THIS_ __out XAUDIO2_PERFORMANCE_DATA* pPerfData) PURE;

	STDMETHOD_(void, SetDebugConfiguration) (THIS_ __in_opt const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration,
		__in_opt __reserved void* pReserved X2DEFAULT(NULL)) PURE;
};

DEFINE_IID(XAudio2Legacy, 5a508685, a254, 4fba, 9b, 82, 9a, 24, b0, 03, 06, af);

class XAudio2DownlevelWrap : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IXAudio2>
{
private:
	WRL::ComPtr<IXAudio2Legacy> m_origPtr;

public:
	XAudio2DownlevelWrap()
	{
		HMODULE xAudio2 = LoadLibrary(L"XAudio2_7.dll");

		HRESULT hr = CoCreateInstance(IID_XAudio2Legacy,
			NULL, CLSCTX_INPROC_SERVER, __uuidof(IXAudio2Legacy), (void**)&m_origPtr);
		if (SUCCEEDED(hr))
		{
			hr = m_origPtr->Initialize(0, 1);
		}
	}

	STDMETHOD(RegisterForCallbacks) (__in IXAudio2EngineCallback* pCallback)
	{
		return m_origPtr->RegisterForCallbacks(pCallback);
	}

	STDMETHOD_(void, UnregisterForCallbacks) (__in IXAudio2EngineCallback* pCallback)
	{
		return m_origPtr->UnregisterForCallbacks(pCallback);
	}

	STDMETHOD(CreateSourceVoice) (THIS_ __deref_out IXAudio2SourceVoice** ppSourceVoice,
		__in const WAVEFORMATEX* pSourceFormat,
		UINT32 Flags X2DEFAULT(0),
		float MaxFrequencyRatio X2DEFAULT(XAUDIO2_DEFAULT_FREQ_RATIO),
		__in_opt IXAudio2VoiceCallback* pCallback X2DEFAULT(NULL),
		__in_opt const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL),
		__in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL))
	{
		return m_origPtr->CreateSourceVoice(ppSourceVoice, pSourceFormat, Flags, MaxFrequencyRatio, pCallback, pSendList, pEffectChain);
	}

	STDMETHOD(CreateSubmixVoice) (THIS_ __deref_out IXAudio2SubmixVoice** ppSubmixVoice,
		UINT32 InputChannels, UINT32 InputSampleRate,
		UINT32 Flags X2DEFAULT(0), UINT32 ProcessingStage X2DEFAULT(0),
		__in_opt const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL),
		__in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL))
	{
		return m_origPtr->CreateSubmixVoice(ppSubmixVoice, InputChannels, InputSampleRate, Flags, ProcessingStage, pSendList, pEffectChain);
	}


	STDMETHOD(CreateMasteringVoice) (THIS_ _Outptr_ IXAudio2MasteringVoice** ppMasteringVoice,
		UINT32 InputChannels X2DEFAULT(XAUDIO2_DEFAULT_CHANNELS),
		UINT32 InputSampleRate X2DEFAULT(XAUDIO2_DEFAULT_SAMPLERATE),
		UINT32 Flags X2DEFAULT(0), _In_opt_z_ LPCWSTR szDeviceId X2DEFAULT(NULL),
		_In_opt_ const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL),
		_In_ AUDIO_STREAM_CATEGORY StreamCategory X2DEFAULT(AudioCategory_GameEffects))
	{
		return m_origPtr->CreateMasteringVoice(ppMasteringVoice, InputChannels, InputSampleRate, Flags, 0, pEffectChain);
	}

	STDMETHOD(StartEngine) (THIS)
	{
		return m_origPtr->StartEngine();
	}

	STDMETHOD_(void, StopEngine) (THIS)
	{
		return m_origPtr->StopEngine();
	}

	STDMETHOD(CommitChanges) (THIS_ UINT32 OperationSet)
	{
		return m_origPtr->CommitChanges(OperationSet);
	}

	STDMETHOD_(void, GetPerformanceData) (THIS_ __out XAUDIO2_PERFORMANCE_DATA* pPerfData)
	{
		return m_origPtr->GetPerformanceData(pPerfData);
	}

	STDMETHOD_(void, SetDebugConfiguration) (THIS_ __in_opt const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration,
		__in_opt __reserved void* pReserved X2DEFAULT(NULL))
	{
		return m_origPtr->SetDebugConfiguration(pDebugConfiguration, pReserved);
	}
};


void MumbleAudioOutput::Initialize()
{
	m_distance = FLT_MAX;
	m_volume = 1.0f;
	m_masteringVoice = nullptr;
	m_thread = std::thread([this] { ThreadFunc(); });

	int error;
	m_opus = opus_decoder_create(48000, 1, &error);
}

MumbleAudioOutput::ClientAudioState::ClientAudioState()
	: volume(1.0f), sequence(0), voice(nullptr), isTalking(false)
{
	position[0] = 0.0f;
	position[1] = 0.0f;
	position[2] = 0.0f;
}

MumbleAudioOutput::ClientAudioState::~ClientAudioState()
{
	if (voice)
	{
		voice->DestroyVoice();
		voice = nullptr;
	}
}

void MumbleAudioOutput::ClientAudioState::OnBufferEnd(void* cxt)
{
	auto buffer = reinterpret_cast<int16_t*>(cxt);

	delete[] buffer;

	XAUDIO2_VOICE_STATE vs;
	voice->GetState(&vs);

	if (vs.BuffersQueued == 0)
	{
		isTalking = false;
	}
}

void MumbleAudioOutput::HandleClientConnect(const MumbleUser& user)
{
	while (!m_masteringVoice)
	{
		Sleep(1);
	}

	WAVEFORMATEX format;
	format.cbSize = sizeof(WAVEFORMATEX);
	format.nSamplesPerSec = 48000;
	format.wBitsPerSample = 16;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = (format.nBlockAlign * format.nSamplesPerSec);

	auto state = std::make_shared<ClientAudioState>();

	IXAudio2SourceVoice* voice = nullptr;
	m_xa2->CreateSourceVoice(&voice, &format, 0, 2.0f, state.get());

	voice->Start();

	state->voice = voice;

	m_clients[user.GetSessionId()] = state;
}

void MumbleAudioOutput::HandleClientVoiceData(const MumbleUser& user, uint64_t sequence, const uint8_t* data, size_t size)
{
	auto client = m_clients[user.GetSessionId()];

	if (!client)
	{
		return;
	}

	/*if (sequence < client->sequence)
	{
		return;
	}*/

	client->sequence = sequence;

	auto voiceBuffer = new int16_t[5760 * 1];
	int len = opus_decode(m_opus, data, size, voiceBuffer, 5760, 0);

	if (len >= 0)
	{
		XAUDIO2_BUFFER bufferData;
		bufferData.LoopBegin = 0;
		bufferData.LoopCount = 0;
		bufferData.LoopLength = 0;
		bufferData.AudioBytes = len * sizeof(int16_t);
		bufferData.Flags = 0;
		bufferData.pAudioData = reinterpret_cast<BYTE*>(voiceBuffer);
		bufferData.pContext = voiceBuffer;
		bufferData.PlayBegin = 0;
		bufferData.PlayLength = len;

		client->voice->SubmitSourceBuffer(&bufferData);

		client->isTalking = true;
	}
	else
	{
		delete[] voiceBuffer;
	}
}

void MumbleAudioOutput::HandleClientPosition(const MumbleUser& user, float position[3])
{
	auto client = m_clients[user.GetSessionId()];

	if (client)
	{
		client->position[0] = position[0];
		client->position[1] = position[1];
		client->position[2] = position[2];

		if (position[0] != 0.0f || position[1] != 0.0f || position[2] != 0.0f)
		{
			float coeffs[2] = { 0 };

			X3DAUDIO_DSP_SETTINGS dsp = { 0 };
			dsp.SrcChannelCount = 1;
			dsp.DstChannelCount = 2;
			dsp.pMatrixCoefficients = coeffs;

			X3DAUDIO_EMITTER emitter = { 0 };
			emitter.OrientFront = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
			emitter.OrientTop = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
			emitter.Position = DirectX::XMFLOAT3(position);
			emitter.ChannelCount = 1;
			//emitter.CurveDistanceScaler = 50.0f;
			emitter.CurveDistanceScaler = m_distance;

			m_x3daCalculate(m_x3da, &m_listener, &emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB, &dsp);

			client->voice->SetOutputMatrix(m_masteringVoice, 1, 2, dsp.pMatrixCoefficients);
		}
		else
		{
			float matrix[2] = { 1.f, 1.f };
			client->voice->SetOutputMatrix(m_masteringVoice, 1, 2, matrix);
		}
	}
}

void MumbleAudioOutput::SetMatrix(float position[3], float front[3], float up[3])
{
	m_listener.OrientFront = DirectX::XMFLOAT3(front);
	m_listener.OrientTop = DirectX::XMFLOAT3(up);
	m_listener.Position = DirectX::XMFLOAT3(position);
}

void MumbleAudioOutput::HandleClientDisconnect(const MumbleUser& user)
{
	m_clients[user.GetSessionId()] = nullptr;
}

void MumbleAudioOutput::GetTalkers(std::vector<uint32_t>* talkers)
{
	talkers->clear();

	for (auto& client : m_clients)
	{
		if (!client.second)
		{
			continue;
		}

		if (client.second->isTalking)
		{
			talkers->push_back(client.first);
		}
	}
}

void MumbleAudioOutput::ThreadFunc()
{
	SetThreadName(-1, "[Mumble] Audio Output Thread");

	// initialize COM for the current thread
	CoInitialize(nullptr);

	HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, (void**)m_mmDeviceEnumerator.GetAddressOf());

	if (FAILED(hr))
	{
		return;
	}

	InitializeAudioDevice();
}

void MumbleAudioOutput::SetAudioDevice(const std::string& deviceId)
{
	if (m_deviceGuid == deviceId)
	{
		return;
	}

	m_deviceGuid = deviceId;

	// save IDs
	std::vector<uint32_t> m_ids;

	for (auto& client : m_clients)
	{
		if (client.second)
		{
			m_ids.push_back(client.first);
		}
	}

	// delete all clients
	m_clients.clear();

	// reinitialize audio device
	InitializeAudioDevice();

	// recreate clients
	for (auto& client : m_ids)
	{
		MumbleUser fakeUser(client);
		HandleClientConnect(fakeUser);
	}
}

void MumbleAudioOutput::SetDistance(float distance)
{
	m_distance = distance;
}

void MumbleAudioOutput::SetVolume(float volume)
{
	m_volume = volume;

	if (m_masteringVoice)
	{
		m_masteringVoice->SetVolume(volume);
	}
}

WRL::ComPtr<IMMDevice> GetMMDeviceFromGUID(bool input, const std::string& guid);

void MumbleAudioOutput::InitializeAudioDevice()
{
	ComPtr<IMMDevice> device;

	if (m_deviceGuid.empty())
	{
		if (FAILED(m_mmDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eCommunications, device.ReleaseAndGetAddressOf())))
		{
			return;
		}
	}
	else
	{
		device = GetMMDeviceFromGUID(false, m_deviceGuid);

		if (!device.Get())
		{
			return;
		}
	}

	auto xa2Dll = LoadLibraryExW(L"XAudio2_8.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

	if (xa2Dll)
	{
		auto _XAudio2Create = (decltype(&XAudio2Create))GetProcAddress(xa2Dll, "XAudio2Create");

		if (FAILED(_XAudio2Create(m_xa2.ReleaseAndGetAddressOf(), 0, 1)))
		{
			return;
		}
	}
	else
	{
		xa2Dll = LoadLibraryExW(L"XAudio2_7.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

		if (!xa2Dll)
		{
			trace("Could not load any XAudio DLL.\n");
			return;
		}

		m_xa2 = WRL::Make<XAudio2DownlevelWrap>();
	}

	XAUDIO2_DEBUG_CONFIGURATION cfg = { 0 };
	cfg.TraceMask = XAUDIO2_LOG_ERRORS;
	//cfg.BreakMask = XAUDIO2_LOG_ERRORS;
	m_xa2->SetDebugConfiguration(&cfg);

	LPWSTR deviceId;
	device->GetId(&deviceId);

	std::wstring deviceIdStr;
	deviceIdStr.reserve(112);
	deviceIdStr.append(L"\\\\?\\SWD#MMDEVAPI#");
	deviceIdStr.append(deviceId);
	deviceIdStr.push_back(L'#');
	size_t offset = deviceIdStr.size();
	deviceIdStr.resize(deviceIdStr.capacity());
	StringFromGUID2(DEVINTERFACE_AUDIO_RENDER, &deviceIdStr[offset], (int)(deviceIdStr.size() - offset));

	CoTaskMemFree(deviceId);

	if (FAILED(m_xa2->CreateMasteringVoice(&m_masteringVoice, 2, 48000, 0, deviceIdStr.c_str())))
	{
		return;
	}

	m_masteringVoice->SetVolume(m_volume);

	auto x3aDll = LoadLibraryExW(L"XAudio2_8.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

	if (!x3aDll)
	{
		x3aDll = LoadLibraryExW(L"X3DAudio1_7.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	}

	if (x3aDll)
	{
		auto _X3DAudioInitialize = (decltype(&X3DAudioInitialize))GetProcAddress(x3aDll, "X3DAudioInitialize");
		auto _X3DAudioCalculate = (decltype(&X3DAudioCalculate))GetProcAddress(x3aDll, "X3DAudioCalculate");

		DWORD channelMask = 0;
		m_masteringVoice->GetChannelMask(&channelMask);

		_X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, m_x3da);

		memset(&m_listener, 0, sizeof(m_listener));
		m_listener.Position = DirectX::XMFLOAT3(100.0f, 0.0f, 0.0f);
		m_listener.OrientFront = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_listener.OrientTop = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

		m_x3daCalculate = _X3DAudioCalculate;
	}
	else
	{
		m_x3daCalculate = nullptr;
	}
}
