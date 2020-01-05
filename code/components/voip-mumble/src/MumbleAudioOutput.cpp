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
#include <mmsystem.h>
#include <CoreConsole.h>

#include <xaudio2fx.h>

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

static std::shared_ptr<ConVar<bool>> g_use3dAudio;
static std::shared_ptr<ConVar<bool>> g_useSendingRangeOnly;

void MumbleAudioOutput::Initialize()
{
	g_use3dAudio = std::make_shared<ConVar<bool>>("voice_use3dAudio", ConVar_None, false);
	g_useSendingRangeOnly = std::make_shared<ConVar<bool>>("voice_useSendingRangeOnly", ConVar_None, false);

	m_initialized = false;
	m_distance = FLT_MAX;
	m_volume = 1.0f;
	m_masteringVoice = nullptr;
	m_submixVoice = nullptr;
	m_thread = std::thread([this]
	{
		ThreadFunc();

		// COM FLS cleanup will fail on thread exit and crash in an unloaded XAudio DLL, so instead we opt to spin
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(60));
		}
	});
}

MumbleAudioOutput::ClientAudioState::ClientAudioState()
	: volume(1.0f), sequence(0), voice(nullptr), opus(nullptr), isTalking(false), isAudible(true), overrideVolume(-1.0f)
{
	position[0] = 0.0f;
	position[1] = 0.0f;
	position[2] = 0.0f;
	distance = 0.0f;

	lastTime = timeGetTime();
}

MumbleAudioOutput::ClientAudioState::~ClientAudioState()
{
	if (voice)
	{
		voice->DestroyVoice();
		voice = nullptr;
	}

	if (opus)
	{
		opus_decoder_destroy(opus);
		opus = nullptr;
	}
}

void MumbleAudioOutput::ClientAudioState::OnBufferEnd(void* cxt)
{
	auto buffer = reinterpret_cast<int16_t*>(cxt);

	_aligned_free(buffer);

	XAUDIO2_VOICE_STATE vs;
	voice->GetState(&vs);

	if (vs.BuffersQueued == 0)
	{
		isTalking = false;
	}
}

void MumbleAudioOutput::HandleClientConnect(const MumbleUser& user)
{
	{
		std::unique_lock<std::mutex> initLock(m_initializeMutex);

		if (!m_initialized)
		{
			m_initializeVar.wait(initLock);
		}
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

	auto xa2 = m_xa2;

	XAUDIO2_SEND_DESCRIPTOR sendDescriptors[2];
	sendDescriptors[0].Flags = 0;// XAUDIO2_SEND_USEFILTER;
	sendDescriptors[0].pOutputVoice = m_masteringVoice;

	if (m_submixVoice)
	{
		sendDescriptors[1].Flags = 0;// XAUDIO2_SEND_USEFILTER;
		sendDescriptors[1].pOutputVoice = m_submixVoice;
	}

	const XAUDIO2_VOICE_SENDS sendList = { (m_submixVoice) ? 2 : 1, sendDescriptors };

	IXAudio2SourceVoice* voice = nullptr;
	HRESULT hr = xa2->CreateSourceVoice(&voice, &format, 0, 2.0f, state.get(), &sendList);

	if (FAILED(hr))
	{
		trace("CreateSourceVoice failed - HR = %08x\n", hr);
		return;
	}

	voice->Start();

	// disable volume initially, we will only set it once we get position data from the client
	voice->SetVolume(0.0f);

	state->voice = voice;

	int error;
	state->opus = opus_decoder_create(48000, 1, &error);

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

	auto voiceBuffer = (int16_t*)_aligned_malloc(5760 * 1 * sizeof(int16_t), 16);
	int len = opus_decode(client->opus, data, size, voiceBuffer, 5760, 0);

	if (len >= 0)
	{
		// work around XA2.7 issue (for Win7) where >64 buffers being enqueued are a fatal error (leading to __debugbreak)
		// "SimpList: non-growable list ran out of room for new elements"

		XAUDIO2_VOICE_STATE vs;
		client->voice->GetState(&vs);

		if (vs.BuffersQueued > 48)
		{
			// return, waiting for buffers to play back
			// flushing buffers would be helpful, but would lead to memory leaks
			// (and wouldn't be instant, either)
			_aligned_free(voiceBuffer);

			return;
		}

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

		client->isTalking = client->isAudible;
	}
	else
	{
		_aligned_free(voiceBuffer);
	}
}

static const X3DAUDIO_DISTANCE_CURVE_POINT Emitter_LFE_CurvePoints[3] = { 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f };
static const X3DAUDIO_DISTANCE_CURVE       Emitter_LFE_Curve = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_LFE_CurvePoints[0], 3 };

static const X3DAUDIO_DISTANCE_CURVE_POINT Emitter_Reverb_CurvePoints[3] = { 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, 0.0f };
static const X3DAUDIO_DISTANCE_CURVE       Emitter_Reverb_Curve = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_Reverb_CurvePoints[0], 3 };

static const X3DAUDIO_CONE Listener_DirectionalCone = { X3DAUDIO_PI*5.0f / 6.0f, X3DAUDIO_PI*11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };

void MumbleAudioOutput::HandleClientDistance(const MumbleUser& user, float distance)
{
	auto client = m_clients[user.GetSessionId()];

	if (client)
	{
		client->distance = distance;
	}
}

void MumbleAudioOutput::HandleClientVolumeOverride(const MumbleUser& user, float volumeOverride)
{
	auto client = m_clients[user.GetSessionId()];

	if (client)
	{
		client->overrideVolume = volumeOverride;
	}
}

void MumbleAudioOutput::HandleClientPosition(const MumbleUser& user, float position[3])
{
	using namespace DirectX;

	auto client = m_clients[user.GetSessionId()];

	if (client)
	{
		auto lastPosition = DirectX::XMFLOAT3(client->position[0], client->position[1], client->position[2]);

		client->position[0] = position[0];
		client->position[1] = position[1];
		client->position[2] = position[2];

		if ((position[0] != 0.0f || position[1] != 0.0f || position[2] != 0.0f))
		{
			float distance = 0.0f;

			if (abs(m_distance) >= 0.01f && abs(client->distance) >= 0.01f)
			{
				distance = std::min(m_distance, client->distance);
			}
			else if (abs(m_distance) >= 0.01f)
			{
				distance = m_distance;
			}
			else if (abs(client->distance) >= 0.01f)
			{
				distance = client->distance;
			}

			// override with the transmitter's range if this is configured like that
			if (g_useSendingRangeOnly->GetValue())
			{
				distance = 0.0f;

				if (abs(client->distance) >= 0.01f)
				{
					distance = client->distance;
				}
			}

			if (g_use3dAudio->GetValue() && m_x3daCalculate && client->overrideVolume < 0.f && distance > 0.0f)
			{
				X3DAUDIO_EMITTER emitter = { 0 };

				if (lastPosition.x != 0.0f || lastPosition.y != 0.0f || lastPosition.z != 0.0f)
				{
					auto curPosition = DirectX::XMFLOAT3(client->position);

					auto v1 = DirectX::XMLoadFloat3(&curPosition);
					auto v2 = DirectX::XMVectorSet(lastPosition.x, lastPosition.y, lastPosition.z, 0.f);

					auto dT = (client->lastTime - timeGetTime());

					if (dT > 0)
					{
						auto eVelocity = (v1 - v2) / (dT / 1000.0f);

						XMFLOAT3 tmp;
						XMStoreFloat3(&tmp, eVelocity);
						emitter.Velocity.x = tmp.x;
						emitter.Velocity.y = tmp.y;
						emitter.Velocity.z = tmp.z;
					}

					client->lastTime = timeGetTime();
				}

				emitter.Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

				float coeffs[2] = { 0 };

				X3DAUDIO_DSP_SETTINGS dsp = { 0 };
				dsp.SrcChannelCount = 1;
				dsp.DstChannelCount = 2;
				dsp.pMatrixCoefficients = coeffs;

				static const X3DAUDIO_DISTANCE_CURVE_POINT _curvePoints[3] = { 0.0f, 1.0f, 0.25f, 1.0f, 1.0f, 0.0f };
				static const X3DAUDIO_DISTANCE_CURVE       _curve = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&_curvePoints[0], 3 };

				emitter.OrientFront = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
				emitter.OrientTop = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
				emitter.Position = DirectX::XMFLOAT3(position);
				emitter.ChannelCount = 1;
				emitter.pVolumeCurve = const_cast<X3DAUDIO_DISTANCE_CURVE*>(&_curve);
				emitter.CurveDistanceScaler = distance;

				emitter.InnerRadius = 2.0f;
				emitter.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;

				emitter.pLFECurve = (X3DAUDIO_DISTANCE_CURVE*)&Emitter_LFE_Curve;
				emitter.pLPFDirectCurve = nullptr; // use default curve
				emitter.pLPFReverbCurve = nullptr; // use default curve
				emitter.pReverbCurve = (X3DAUDIO_DISTANCE_CURVE*)&Emitter_Reverb_Curve;
				emitter.DopplerScaler = 1.0f;

				m_x3daCalculate(m_x3da, &m_listener, &emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER
					| X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB
					| X3DAUDIO_CALCULATE_REVERB, &dsp);

				if (!isnan(dsp.DopplerFactor))
				{
					client->voice->SetFrequencyRatio(dsp.DopplerFactor);
				}

				// reset the volume in case we were in 2d mode
				client->voice->SetVolume(1.0f);

				client->voice->SetOutputMatrix(m_masteringVoice, 1, 2, dsp.pMatrixCoefficients);

				if (m_submixVoice)
				{
					client->voice->SetOutputMatrix(m_submixVoice, 1, 1, &dsp.ReverbLevel);
				}

				//XAUDIO2_FILTER_PARAMETERS FilterParametersDirect = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dsp.LPFDirectCoefficient), 1.0f };
				//client->voice->SetOutputFilterParameters(m_masteringVoice, &FilterParametersDirect);
				//XAUDIO2_FILTER_PARAMETERS FilterParametersReverb = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dsp.LPFReverbCoefficient), 1.0f };
				//client->voice->SetOutputFilterParameters(m_submixVoice, &FilterParametersReverb);

				client->isAudible = (dsp.pMatrixCoefficients[0] > 0.1f || dsp.pMatrixCoefficients[1] > 0.1f);
			}
			else
			{
				auto emitterPos = DirectX::XMVectorSet(position[0], position[1], position[2], 0.0f);
				auto listenerPos = DirectX::XMVectorSet(m_listener.Position.x, m_listener.Position.y, m_listener.Position.z, 0.0f);

				bool shouldHear = (abs(distance) < 0.01f) ? true : (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(emitterPos - listenerPos)) < (distance * distance));

				if (client->overrideVolume >= 0.0f)
				{
					client->voice->SetVolume(client->overrideVolume);

					shouldHear = client->overrideVolume >= 0.005f;
				}
				else
				{
					client->voice->SetVolume(shouldHear ? 1.0f : 0.0f);
				}

				// reset the output matrix in case we were in 3d mode
				float monoAllSpeakers[] = {
					1.0f, 1.0f
				};

				client->voice->SetOutputMatrix(m_masteringVoice, 1, 2, monoAllSpeakers);

				client->isAudible = shouldHear;
			}
		}
		else
		{
			if (g_use3dAudio->GetValue())
			{
				// reset matrix (to inaudible)
				float matrix[2] = { 0.f, 0.f };
				client->voice->SetOutputMatrix(m_masteringVoice, 1, 2, matrix);

				// disable submix voice
				matrix[0] = 0.0f;

				if (m_submixVoice)
				{
					client->voice->SetOutputMatrix(m_submixVoice, 1, 1, matrix);
				}

				// reset frequency ratio
				client->voice->SetFrequencyRatio(1.0f);

				client->isAudible = false;
			}
			else
			{
				// we don't want to hear data-less clients at this time
				client->isAudible = false;
				client->voice->SetVolume(0.0f);
			}
		}
	}
}

void MumbleAudioOutput::SetMatrix(float position[3], float front[3], float up[3])
{
	using namespace DirectX;

	m_listener.pCone = (X3DAUDIO_CONE*)&Listener_DirectionalCone;

	m_listener.OrientFront = DirectX::XMFLOAT3(front);
	m_listener.OrientTop = DirectX::XMFLOAT3(up);

	auto curPosition = DirectX::XMFLOAT3(position);

	if (m_listener.Position.x != 0.0f || m_listener.Position.y != 0.0f || m_listener.Position.z != 0.0f)
	{
		auto dT = (m_lastMatrixTime - timeGetTime());

		if (dT > 0)
		{
			auto v1 = DirectX::XMLoadFloat3(&curPosition);
			auto v2 = DirectX::XMVectorSet(m_lastPosition.x, m_lastPosition.y, m_lastPosition.z, 0.f);

			auto eVelocity = (v1 - v2) / (dT / 1000.0f);

			XMFLOAT3 tmp;
			XMStoreFloat3(&tmp, eVelocity);
			m_listener.Velocity.x = tmp.x;
			m_listener.Velocity.y = tmp.y;
			m_listener.Velocity.z = tmp.z;

			m_lastPosition = DirectX::XMFLOAT3(position);
			m_lastMatrixTime = timeGetTime();
		}
	}

	m_listener.Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

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

	HANDLE mmcssHandle;
	DWORD mmcssTaskIndex;

	mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);

	// initialize COM for the current thread
	CoInitialize(nullptr);

	HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, (void**)m_mmDeviceEnumerator.GetAddressOf());

	if (FAILED(hr))
	{
		trace("%s: failed MMDeviceEnumerator\n", __func__);
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

	// mark as uninitialized
	{
		std::unique_lock<std::mutex> lock(m_initializeMutex);
		m_initialized = false;
	}

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

	if (m_masteringVoice && m_initialized)
	{
		m_masteringVoice->SetVolume(volume);
	}
}

static bool SafeCallX3DA(decltype(&X3DAudioInitialize) func, UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, _Out_writes_bytes_(X3DAUDIO_HANDLE_BYTESIZE) X3DAUDIO_HANDLE Instance)
{
	__try
	{
		func(SpeakerChannelMask, SpeedOfSound, Instance);

		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

WRL::ComPtr<IMMDevice> GetMMDeviceFromGUID(bool input, const std::string& guid);

void DuckingOptOut(WRL::ComPtr<IMMDevice> device);

void MumbleAudioOutput::InitializeAudioDevice()
{
	{
		std::unique_lock<std::mutex> lock(m_initializeMutex);
		m_initialized = false;
	}

	ComPtr<IMMDevice> device;

	if (m_deviceGuid.empty())
	{
		if (FAILED(m_mmDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eCommunications, device.ReleaseAndGetAddressOf())))
		{
			trace("%s: failed GetDefaultAudioEndpoint\n", __func__);
			return;
		}

		// opt out of ducking
		DuckingOptOut(device);
	}
	else
	{
		device = GetMMDeviceFromGUID(false, m_deviceGuid);

		if (!device.Get())
		{
			trace("%s: failed GetMMDeviceFromGUID\n", __func__);
			return;
		}
	}

	auto xa2Dll = LoadLibraryExW(L"XAudio2_8.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	decltype(&CreateAudioReverb) _CreateAudioReverb;

	if (xa2Dll)
	{
		auto _XAudio2Create = (decltype(&XAudio2Create))GetProcAddress(xa2Dll, "XAudio2Create");

		if (FAILED(_XAudio2Create(m_xa2.ReleaseAndGetAddressOf(), 0, 1)))
		{
			trace("%s: failed XA2.8 create\n", __func__);
			return;
		}

		_CreateAudioReverb = (decltype(&CreateAudioReverb))GetProcAddress(xa2Dll, "CreateAudioReverb");
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

		_CreateAudioReverb = [](IUnknown** ppApo) -> HRESULT
		{
			return CoCreateInstance(CLSID_AudioReverb,
				NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)ppApo);
		};
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
		trace("%s: failed CreateMasteringVoice\n", __func__);
		return;
	}

	m_masteringVoice->SetVolume(m_volume);

	if (IsWindows8Point1OrGreater())
	{
		IUnknown* reverbEffect;

		HRESULT hr;

		UINT32 rflags = 0;
		if (FAILED(hr = _CreateAudioReverb(&reverbEffect)))
		{
			return;
		}

		//
		// Create a submix voice
		//

		XAUDIO2_EFFECT_DESCRIPTOR effects[] = { { reverbEffect, TRUE, 1 } };
		XAUDIO2_EFFECT_CHAIN effectChain = { 1, effects };

		if (FAILED(hr = m_xa2->CreateSubmixVoice(&m_submixVoice, 1,
			48000, 0, 0,
			nullptr, &effectChain)))
		{
			return;
		}

		// Set default FX params
		XAUDIO2FX_REVERB_PARAMETERS native;
		XAUDIO2FX_REVERB_I3DL2_PARAMETERS preset = XAUDIO2FX_I3DL2_PRESET_DEFAULT;

		ReverbConvertI3DL2ToNative(&preset, &native);
		m_submixVoice->SetEffectParameters(0, &native, sizeof(native));
	}

	auto x3aDll = (HMODULE)LoadLibraryExW(L"XAudio2_8.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

	if (!x3aDll)
	{
		x3aDll = LoadLibraryExW(L"X3DAudio1_7.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	}

	if (x3aDll && IsWindows8Point1OrGreater())
	{
		auto _X3DAudioInitialize = (decltype(&X3DAudioInitialize))GetProcAddress(x3aDll, "X3DAudioInitialize");
		auto _X3DAudioCalculate = (decltype(&X3DAudioCalculate))GetProcAddress(x3aDll, "X3DAudioCalculate");

		DWORD channelMask = 0;
		m_masteringVoice->GetChannelMask(&channelMask);

		if (_X3DAudioInitialize)
		{
			if (SafeCallX3DA(_X3DAudioInitialize, channelMask, X3DAUDIO_SPEED_OF_SOUND, m_x3da))
			{
				memset(&m_listener, 0, sizeof(m_listener));
				m_listener.Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
				m_listener.OrientFront = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
				m_listener.OrientTop = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

				m_x3daCalculate = _X3DAudioCalculate;
			}
			else
			{
				trace("X3DAudio initialization crashed - wtf?\n");

				m_x3daCalculate = nullptr;
			}
		}
		else
		{
			trace("No X3DAudio found, this is odd.\n");

			m_x3daCalculate = nullptr;
		}
	}
	else
	{
		m_x3daCalculate = nullptr;
	}

	{
		std::unique_lock<std::mutex> lock(m_initializeMutex);
		m_initialized = true;

		m_initializeVar.notify_all();
	}
}
