/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "LabSound/extended/LabSound.h"
#include <libnyquist/Common.h>

#include "MumbleAudioOutput.h"
#include <avrt.h>
#include <sstream>
#include <PacketDataStream.h>
#include <MumbleClientImpl.h>
#include <MumbleClientState.h>
#include <mmsystem.h>
#include <CoreConsole.h>

#include <speex/speex_jitter.h>

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
static std::shared_ptr<ConVar<bool>> g_use2dAudio;
static std::shared_ptr<ConVar<bool>> g_useNativeAudio;
static std::shared_ptr<ConVar<bool>> g_useAudioContext;

void MumbleAudioOutput::Initialize()
{
	g_use3dAudio = std::make_shared<ConVar<bool>>("voice_use3dAudio", ConVar_None, false);
	g_useSendingRangeOnly = std::make_shared<ConVar<bool>>("voice_useSendingRangeOnly", ConVar_None, false);
	g_use2dAudio = std::make_shared<ConVar<bool>>("voice_use2dAudio", ConVar_None, false);
	g_useNativeAudio = std::make_shared<ConVar<bool>>("voice_useNativeAudio", ConVar_None, false);
	g_useAudioContext = std::make_shared<ConVar<bool>>("voice_useAudioContext", ConVar_None, false);

	m_initialized = false;
	m_distance = FLT_MAX;
	m_channelCount = 0;
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

	OnSetMumbleVolume(m_volume);
}

MumbleAudioOutput::BaseAudioState::BaseAudioState()
	: volume(1.0f), overrideVolume(-1.0f)
{
	position[0] = 0.0f;
	position[1] = 0.0f;
	position[2] = 0.0f;
	distance = 0.0f;

	lastTime = timeGetTime();
}

MumbleAudioOutput::BaseAudioState::~BaseAudioState()
{

}

MumbleAudioOutput::ClientAudioStateBase::ClientAudioStateBase()
	: sequence(0), opus(nullptr), isTalking(false)
{
	jitter = jitter_buffer_init(48000 / 100);

	int margin = 2 * (48000 / 100);
	jitter_buffer_ctl(jitter, JITTER_BUFFER_SET_MARGIN, &margin);

	pfBuffer = new float[iBufferSize];

	int error;
	opus = opus_decoder_create(48000, 1, &error);
}

MumbleAudioOutput::ClientAudioStateBase::~ClientAudioStateBase()
{
	if (jitter != nullptr)
	{
		jitter_buffer_destroy(jitter);
		jitter = nullptr;
	}

	delete[] pfBuffer;

	if (opus != nullptr)
	{
		opus_decoder_destroy(opus);
		opus = nullptr;
	}
}

MumbleAudioOutput::ClientAudioState::ClientAudioState()
	: BaseAudioState(), shuttingDown(false), voice(nullptr)
{

}

void MumbleAudioOutput::ClientAudioState::OnBufferEnd(void* cxt)
{
	auto buffer = reinterpret_cast<int16_t*>(cxt);

	if (buffer != nullptr) // Preemptively handle pBufferContext being NULL. Even if never in this case
	{
		_aligned_free(buffer);
	}

	XAUDIO2_VOICE_STATE vs;
	{
		vs.BuffersQueued = 0;

		std::lock_guard _(m_render);
		if (voice != nullptr)
		{
			voice->GetState(&vs);
		}
	}

	if (vs.BuffersQueued == 0)
	{
		if (auto innerState = GetInnerState())
		{
			innerState->isTalking = false;
		}
	}
}

static std::map<lab::AudioContext*, std::weak_ptr<lab::AudioSourceNode>> g_audioToClients;

struct XA2DestinationNode : public lab::AudioDestinationNode
{
	std::thread exitThread;
	std::function<void()> onDtor;

	XA2DestinationNode(lab::AudioContext* context, std::weak_ptr<MumbleAudioOutput::ClientAudioState> state)
		: AudioDestinationNode(context, 1, 48000.0f), m_state(state), m_outBuffer(1, 5760, false), m_shutDown(false), m_inBuffer(nullptr)
	{
		m_outBuffer.setSampleRate(48000.f);
		m_outBuffer.setChannelMemory(0, m_floatBuffer, 5760);
	}

	virtual ~XA2DestinationNode() override
	{
		if (onDtor)
		{
			onDtor();
		}

		if (exitThread.joinable())
		{
			exitThread.join();
		}
	}

	virtual void startRendering() override { }

	void PutThread(std::thread&& thread, std::function<void()>&& onDtor)
	{
		this->exitThread = std::move(thread);
		this->onDtor = std::move(onDtor);
	}

	void Push(lab::AudioBus* inBuffer)
	{
		m_inBuffer = inBuffer;
	}

	void Poll(size_t numFrames)
	{
		__try
		{
			PollReal(numFrames);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		
		}
	}

	void PollReal(size_t numFrames)
	{
		if (m_shutDown)
		{
			return;
		}

		for (int i = 0; i < numFrames; i += lab::AudioNode::ProcessingSizeInFrames)
		{
			lab::AudioBus inBuffer{ 1, lab::AudioNode::ProcessingSizeInFrames, false };
			lab::AudioBus outBuffer{ 1, lab::AudioNode::ProcessingSizeInFrames, false };

			float inFloats[lab::AudioNode::ProcessingSizeInFrames];
			float outFloats[lab::AudioNode::ProcessingSizeInFrames];

			memcpy(inFloats, m_inBuffer->channel(0)->data() + i, lab::AudioNode::ProcessingSizeInFrames * 4);

			inBuffer.setChannelMemory(0, inFloats, lab::AudioNode::ProcessingSizeInFrames);
			outBuffer.setChannelMemory(0, outFloats, lab::AudioNode::ProcessingSizeInFrames);

			if (!m_shutDown)
			{
				render(&inBuffer, &outBuffer, lab::AudioNode::ProcessingSizeInFrames);

				if ((i + lab::AudioNode::ProcessingSizeInFrames) < 5760)
				{
					memcpy(m_outBuffer.channel(0)->mutableData() + i, outFloats, lab::AudioNode::ProcessingSizeInFrames * 4);
				}
			}
		}

		auto state = m_state.lock();

		if (!state || m_shutDown)
		{
			return;
		}

		// work around XA2.7 issue (for Win7) where >64 buffers being enqueued are a fatal error (leading to __debugbreak)
		// "SimpList: non-growable list ran out of room for new elements"

		// @FIX(fillet-zulu-mississippi)
		std::lock_guard _(state->m_render);
		if (state->voice == nullptr)
		{
			return;
		}

		XAUDIO2_VOICE_STATE vs;
		state->voice->GetState(&vs);

		if (vs.BuffersQueued > 48)
		{
			// return, waiting for buffers to play back
			// flushing buffers would be helpful, but would lead to memory leaks
			// (and wouldn't be instant, either)

			return;
		}

		uint16_t* voiceBuffer = (uint16_t*)_aligned_malloc(5760 * sizeof(uint16_t), 16);
		nqr::ConvertFromFloat32((uint8_t*)voiceBuffer, m_floatBuffer, numFrames, nqr::PCM_16);

		state->PushSoundInternal(voiceBuffer, numFrames);
	}

	virtual void uninitialize() override
	{
		lab::AudioDestinationNode::uninitialize();

		m_shutDown = true;
	}

	std::weak_ptr<MumbleAudioOutput::ClientAudioState> m_state;
	lab::AudioBus* m_inBuffer;
	lab::AudioBus m_outBuffer;

	float m_floatBuffer[5760];
	bool m_shutDown;
};

void MumbleAudioOutput::ClientAudioState::PushSoundInternal(uint16_t* voiceBuffer, int numFrames)
{
	if (!voice)
	{
		return;
	}

	XAUDIO2_BUFFER bufferData;
	bufferData.LoopBegin = 0;
	bufferData.LoopCount = 0;
	bufferData.LoopLength = 0;
	bufferData.AudioBytes = numFrames * sizeof(int16_t);
	bufferData.Flags = 0;
	bufferData.pAudioData = reinterpret_cast<BYTE*>(voiceBuffer);
	bufferData.pContext = voiceBuffer;
	bufferData.PlayBegin = 0;
	bufferData.PlayLength = numFrames;

	voice->SubmitSourceBuffer(&bufferData);
}

MumbleAudioOutput::ClientAudioState::~ClientAudioState()
{
	// Hopefully @FIX(pasta-pizza-purple): the voice object exists (rcx).
	// However, its vtable "mov rax, [rcx]" is 0x0.
	//
	// This will likely do nothing for friend-two-oranges, as that issue seems
	// correlated to offloading audio context deconstruction duties onto another
	// thread
	std::lock_guard _(m_render);
	lab::AudioContext* contextRef = context.get();

	if (contextRef)
	{
		shuttingDown = true;

		// we shall not use a shared_ptr inside of the thread as it'll then try to join itself
		// which is nonsense
		auto cdg = contextRef->destination().get();

		std::static_pointer_cast<XA2DestinationNode>(context->destination())->PutThread(std::thread([this, contextRef, cdg]()
		{
			static float floatBuffer[24000] = { 0 };

			lab::AudioBus inBuffer{ 1, 24000, false };
			inBuffer.setSampleRate(48000.f);
			inBuffer.setChannelMemory(0, floatBuffer, 24000);

			while (shuttingDown)
			{
				auto context = contextRef;

				if (context)
				{
					auto d = static_cast<XA2DestinationNode*>(cdg);

					if (d)
					{
						d->Push(&inBuffer);
						d->Poll(24000);
					}
				}
			}
		}), [this]()
		{
			shuttingDown = false;
		});
	}

	if (auto ctx = std::move(context))
	{
		// destroy the lab::AudioContext off-thread as it may be blocking for a while
		struct DtorWorker
		{
			std::shared_ptr<lab::AudioContext> audCxt;
		};

		auto dtorWorker = new DtorWorker();
		dtorWorker->audCxt = ctx;

		QueueUserWorkItem([](void* data) -> DWORD
		{
			auto dtorWorker = (DtorWorker*)data;
			delete dtorWorker;

			return 0;
		},
		dtorWorker, 0);
	}

	if (voice)
	{
		voice->DestroyVoice();
		voice = nullptr;
	}
}

DLL_EXPORT
fwEvent<const std::wstring&, fwRefContainer<IMumbleAudioSink>*>
OnGetMumbleAudioSink;

DLL_EXPORT
fwEvent<float>
OnSetMumbleVolume;

MumbleAudioOutput::ExternalAudioState::ExternalAudioState(fwRefContainer<IMumbleAudioSink> sink)
	: BaseAudioState(), sink(sink)
{
}

MumbleAudioOutput::ExternalAudioState::~ExternalAudioState()
{
	
}

void MumbleAudioOutput::ExternalAudioState::AfterConstruct()
{
	std::weak_ptr thisWeak = shared_from_this();

	sink->SetResetHandler([thisWeak]()
	{
		auto selfBase = thisWeak.lock();

		if (selfBase)
		{
			auto self = std::static_pointer_cast<ExternalAudioState>(selfBase);
			self->SetInnerState({});
		}
	});

	sink->SetPollHandler([thisWeak](int numSamples)
	{
		auto selfBase = thisWeak.lock();

		if (selfBase)
		{
			auto self = std::static_pointer_cast<ExternalAudioState>(selfBase);

			auto is = self->GetInnerState();

			if (is)
			{
				if (!is->PollAudio(numSamples, self.get()))
				{
					self->SetInnerState({});
				}
			}
		}
	});
}

void MumbleAudioOutput::ExternalAudioState::PushPosition(MumbleAudioOutput* baseIo, float position[3])
{
	float distance = 0.0f;
	auto client = this;

	if (abs(baseIo->m_distance) >= 0.01f && abs(client->distance) >= 0.01f)
	{
		distance = std::min(baseIo->m_distance, client->distance);
	}
	else if (abs(baseIo->m_distance) >= 0.01f)
	{
		distance = baseIo->m_distance;
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

	using namespace DirectX;

	auto emitterPos = XMVectorSet(position[0], position[1], position[2], 0.0f);
	auto listenerPos = XMVectorSet(baseIo->m_listener.Position.x, baseIo->m_listener.Position.y, baseIo->m_listener.Position.z, 0.0f);

	// default algorithm: (abs(distance) < 0.01f) ? true : (XMVectorGetX(XMVector3LengthSq(emitterPos - listenerPos)) < (distance * distance))
	bool shouldHear = sink->IsTalkingAt(XMVectorGetX(XMVector3Length(emitterPos - listenerPos)));

	if (client->overrideVolume >= 0.0f)
	{
		shouldHear = client->overrideVolume >= 0.005f;
	}

	client->isAudible = shouldHear;

	sink->SetPosition(position, distance, client->overrideVolume);
}

void MumbleAudioOutput::ExternalAudioState::PushSound(int16_t* voiceBuffer, int len)
{
	// 48kHz = 48 samples/msec, 30ms to account for ticking anomaly
	lastPush = timeGetTime() + (len / 48) + 30;

	sink->PushAudio(voiceBuffer, len);
}

bool MumbleAudioOutput::ExternalAudioState::Valid()
{
	return sink.GetRef();
}

bool MumbleAudioOutput::ExternalAudioState::IsTalking()
{
	if (auto is = GetInnerState())
	{
		if (is->isTalking)
		{
			if (timeGetTime() >= lastPush)
			{
				is->isTalking = false;
			}
		}

		return is->isTalking;
	}

	return false;
}

void MumbleAudioOutput::HandleClientConnect(const MumbleUser& user)
{
	{
		std::unique_lock<std::mutex> initLock(m_initializeMutex);

		if (!m_initialized && !m_initializeSignaled)
		{
			m_initializeVar.wait(initLock);
		}
	}

	// if still not initialized, exit out
	if (!m_initialized)
	{
		return;
	}

	if (g_useNativeAudio->GetValue())
	{
		fwRefContainer<IMumbleAudioSink> sinkRef;
		OnGetMumbleAudioSink(user.GetName(), &sinkRef);

		if (sinkRef.GetRef())
		{
			auto state = std::make_shared<ExternalAudioState>(sinkRef);
			state->AfterConstruct();

			std::unique_lock<std::shared_mutex> _(m_clientsMutex);
			m_clients[user.GetSessionId()] = state;

			return;
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
	state->AfterConstruct();

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

	if (g_useAudioContext->GetValue())
	{
		auto context = std::make_shared<lab::AudioContext>(false, true);

		{
			lab::ContextRenderLock r(context.get(), "init");
			auto outNode = std::make_shared<XA2DestinationNode>(context.get(), state);
			context->setDestinationNode(outNode);

			state->inNode = lab::Sound::MakeHardwareSourceNode(r);
		}

		g_audioToClients[context.get()] = state->inNode;

		context->lazyInitialize();

		static auto tNode = std::make_shared<lab::BiquadFilterNode>();
		context->connect(context->destination(), state->inNode);

		context->startRendering();

		state->context = context;
	}

	std::unique_lock<std::shared_mutex> _(m_clientsMutex);
	m_clients[user.GetSessionId()] = state;
}

void MumbleAudioOutput::HandleClientVoiceData(const MumbleUser& user, uint64_t sequence, const uint8_t* data, size_t size, bool hasTerminator)
{
	std::shared_ptr<BaseAudioState> client;

	{
		std::shared_lock<std::shared_mutex> _(m_clientsMutex);
		auto it = m_clients.find(user.GetSessionId());

		if (it != m_clients.end())
		{
			client = it->second;
		}
	}

	if (!client || !client->Valid())
	{
		return;
	}

	client->HandleVoiceData(sequence, data, size, hasTerminator);
}

void MumbleAudioOutput::BaseAudioState::HandleVoiceData(uint64_t sequence, const uint8_t* data, size_t size, bool hasTerminator)
{
	auto is = GetInnerState();

	if (!is)
	{
		is = std::make_shared<ClientAudioStateBase>();
		SetInnerState(is);
	}

	is->sequence = sequence;
	is->bHasTerminator = hasTerminator;

	int numSamples = opus_decoder_get_nb_samples(is->opus, data, size);

	JitterBufferPacket jbp;
	jbp.data = const_cast<char*>(reinterpret_cast<const char*>(data));
	jbp.len = size;
	jbp.span = numSamples;
	jbp.timestamp = (48000 / 100) * sequence;

	{
		std::unique_lock _(is->jitterLock);
		jitter_buffer_put(is->jitter, &jbp);
	}

	if (ShouldManagePoll())
	{
		if (!is->PollAudio(numSamples, this))
		{
			SetInnerState({});
		}
	}
}

void MumbleAudioOutput::ClientAudioStateBase::resizeBuffer(size_t newsize)
{
	if (newsize > iBufferSize)
	{
		float* n = new float[newsize];
		if (pfBuffer)
		{
			memcpy(n, pfBuffer, sizeof(float) * iBufferSize);
			delete[] pfBuffer;
		}
		pfBuffer = n;
		iBufferSize = newsize;
	}
}

bool MumbleAudioOutput::ClientAudioStateBase::PollAudio(int frameCount, BaseAudioState* root)
{
	if (sequence == 0)
	{
		return true;
	}

	// adapted from Mumble reference code: https://github.com/mumble-voip/mumble/blob/3beb90245cf00f72de217a2819dc7bd1d564f5f9/src/mumble/AudioOutputSpeech.cpp#L219
	// (c) 2011-2021 The Mumble Developers. 

	unsigned int channels = 1;
	// Note: all stereo supports are crafted for opus, since other codecs are deprecated and will soon be removed.

	unsigned int sampleCount = frameCount * channels;

	if (iBufferFilled < 0 || iBufferFilled >= iBufferSize)
	{
		iBufferFilled = 0;
	}

	// we can not control exactly how many frames decoder returns
	// so we need a buffer to keep unused frames
	// shift the buffer, remove decoded and played frames
	for (unsigned int i = iLastConsume; i < iBufferFilled; ++i)
		pfBuffer[i - iLastConsume] = pfBuffer[i];

	iBufferFilled -= iLastConsume;

	iLastConsume = sampleCount;

	auto consume = [this, root]()
	{
		std::vector<int16_t> s16(iLastConsume);
		for (size_t i = 0; i < iLastConsume; i++)
		{
			s16[i] = int16_t(std::clamp(pfBuffer[i] * 32768, -32768.f, 32767.f));
		}

		root->PushSound(s16.data(), iLastConsume);
		isTalking = !quiet && root->isAudible;
	};

	if (iBufferFilled >= sampleCount)
	{
		consume();
		return bLastAlive;
	}

	float* pOut;
	bool nextalive = bLastAlive;

	// 20ms
	int iOutputSize = 100 * 48;
	int iFrameSize = 48000 / 100; // 10ms

	while (iBufferFilled < sampleCount)
	{
		std::unique_lock _(jitterLock);

		int decodedSamples = iFrameSize;
		resizeBuffer(iBufferFilled + iOutputSize);
		// TODO: allocating memory in the audio callback will crash mumble in some cases.
		//       we need to initialize the buffer with an appropriate size when initializing
		//       this class. See #4250.

		pOut = (pfBuffer + iBufferFilled);

		if (!bLastAlive)
		{
			memset(pOut, 0, iFrameSize * sizeof(float));
		}
		else
		{
			int avail = 0;
			int ts = jitter_buffer_get_pointer_timestamp(jitter);
			jitter_buffer_ctl(jitter, JITTER_BUFFER_GET_AVAILABLE_COUNT, &avail);

			if ((ts == 0))
			{
				int want = static_cast<int>(fAverageAvailable);
				if (avail < want)
				{
					++iMissCount;
					if (iMissCount < 20)
					{
						memset(pOut, 0, iFrameSize * sizeof(float));
						goto nextframe;
					}
				}
			}

			if (qlFrames.empty())
			{
				char data[4096];
				JitterBufferPacket jbp;
				jbp.data = data;
				jbp.len = 4096;

				spx_int32_t startofs = 0;

				if (jitter_buffer_get(jitter, &jbp, iFrameSize, &startofs) == JITTER_BUFFER_OK)
				{
					iMissCount = 0;

					qlFrames.push_back(std::make_unique<std::vector<uint8_t>>((uint8_t*)jbp.data, (uint8_t*)jbp.data + jbp.len));

					{
						float a = static_cast<float>(avail);
						if (avail >= fAverageAvailable)
							fAverageAvailable = a;
						else
							fAverageAvailable *= 0.99f;
					}
				}
				else
				{
					// Let the jitter buffer know it's the right time to adjust the buffering delay to the network
					// conditions.
					jitter_buffer_update_delay(jitter, &jbp, nullptr);

					iMissCount++;
					if (iMissCount > 10)
						nextalive = false;
				}
			}

			if (!qlFrames.empty())
			{
				auto array = std::move(qlFrames.front());
				qlFrames.pop_front();

				decodedSamples = opus_decode_float(opus, !array->empty() ? array->data() : nullptr, array->size(), pOut, 48 * 60, 0);

				if (decodedSamples < 0)
				{
					decodedSamples = iFrameSize;
					memset(pOut, 0, iFrameSize * sizeof(float));
				}

				bool update = true;

				{
					float& fPowerMax = this->fPowerMax;
					float& fPowerMin = this->fPowerMin;

					float pow = 0.0f;
					for (int i = 0; i < decodedSamples; ++i)
						pow += pOut[i] * pOut[i];
					pow = sqrtf(pow / static_cast<float>(decodedSamples)); // Average over both L and R channel.

					if (pow >= fPowerMax)
					{
						fPowerMax = pow;
					}
					else
					{
						if (pow <= fPowerMin)
						{
							fPowerMin = pow;
						}
						else
						{
							fPowerMax = 0.99f * fPowerMax;
							fPowerMin += 0.0001f * pow;
						}
					}

					update = (pow < (fPowerMin + 0.01f * (fPowerMax - fPowerMin))); // Update jitter buffer when quiet.
				}

				quiet = update;

				// qlFrames.isEmpty() will always be true if using opus.
				// Q_ASSERT(qlFrames.isEmpty());
				if (qlFrames.empty() && update)
					jitter_buffer_update_delay(jitter, nullptr, nullptr);

				// lastPush check is an extra since we don't always send terminators
				if (qlFrames.empty() && (bHasTerminator || (timeGetTime() >= root->lastPush)))
					nextalive = false;

				iInterpCount = 0;
			}
			else
			{
				// note this will interpolate since it's meant for packet loss - no data does *not* mean no audio
				if (opus)
				{
					decodedSamples = opus_decode_float(opus, nullptr, 0, pOut, iFrameSize, 0);
					decodedSamples *= channels;
				}

				if (decodedSamples < 0)
				{
					decodedSamples = iFrameSize;
					memset(pOut, 0, iFrameSize * sizeof(float));
				}

				// mitigate interpolation (we don't currently have terminators)
				iInterpCount++;
				if (iInterpCount > 10)
				{
					memset(pOut, 0, decodedSamples * sizeof(float));
					quiet = true;
				}
			}

/*
			if (!nextalive)
			{
				for (unsigned int i = 0; i < static_cast<unsigned int>(iFrameSizePerChannel); ++i)
				{
					for (unsigned int s = 0; s < channels; ++s)
						pOut[i * channels + s] *= fFadeOut[i];
				}
			}
			else if (ts == 0)
			{
				for (unsigned int i = 0; i < static_cast<unsigned int>(iFrameSizePerChannel); ++i)
				{
					for (unsigned int s = 0; s < channels; ++s)
						pOut[i * channels + s] *= fFadeIn[i];
				}
			}
*/

			for (int i = decodedSamples / iFrameSize; i > 0; --i)
			{
				jitter_buffer_tick(jitter);
			}
		}
	nextframe:
		// local mute logic

		spx_uint32_t inlen = decodedSamples / channels; // per channel
		spx_uint32_t outlen = inlen;
		iBufferFilled += outlen * channels;
	}

	bool tmp = bLastAlive;
	this->bLastAlive = nextalive;

	consume();

	return tmp;
}

void MumbleAudioOutput::ClientAudioState::PushSound(int16_t* voiceBuffer, int len)
{
	// 48kHz = 48 samples/msec, 30ms to account for ticking anomaly
	lastPush = timeGetTime() + (len / 48) + 30;

	if (!context)
	{
		auto voiceBufferCopy = (uint16_t*)_aligned_malloc(len * sizeof(uint16_t), 16);
		memcpy(voiceBufferCopy, voiceBuffer, len * sizeof(uint16_t));

		PushSoundInternal(voiceBufferCopy, len);
	}
	else
	{
		auto floatBuffer = (float*)_aligned_malloc(len * 1 * sizeof(float), 16);
		nqr::ConvertToFloat32(floatBuffer, voiceBuffer, len, nqr::PCM_16);

		{
			lab::AudioBus inBuffer{ 1, size_t(len), false };
			inBuffer.setSampleRate(48000.f);
			inBuffer.setChannelMemory(0, floatBuffer, len);

			std::static_pointer_cast<XA2DestinationNode>(context->destination())->Push(&inBuffer);
			std::static_pointer_cast<XA2DestinationNode>(context->destination())->Poll(len);
		}

		_aligned_free(floatBuffer);
	}
}

static const X3DAUDIO_DISTANCE_CURVE_POINT Emitter_LFE_CurvePoints[3] = { 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f };
static const X3DAUDIO_DISTANCE_CURVE       Emitter_LFE_Curve = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_LFE_CurvePoints[0], 3 };

static const X3DAUDIO_DISTANCE_CURVE_POINT Emitter_Reverb_CurvePoints[3] = { 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, 0.0f };
static const X3DAUDIO_DISTANCE_CURVE       Emitter_Reverb_Curve = { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_Reverb_CurvePoints[0], 3 };

static const X3DAUDIO_CONE Listener_DirectionalCone = { X3DAUDIO_PI*5.0f / 6.0f, X3DAUDIO_PI*11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };

void MumbleAudioOutput::HandleClientDistance(const MumbleUser& user, float distance)
{
	std::shared_ptr<BaseAudioState> client;

	{
		std::shared_lock<std::shared_mutex> _(m_clientsMutex);
		auto it = m_clients.find(user.GetSessionId());

		if (it != m_clients.end())
		{
			client = it->second;
		}
	}

	if (client)
	{
		client->distance = distance;
	}
}

void MumbleAudioOutput::HandleClientVolumeOverride(const MumbleUser& user, float volumeOverride)
{
	std::shared_ptr<BaseAudioState> client;

	{
		std::shared_lock<std::shared_mutex> _(m_clientsMutex);
		auto it = m_clients.find(user.GetSessionId());

		if (it != m_clients.end())
		{
			client = it->second;
		}
	}

	if (client)
	{
		client->overrideVolume = volumeOverride;
	}
}

void MumbleAudioOutput::HandleClientPosition(const MumbleUser& user, float position[3])
{
	std::shared_ptr<BaseAudioState> client;

	{
		std::shared_lock<std::shared_mutex> _(m_clientsMutex);
		auto it = m_clients.find(user.GetSessionId());

		if (it != m_clients.end())
		{
			client = it->second;
		}
	}

	if (client)
	{
		client->PushPosition(this, position);
	}
}

void MumbleAudioOutput::ClientAudioState::PushPosition(MumbleAudioOutput* baseIo, float position[3])
{
	using namespace DirectX;

	std::lock_guard _(m_render);
	auto client = this;

	if (voice)
	{
		auto lastPosition = DirectX::XMFLOAT3(client->position[0], client->position[1], client->position[2]);

		client->position[0] = position[0];
		client->position[1] = position[1];
		client->position[2] = position[2];

		if ((position[0] != 0.0f || position[1] != 0.0f || position[2] != 0.0f))
		{
			float distance = 0.0f;

			if (abs(baseIo->m_distance) >= 0.01f && abs(client->distance) >= 0.01f)
			{
				distance = std::min(baseIo->m_distance, client->distance);
			}
			else if (abs(baseIo->m_distance) >= 0.01f)
			{
				distance = baseIo->m_distance;
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

			if (g_use3dAudio->GetValue() && baseIo->m_x3daCalculate && client->overrideVolume < 0.f && distance > 0.0f)
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

				float coeffs[16] = { 0 };

				X3DAUDIO_DSP_SETTINGS dsp = { 0 };
				dsp.SrcChannelCount = 1;
				dsp.DstChannelCount = baseIo->m_channelCount;
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

				baseIo->m_x3daCalculate(baseIo->m_x3da, &baseIo->m_listener, &emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER
					| X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_LPF_REVERB
					| X3DAUDIO_CALCULATE_REVERB, &dsp);

				if (!isnan(dsp.DopplerFactor))
				{
					client->voice->SetFrequencyRatio(dsp.DopplerFactor);
				}

				// reset the volume in case we were in 2d mode
				client->voice->SetVolume(1.0f);

				client->voice->SetOutputMatrix(baseIo->m_masteringVoice, 1, dsp.DstChannelCount, dsp.pMatrixCoefficients);

				if (baseIo->m_submixVoice)
				{
					client->voice->SetOutputMatrix(baseIo->m_submixVoice, 1, 1, &dsp.ReverbLevel);
				}

				//XAUDIO2_FILTER_PARAMETERS FilterParametersDirect = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dsp.LPFDirectCoefficient), 1.0f };
				//client->voice->SetOutputFilterParameters(m_masteringVoice, &FilterParametersDirect);
				//XAUDIO2_FILTER_PARAMETERS FilterParametersReverb = { LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * dsp.LPFReverbCoefficient), 1.0f };
				//client->voice->SetOutputFilterParameters(m_submixVoice, &FilterParametersReverb);

				client->isAudible = (dsp.pMatrixCoefficients[0] > 0.1f || dsp.pMatrixCoefficients[1] > 0.1f);
			}
			else if (g_use2dAudio->GetValue() && client->overrideVolume < 0.f && distance > 0.0f)
			{
				auto emitterPos = DirectX::XMVectorSet(position[0], position[1], position[2], 0.0f);
				auto listenerPos = DirectX::XMVectorSet(baseIo->m_listener.Position.x, baseIo->m_listener.Position.y, baseIo->m_listener.Position.z, 0.0f);

				float distanceFromListener = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(emitterPos - listenerPos));
				bool shouldHear = (abs(distance) < 0.01f) ? true : (distanceFromListener < (distance * distance));

				if (shouldHear)
				{
					float volume = distanceFromListener / (distance * distance);
					client->voice->SetVolume(std::max(0.0f, std::min(1.0f, 1.0f - volume)));
				}
				else
				{
					client->voice->SetVolume(0.0f);
				}

				// reset the output matrix in case we were in 3d mode
				float monoAllSpeakers[] = {
					1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
					1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				};

				client->voice->SetOutputMatrix(baseIo->m_masteringVoice, 1, baseIo->m_channelCount, monoAllSpeakers);

				client->isAudible = shouldHear;
			}
			else
			{
				auto emitterPos = DirectX::XMVectorSet(position[0], position[1], position[2], 0.0f);
				auto listenerPos = DirectX::XMVectorSet(baseIo->m_listener.Position.x, baseIo->m_listener.Position.y, baseIo->m_listener.Position.z, 0.0f);

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
					1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 
					1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				};

				client->voice->SetOutputMatrix(baseIo->m_masteringVoice, 1, baseIo->m_channelCount, monoAllSpeakers);

				client->isAudible = shouldHear;
			}
		}
		else
		{
			if (g_use3dAudio->GetValue())
			{
				// reset matrix (to inaudible)
				float matrix[] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, };
				client->voice->SetOutputMatrix(baseIo->m_masteringVoice, 1, baseIo->m_channelCount, matrix);

				// disable submix voice
				matrix[0] = 0.0f;

				if (baseIo->m_submixVoice)
				{
					client->voice->SetOutputMatrix(baseIo->m_submixVoice, 1, 1, matrix);
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
	std::shared_ptr<BaseAudioState> ptr;

	{
		std::unique_lock<std::shared_mutex> _(m_clientsMutex);
		std::swap(ptr, m_clients[user.GetSessionId()]);
	}
}

void MumbleAudioOutput::GetTalkers(std::vector<uint32_t>* talkers)
{
	talkers->clear();

	std::shared_lock<std::shared_mutex> _(m_clientsMutex);

	for (auto& client : m_clients)
	{
		if (!client.second)
		{
			continue;
		}

		if (client.second->IsTalking())
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

	{
		std::unique_lock<std::shared_mutex> _(m_clientsMutex);

		for (auto& client : m_clients)
		{
			if (client.second)
			{
				m_ids.push_back(client.first);
			}
		}

		// delete all clients
		m_clients.clear();
	}

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

float MumbleAudioOutput::GetDistance()
{
	return m_distance;
}

void MumbleAudioOutput::SetVolume(float volume)
{
	m_volume = volume;
	OnSetMumbleVolume(volume);

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

std::shared_ptr<lab::AudioContext> MumbleAudioOutput::GetAudioContext(const std::string& name)
{
	auto wideName = ToWide(name);

	std::shared_lock<std::shared_mutex> _(m_clientsMutex);

	for (auto& client : m_clients)
	{
		if (!client.second)
		{
			continue;
		}

		auto user = m_client->GetState().GetUser(client.first);

		if (!user || user->GetName() != wideName)
		{
			continue;
		}

		return std::static_pointer_cast<ClientAudioState>(client.second)->context;
	}

	return {};
}

extern std::mutex g_mmDeviceMutex;

void MumbleAudioOutput::InitializeAudioDevice()
{
	{
		std::unique_lock<std::mutex> lock(m_initializeMutex);
		m_initialized = false;
	}

	ComPtr<IMMDevice> device;

	// ensure the initialize variable is signaled no matter what
	struct Unlocker
	{
		Unlocker(MumbleAudioOutput* self)
			: self(self)
		{
		
		}

		~Unlocker()
		{
			std::unique_lock<std::mutex> lock(self->m_initializeMutex);
			self->m_initializeVar.notify_all();
			self->m_initializeSignaled = true;
		}

		MumbleAudioOutput* self;
	} unlocker(this);

	// this initialization *needs* to be locked as otherwise it'll expose a race condition in Steam's 'gameoverlayrenderer64.dll' leading to a
	// stack overflow with the built-in OS mic monitoring shim in apphelp.dll and Steam's hook calling itself recursively
	{
		std::unique_lock _(g_mmDeviceMutex);

		if (!m_mmDeviceEnumerator)
		{
			HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator, (void**)m_mmDeviceEnumerator.GetAddressOf());

			if (FAILED(hr))
			{
				trace("%s: failed MMDeviceEnumerator (%08x)\n", __func__, hr);
				return;
			}
		}

		while (!device.Get())
		{
			if (m_deviceGuid.empty())
			{
				if (FAILED(m_mmDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eCommunications, device.ReleaseAndGetAddressOf())))
				{
					trace("%s: failed GetDefaultAudioEndpoint\n", __func__);
					return;
				}
			}
			else
			{
				device = GetMMDeviceFromGUID(false, m_deviceGuid);

				if (!device.Get())
				{
					trace("%s: failed GetMMDeviceFromGUID\n", __func__);
					m_deviceGuid = "";
				}
			}
		}

		// opt out of ducking
		DuckingOptOut(device);
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

	if (FAILED(m_xa2->CreateMasteringVoice(&m_masteringVoice, 0, 48000, 0, deviceIdStr.c_str())))
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

		// some devices (like Sony 'Wireless Controller') return 0 channels
		// assume these will be stereo
		if (channelMask == 0)
		{
			channelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
		}

		m_channelCount = 0;

		for (int i = 0; i < 32; i++)
		{
			if (channelMask & (1 << i))
			{
				m_channelCount++;
			}
		}

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
		m_channelCount = 2;
		m_x3daCalculate = nullptr;
	}

	{
		std::unique_lock<std::mutex> lock(m_initializeMutex);
		m_initialized = true;
	}
}

// temp scrbind
#include <scrBind.h>

template<>
struct scrBindArgument<lab::FilterType>
{
	static lab::FilterType Get(fx::ScriptContext& cxt, int i)
	{
		std::string_view a = cxt.CheckArgument<const char*>(i);

		if (a == "lowpass")
		{
			return lab::LOWPASS;
		}
		else if (a == "highpass")
		{
			return lab::HIGHPASS;
		}
		else if (a == "bandpass")
		{
			return lab::BANDPASS;
		}
		else if (a == "lowshelf")
		{
			return lab::LOWSHELF;
		}
		else if (a == "highshelf")
		{
			return lab::HIGHSHELF;
		}
		else if (a == "peaking")
		{
			return lab::PEAKING;
		}
		else if (a == "notch")
		{
			return lab::NOTCH;
		}
		else if (a == "allpass")
		{
			return lab::ALLPASS;
		}

		return lab::FILTER_NONE;
	}
};

template<>
struct scrBindArgument<lab::NoiseNode::NoiseType>
{
	static lab::NoiseNode::NoiseType Get(fx::ScriptContext& cxt, int i)
	{
		std::string_view a = cxt.CheckArgument<const char*>(i);

		if (a == "pink")
		{
			return lab::NoiseNode::NoiseType::PINK;
		}
		else if (a == "brown")
		{
			return lab::NoiseNode::NoiseType::BROWN;
		}
		else if (a == "white")
		{
			return lab::NoiseNode::NoiseType::WHITE;
		}

		return lab::NoiseNode::NoiseType::PINK;;
	}
};

static std::shared_ptr<lab::BiquadFilterNode> createBiquadFilterNode(lab::AudioContext* self)
{
	return std::make_shared<lab::BiquadFilterNode>();
}

static std::shared_ptr<lab::WaveShaperNode> createWaveShaperNode(lab::AudioContext* self)
{
	return std::make_shared<lab::WaveShaperNode>();
}

static std::shared_ptr<lab::StereoPannerNode> createStereoPannerNode(lab::AudioContext* self)
{
	return std::make_shared<lab::StereoPannerNode>();
}

static std::shared_ptr<lab::NoiseNode> createNoiseNode(lab::AudioContext* self)
{
	return std::make_shared<lab::NoiseNode>();
}

static std::shared_ptr<lab::PeakCompNode> createPeakCompNode(lab::AudioContext* self)
{
	return std::make_shared<lab::PeakCompNode>();
}

static std::shared_ptr<lab::AudioSourceNode> getSource(lab::AudioContext* self)
{
	return g_audioToClients[self].lock();
}

static InitFunction initFunctionScript([]()
{
	scrBindClass<std::shared_ptr<lab::AudioContext>>()
		.AddMethod("AUDIOCONTEXT_CONNECT", &lab::AudioContext::connect)
		.AddMethod("AUDIOCONTEXT_DISCONNECT", &lab::AudioContext::disconnect)
		.AddMethod("AUDIOCONTEXT_GET_CURRENT_TIME", &lab::AudioContext::currentTime)
		.AddMethod("AUDIOCONTEXT_GET_SOURCE", &getSource)
		.AddMethod("AUDIOCONTEXT_GET_DESTINATION", &lab::AudioContext::destination)
		.AddMethod("AUDIOCONTEXT_CREATE_BIQUADFILTERNODE", &createBiquadFilterNode)
		.AddMethod("AUDIOCONTEXT_CREATE_WAVESHAPERNODE", &createWaveShaperNode)
		.AddMethod("AUDIOCONTEXT_CREATE_STEREOPANNERNODE", &createStereoPannerNode)
		.AddMethod("AUDIOCONTEXT_CREATE_NOISENODE", &createNoiseNode)
		.AddMethod("AUDIOCONTEXT_CREATE_PEAKCOMPNODE", &createPeakCompNode);

	scrBindClass<std::shared_ptr<lab::AudioNode>>()
		.AddMethod("AUDIOCONTEXT_GET_NODE_PARAM", &lab::AudioNode::getParam);
	
	scrBindClass<std::shared_ptr<lab::BiquadFilterNode>>()
		.AddMethod("BIQUADFILTERNODE_SET_TYPE", &lab::BiquadFilterNode::setType)
		.AddMethod("BIQUADFILTERNODE_Q", &lab::BiquadFilterNode::q)
		// needs msgpack wrappers for in/out and a weird lock thing
		//.AddMethod("BIQUAFTILERNODE_GET_FREQUENCY_RESPONSE", &lab::BiquadFilterNode::getFrequencyResponse)
		.AddMethod("BIQUADFILTERNODE_GAIN", &lab::BiquadFilterNode::gain)
		.AddMethod("BIQUADFILTERNODE_FREQUENCY", &lab::BiquadFilterNode::frequency)
		.AddMethod("BIQUADFILTERNODE_DETUNE", &lab::BiquadFilterNode::detune)
		.AddDestructor("BIQUADFILTERNODE_DESTROY");

	scrBindClass<std::shared_ptr<lab::WaveShaperNode>>()
		// this is if 0'd?
		//.AddMethod("WAVESHAPERNODE_GET_CURVE", &lab::WaveShaperNode::curve)
		.AddMethod("WAVESHAPERNODE_SET_CURVE", &lab::WaveShaperNode::setCurve)
		.AddDestructor("WAVESHAPERNODE_DESTROY");

	scrBindClass<std::shared_ptr<lab::StereoPannerNode>>()
		.AddDestructor("STEREOPANNERNODE_DESTROY");

	scrBindClass<std::shared_ptr<lab::PeakCompNode>>()
		.AddDestructor("PEAKCOMPNODE_DESTROY");

	scrBindClass<std::shared_ptr<lab::NoiseNode>>()
		.AddMethod("NOISENODE_SET_TYPE", &lab::NoiseNode::setType)
		.AddDestructor("NOISENODE_DESTROY");

	scrBindClass<std::shared_ptr<lab::AudioParam>>()
		.AddMethod("AUDIOPARAM_SET_VALUE", &lab::AudioParam::setValue)
		.AddMethod("AUDIOPARAM_SET_VALUE_AT_TIME", &lab::AudioParam::setValueAtTime)
		.AddMethod("AUDIOPARAM_SET_VALUE_CURVE_AT_TIME", &lab::AudioParam::setValueCurveAtTime)
		.AddDestructor("AUDIOPARAM_DESTROY");
});
