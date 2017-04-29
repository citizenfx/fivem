#include <StdInc.h>

#include <Hooking.h>

#include <xaudio2.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

#define DECLSPEC_UUID_WRAPPER(x) __declspec(uuid(#x))

#define DEFINE_IID(interfaceName, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
            interface DECLSPEC_UUID_WRAPPER(l##-##w1##-##w2##-##b1##b2##-##b3##b4##b5##b6##b7##b8) interfaceName; \
            EXTERN_C const GUID DECLSPEC_SELECTANY IID_##interfaceName = __uuidof(interfaceName)

DEFINE_IID(IXAudio2Wrap, 8bcf1f58, 9fe7, 4583, 8a, c6, e2, ad, c4, 65, c8, bb);

DECLARE_INTERFACE_(IXAudio2Wrap, IUnknown)
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

class XAudio2Wrap : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IXAudio2Wrap>
{
private:
	WRL::ComPtr<IXAudio2> m_origPtr;

public:
	STDMETHOD(GetDeviceCount) (THIS_ __out UINT32* pCount)
	{
		*pCount = 0;

		return E_FAIL;
	}

	STDMETHOD(GetDeviceDetails) (THIS_ UINT32 Index, __out void* pDeviceDetails)
	{
		return E_FAIL;
	}

	STDMETHOD(Initialize) (THIS_ UINT32 Flags X2DEFAULT(0),
		XAUDIO2_PROCESSOR XAudio2Processor X2DEFAULT(XAUDIO2_DEFAULT_PROCESSOR))
	{
		HMODULE xAudio2 = LoadLibrary(L"XAudio2_8.dll");
		auto _XAudio2Create = (decltype(&XAudio2Create))GetProcAddress(xAudio2, "XAudio2Create");

		return _XAudio2Create(m_origPtr.GetAddressOf(), Flags, XAudio2Processor);
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


	STDMETHOD(CreateMasteringVoice) (THIS_ __deref_out IXAudio2MasteringVoice** ppMasteringVoice,
		UINT32 InputChannels X2DEFAULT(XAUDIO2_DEFAULT_CHANNELS),
		UINT32 InputSampleRate X2DEFAULT(XAUDIO2_DEFAULT_SAMPLERATE),
		UINT32 Flags X2DEFAULT(0), UINT32 DeviceIndex X2DEFAULT(0),
		__in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL))
	{
		return m_origPtr->CreateMasteringVoice(ppMasteringVoice, InputChannels, InputSampleRate, Flags, nullptr, pEffectChain, AudioCategory_GameEffects);
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

static HRESULT CreateXAudio2Instance(REFIID, void*, DWORD, REFIID, void** ppv)
{
	auto ptr = WRL::Make<XAudio2Wrap>();

	return ptr.CopyTo(IID_IXAudio2Wrap, ppv);
}

#define INITGUID
#include <guiddef.h>
#include <d3dcompiler.h>
#undef INITGUID

static HookFunction hookFunction([]()
{
	if (GetModuleHandle(L"d3dcompiler_47.dll") != nullptr)
	{
		hook::put<IID>(hook::get_pattern("19 37 23 0A 60 39 78 45 9D 7C", 0), IID_ID3D11ShaderReflection);
	}

	HMODULE xAudio2 = LoadLibrary(L"XAudio2_8.dll");

	if (!xAudio2)
	{
		trace("Skipping XAudio2 patches - XAudio 2.8 is not present.\n");
		return;
	}

	trace("XAudio2 patches initializing\n");

	auto loc = hook::get_pattern("49 8D 43 20 33 D2 49 89 43 E8 FF 15", 10);
	hook::nop(loc, 6);
	hook::call(loc, CreateXAudio2Instance);
});