#include <StdInc.h>
#include <CefOverlay.h>

#include <gameSkeleton.h>

#include <jitasm.h>
#include <Hooking.h>

#include <sysAllocator.h>

#include <nutsnbolts.h>

#include <fiDevice.h>

#include <CoreConsole.h>

#include <NetLibrary.h>
#include <EntitySystem.h>

#include <CoreConsole.h>

#include <regex>

#include <MumbleAudioSink.h>
#include <concurrent_queue.h>

#include <MinHook.h>

#include <ScriptEngine.h>
#include <audDspEffect.h>

#include <ICoreGameInit.h>

#include <GameAudioState.h>
#include <CrossBuildRuntime.h>

#include <CL2LaunchMode.h>

static concurrency::concurrent_queue<std::function<void()>> g_mainQueue;

namespace rage
{
	class audCurve
	{
	public:
		// input: units of distance
		// output: attenuation in dB from -100 to 0
		static float DefaultDistanceAttenuation_CalculateValue(float x);
	};

	static hook::cdecl_stub<float(float)> _audCurve_DefaultDistanceAttenuation_CalculateValue([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("0F 28 C8 F3 0F 59 08 48 83 C0 04", -0x38);
#elif IS_RDR3
		return hook::get_pattern("0F 28 D8 0F 28 D0 F3 0F 5C 1D ? ? ? ? F3", -0xF);
#endif
	});

	float audCurve::DefaultDistanceAttenuation_CalculateValue(float x)
	{
		return _audCurve_DefaultDistanceAttenuation_CalculateValue(x);
	}

	class audWaveSlot
	{
	public:
		static audWaveSlot* FindWaveSlot(uint32_t hash);
	};

	class audChannelVoiceVolumes
	{
	public:
		alignas(16) float volumes[6];

		audChannelVoiceVolumes()
		{
			memset(volumes, 0, sizeof(volumes));
		}
	};

	class audMixerSubmix
	{
	public:
		void AddOutput(uint32_t output, bool a2, bool a3);
		void SetEffect(int slot, audDspEffect* effect, uint32_t mask = 0xF);
		void SetFlag(int id, bool value);
		void SetEffectParam(int slot, uint32_t hash, uint32_t value);
		void SetEffectParam(int slot, uint32_t hash, float value);
		void SetOutputVolumes(uint32_t slot, const audChannelVoiceVolumes& volumes);
	};

	static hook::thiscall_stub<void(audMixerSubmix* self, uint32_t output, bool a2, bool a3)> _audMixerSubmix_AddOutput([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("44 88 4C 24 29 0D 28 00 00 03", -0x1B);
#elif IS_RDR3
		return hook::get_pattern("89 44 24 20 44 88 4C 24 ? E8 ? ? ? ? 48 83 C4 38", -0x20);
#endif
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, int slot, audDspEffect* effect, uint32_t mask)> _audMixerSubmix_SetEffect([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("0D 08 00 00 06 89 44 24 20", -0x20);
#elif IS_RDR3
		return hook::get_pattern("0D ? ? ? ? 4C 89 44 24 ? 48 8D 54 24 ? 89 44 24 20", -0x11);
#endif
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, int slot, uint32_t hash, uint32_t value)> _audMixerSubmix_SetEffectParam_int([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("44 89 44 24 24 44 89 4C 24 28 0D 10 00 00", -0x16);
#elif IS_RDR3
		return hook::get_pattern("88 54 24 2C 0D ? ? ? ? 44 89 44 24 ? 48 8D 54 24 ? 89 44 24 20", -0xD);
#endif
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, int slot, uint32_t hash, float value)> _audMixerSubmix_SetEffectParam_float([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("F3 0F 11 5C 24 28 25 07 F8 3F 00 44 89", -0x11);
#elif IS_RDR3
		return hook::get_pattern("F3 0F 11 5C 24 ? 48 8D 54 24 ? 89 44 24 20 44 89 44 24 ? E8", -0x16);
#endif
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, int id, bool value)> _audMixerSubmix_SetFlag([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("0D 20 00 00 03 89 44 24 20", -0x1B);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? BB ? ? ? ? 41 B0 01"));
#endif
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, uint32_t slot, const audChannelVoiceVolumes& volumes)> _audMixerSubmix_SetOutputVolumes([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("25 07 F8 3F 00 89 54 24 24 48 8D", -0x11);
#elif IS_RDR3
		return hook::get_pattern("0F C6 CA E8 89 54 24 24 48 8D 54 24 ? 0F 29 4C 24 ? 89 44 24 20", -0x42);
#endif
	});

	void audMixerSubmix::AddOutput(uint32_t output, bool a2, bool a3)
	{
		return _audMixerSubmix_AddOutput(this, output, a2, a3);
	}

	void audMixerSubmix::SetEffect(int slot, audDspEffect* effect, uint32_t mask /* = 0xF */)
	{
		return _audMixerSubmix_SetEffect(this, slot, effect, mask);
	}

	void audMixerSubmix::SetEffectParam(int slot, uint32_t hash, float value)
	{
		return _audMixerSubmix_SetEffectParam_float(this, slot, hash, value);
	}

	void audMixerSubmix::SetEffectParam(int slot, uint32_t hash, uint32_t value)
	{
		return _audMixerSubmix_SetEffectParam_int(this, slot, hash, value);
	}

	void audMixerSubmix::SetFlag(int id, bool value)
	{
		return _audMixerSubmix_SetFlag(this, id, value);
	}

	void audMixerSubmix::SetOutputVolumes(uint32_t slot, const audChannelVoiceVolumes& volumes)
	{
		return _audMixerSubmix_SetOutputVolumes(this, slot, volumes);
	}

	class audMixerDevice
	{
	public:
		audMixerSubmix* CreateSubmix(const char* name, int numOutputChannels, bool a3);
		uint8_t GetSubmixIndex(audMixerSubmix* submix);
		void ComputeProcessingGraph();
		void FlagThreadCommandBufferReadyToProcess(uint32_t a1 = 0);
		void InitClientThread(const char* name, uint32_t bufferSize);

		inline audMixerSubmix* GetSubmix(int idx)
		{
			if (idx < 0 || idx >= 40)
			{
				return nullptr;
			}

			return (audMixerSubmix*)m_submixes[idx];
		}

	private:
		virtual ~audMixerDevice() = 0;

		uint64_t m_8;
#ifdef GTA_FIVE
		uint8_t m_submixes[40][256];
#elif IS_RDR3
		uint8_t m_submixes[40][368];
#endif
		uint32_t m_numSubmixes;
	};

	static hook::thiscall_stub<audMixerSubmix*(audMixerDevice* self, const char* name, int numOutputChannels, bool a3)> _audMixerDevice_CreateSubmix([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("48 C1 E3 08 89 82 00 28 00", -0x17);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 63 8F ? ? ? ? 48 8B D0"));
#endif
	});
#ifdef GTA_FIVE
	static hook::thiscall_stub<uint8_t(audMixerDevice* self, audMixerSubmix* submix)> _audMixerDevice_GetSubmixIndex([]
	{
		return hook::get_pattern("83 C8 FF C3 48 2B D1", -0x5);
	});

	static hook::thiscall_stub<void(audMixerDevice* self)> _audMixerDevice_ComputeProcessingGraph([]
	{
		return hook::get_pattern("48 63 83 00 28 00 00 48 8B CB 41 BD", -0x28);
	});
#endif
	static hook::thiscall_stub<void(audMixerDevice* self, uint32_t)> _audMixerDevice_FlagThreadCommandBufferReadyToProcess([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("48 8B 81 68 F2 00 00 4E 8B", -0x25);
#elif IS_RDR3
		return hook::get_pattern("41 8B 81 ? ? ? ? 48 8D 14 40 48 03 D2 45 89 54 D1 ? 41", -0x30);
#endif
	});

	static hook::thiscall_stub<void(audMixerDevice* self, const char*, uint32_t)> _audMixerDevice_InitClientThread([]
	{
#ifdef GTA_FIVE
		return hook::get_pattern("B9 B0 00 00 00 45 8B F0 48 8B FA E8", -0x1C);
#elif IS_RDR3
		return hook::get_pattern("48 89 48 DC 89 48 E4", -0x45);
#endif
	});

	audMixerSubmix* audMixerDevice::CreateSubmix(const char* name, int numOutputChannels, bool a3)
	{
		return _audMixerDevice_CreateSubmix(this, name, numOutputChannels, a3);
	}

	uint8_t audMixerDevice::GetSubmixIndex(audMixerSubmix* submix)
	{
#ifdef GTA_FIVE
		return _audMixerDevice_GetSubmixIndex(this, submix);
#elif IS_RDR3
		if (!submix)
		{
			return -1;
		}

		return *reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(submix) + 0x150);
#endif
	}

#ifdef GTA_FIVE
	void audMixerDevice::ComputeProcessingGraph()
	{
		return _audMixerDevice_ComputeProcessingGraph(this);
	}
#endif
	void audMixerDevice::FlagThreadCommandBufferReadyToProcess(uint32_t a1 /* = 0 */)
	{
		return _audMixerDevice_FlagThreadCommandBufferReadyToProcess(this, a1);
	}

	void audMixerDevice::InitClientThread(const char* name, uint32_t bufferSize)
	{
		return _audMixerDevice_InitClientThread(this, name, bufferSize);
	}

	class audDriver
	{
	public:
		inline static audMixerDevice* GetMixer()
		{
			return *sm_Mixer;
		}

	public:
		static audMixerDevice** sm_Mixer;
	};

	class audRequestedSettings;

	class audSound
	{
	public:
#ifdef GTA_FIVE
		virtual ~audSound() = 0;

		virtual void m_8() = 0;

		virtual void m_10() = 0;

		virtual void m_18() = 0;

		virtual void Init() = 0;

		virtual void m_28() = 0;
#elif IS_RDR3
		virtual bool FindAndSetVariableValueWrapper(void) = 0;

		virtual bool FindAndSetVariableValue(void) = 0;

		virtual bool FindAndSetVariableHashValue(void) = 0;

		virtual void throw__0x52D76AA0_01() = 0;

		virtual uint64_t Pause(uint32_t unk) = 0;

		virtual uint64_t FindVariableDownHierarchy(uint32_t, uint32_t) = 0;

		virtual uint64_t FindVariableUpHierarchy(uint32_t, bool) = 0;

		virtual ~audSound() = 0;

		virtual uint64_t Init(const class audSoundInternalInitParams*, class audSoundScratchInitParams*, void*) = 0;

		virtual void throw__0x52D76AA0_02() = 0;

		virtual void throw__0x52D76AA0_03() = 0;

		virtual void throw__0x52D76AA0_04() = 0;

		virtual void throw__0x52D76AA0_05() = 0;

		virtual uint64_t ActionReleaseRequest(uint32_t) = 0;
#endif

		void PrepareAndPlay(audWaveSlot* waveSlot, bool a2, int a3, bool a4);

		void StopAndForget(bool a1);

		audRequestedSettings* GetRequestedSettings();

	public:
#ifdef GTA_FIVE
		char pad[141 - 8];
		uint8_t unkBitFlag : 3;
#elif IS_RDR3
		char pad_0008[152];
		uint8_t bucketID;
		char pad_00A1[53];
		uint16_t settingsID;
#endif
	};

	static hook::cdecl_stub<void(audSound*, void*, bool, int, bool)> _audSound_PrepareAndPlay([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("0F 85 ? 00 00 00 41 83 CB FF 45 33 C0", -0x35);
#elif IS_RDR3
		return hook::get_pattern("48 83 EC 20 33 DB 41 8B F9 45 8A F0", -0x15);
#endif
	});

	static hook::cdecl_stub<void(audSound*, bool)> _audSound_StopAndForget([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("74 24 45 0F B6 41 62", -0x10);
#elif IS_RDR3
		return hook::get_pattern("88 91 ? ? ? ? 8A C2 4D 8B 41 58", -0x52);
#endif
	});

	void audSound::PrepareAndPlay(audWaveSlot* a1, bool a2, int a3, bool a4)
	{
		_audSound_PrepareAndPlay(this, a1, a2, a3, a4);
	}

	void audSound::StopAndForget(bool a1)
	{
		_audSound_StopAndForget(this, a1);
	}

	class audReferencedRingBuffer : public sysUseAllocator
	{
	public:
		audReferencedRingBuffer();

	private:
		~audReferencedRingBuffer();

	public:
		inline void SetBuffer(void* buffer, uint32_t size)
		{
			m_data = buffer;
			m_size = size;
			m_initialized = true;
		}

		uint32_t PushAudio(const void* data, uint32_t size);

		inline void Release()
		{
			if (InterlockedDecrement(&m_usageCount) == 0)
			{
				delete this;
			}
		}

		int GetCustomMode();
		void SetCustomMode(int idx);

	private:
		void* m_data; // +0
		uint32_t m_size; // +8
		uint32_t m_fC; // +12
		uint32_t m_f10_0; // +16
		uint32_t m_f14_0; // +20
		uint32_t m_f18_0; // +24
		uint32_t m_pad_f1C; // +28
		CRITICAL_SECTION m_lock; // +32
		bool m_f48_0; // +72
		bool m_initialized; // +73
		uint8_t m_pad_f4A[6]; // +74
		uint32_t m_usageCount; // +80
		uint32_t m_f54; // +84
	};

	audReferencedRingBuffer::audReferencedRingBuffer()
	{
		m_data = nullptr;
		m_size = 0;

		m_f10_0 = 0;
		m_f14_0 = 0;
		m_f18_0 = 0;
		m_f48_0 = false;
		m_initialized = false;

		m_usageCount = 1;

		InitializeCriticalSectionAndSpinCount(&m_lock, 1000);
	}

	audReferencedRingBuffer::~audReferencedRingBuffer()
	{
		if (m_data)
		{
			rage::GetAllocator()->Free(m_data);
			m_data = nullptr;
		}

		DeleteCriticalSection(&m_lock);
	}

	int audReferencedRingBuffer::GetCustomMode()
	{
		if (m_pad_f1C == 0xBEEFCA3E)
		{
			return *(int*)&m_pad_f4A[0];
		}

		return -1;
	}

	void audReferencedRingBuffer::SetCustomMode(int idx)
	{
		m_pad_f1C = 0xBEEFCA3E;
		*(int*)&m_pad_f4A[0] = idx;
	}

	static hook::cdecl_stub<uint32_t(audReferencedRingBuffer*, const void*, uint32_t)> _audReferencedRingBuffer_PushAudio([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("8B 71 08 48 8B 29 2B F0 48 8B D9 41 8B", -0x1B);
#elif IS_RDR3
		return hook::get_pattern("44 8B 49 14 41 8B F8 8B 41 08 41 8B C9", -0x28);
#endif
	});

	uint32_t audReferencedRingBuffer::PushAudio(const void* data, uint32_t size)
	{
		return _audReferencedRingBuffer_PushAudio(this, data, size);
	}

	class audExternalStreamSound : public rage::audSound
	{
	public:
		bool InitStreamPlayer(rage::audReferencedRingBuffer* buffer, int channels, int frequency);
	};

	static hook::cdecl_stub<bool(rage::audExternalStreamSound*, rage::audReferencedRingBuffer*, int, int)> _audExternalStreamSound_InitStreamPlayer([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("F0 FF 42 50 48 89 11 44 89 41", -0x2A);
#elif IS_RDR3
		return hook::get_pattern("49 03 8C 02 ? ? ? ? 74 12", -0x23);
#endif
	});

	bool audExternalStreamSound::InitStreamPlayer(rage::audReferencedRingBuffer* buffer, int channels, int frequency)
	{
		return _audExternalStreamSound_InitStreamPlayer(this, buffer, channels, frequency);
	}

	class audCategory
	{

	};

	class audCategoryManager
	{
	public:
		rage::audCategory* GetCategoryPtr(uint32_t category);
	};

	static hook::cdecl_stub<rage::audCategory*(rage::audCategoryManager*, uint32_t)> _audCategoryManager_GetCategoryPtr([]()
	{
#ifdef GTA_FIVE
		return hook::get_call(hook::get_pattern("48 8B CB BA EA 75 96 D5 E8", 8));
#elif IS_RDR3
		return hook::get_pattern("43 8D 04 08 99 2B C2 D1 F8 8B D0 8B C8 48 03 C0", -0x2C);
#endif
	});

	rage::audCategory* audCategoryManager::GetCategoryPtr(uint32_t category)
	{
		return _audCategoryManager_GetCategoryPtr(this, category);
	}

	struct Vec3V
	{
		float x;
		float y;
		float z;
		float pad;
	};

	struct audOrientation
	{
		float x;
		float y;
	};

	class audTracker
	{
	public:
		virtual ~audTracker() = default;

		virtual rage::Vec3V GetPosition()
		{
			return { 0.f, 0.f, 0.f, 0.f };
		}

		virtual rage::audOrientation GetOrientation()
		{
			return { 0.f, 0.f };
		}
	};

	class audEnvironmentGroupInterface
	{
	public:
		virtual ~audEnvironmentGroupInterface() = 0;
	};

	class audSoundInitParams
	{
	public:
		audSoundInitParams();

		void SetCategory(rage::audCategory* category);

		void SetEnvironmentGroup(rage::audEnvironmentGroupInterface* environmentGroup);

		void SetVolume(float volume);

		void SetPositional(bool positional);

		void SetTracker(audTracker* parent);

		void SetPosition(float x, float y, float z);

		void SetSubmixIndex(uint8_t index);

		void SetUnk();

		void SetAllocationBucket(uint8_t bucket);

	private:
#ifdef GTA_FIVE
		uint8_t m_pad[0xB0];
#elif IS_RDR3
		uint8_t m_pad[0x150];
#endif
	};

	void audSoundInitParams::SetCategory(rage::audCategory* category)
	{
#ifdef GTA_FIVE
		*(audCategory**)(&m_pad[88]) = category;
#elif IS_RDR3
		*(audCategory**)(&m_pad[216]) = category;
#endif
	}

	void audSoundInitParams::SetEnvironmentGroup(rage::audEnvironmentGroupInterface* environmentGroup)
	{
#ifdef GTA_FIVE
		*(audEnvironmentGroupInterface**)(&m_pad[96]) = environmentGroup;
#elif IS_RDR3
		*(audEnvironmentGroupInterface**)(&m_pad[224]) = environmentGroup;
#endif
	}

	void audSoundInitParams::SetPosition(float x, float y, float z)
	{
		auto f = (float*)m_pad;

		f[0] = x;
		f[1] = y;
		f[2] = z;
		f[3] = 0.f;
	}

	void audSoundInitParams::SetVolume(float volume)
	{
#ifdef GTA_FIVE
		*(float*)(&m_pad[48]) = volume;
#elif IS_RDR3
		*(float*)(&m_pad[164]) = volume;
#endif
	}

	void audSoundInitParams::SetTracker(audTracker* parent)
	{
#ifdef GTA_FIVE
		*(audTracker**)(&m_pad[72]) = parent;
#elif IS_RDR3
		*(audTracker**)(&m_pad[200]) = parent;
#endif
	}

	void audSoundInitParams::SetPositional(bool positional)
	{
#ifdef GTA_FIVE
		if (positional)
		{
			m_pad[158] |= 1;
		}
		else
		{
			m_pad[158] &= ~1;
		}
#elif IS_RDR3
		if (positional)
		{
			m_pad[315] |= 1;
		}
		else
		{
			m_pad[315] &= ~1;
		}
#endif
	}

	void audSoundInitParams::SetSubmixIndex(uint8_t submix)
	{
#ifdef GTA_FIVE
		m_pad[155] = (submix - 0x1C) | 0x20;
		// this field does really weird stuff in game
		//*(uint8_t*)(&m_pad[0x98]) = submix;
		//*(uint16_t*)(&m_pad[0x9B]) = 3;
#elif IS_RDR3
		*(uint16_t*)(&m_pad[0x134]) = (uint16_t)(submix);
#endif
	}
#ifdef GTA_FIVE
	void audSoundInitParams::SetUnk()
	{
		m_pad[155] = 27;
	}
#endif
	void audSoundInitParams::SetAllocationBucket(uint8_t bucket)
	{
#ifdef GTA_FIVE
		m_pad[154] = bucket;
#elif IS_RDR3
		m_pad[310] = bucket;
#endif
	}

	static hook::cdecl_stub<void(rage::audSoundInitParams*)> _audSoundInitParams_ctor([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("0F 57 C0 48 8D 41 10 BA 03 00 00 00");
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? 8A 45 7A"));
#endif
	});

	static uint8_t* initParamVal;

	audSoundInitParams::audSoundInitParams()
	{
		_audSoundInitParams_ctor(this);

		SetAllocationBucket(*initParamVal);
	}

	class audRequestedSettings
	{
	public:
		void SetQuadSpeakerLevels(float levels[4]);

		void SetVolume(float vol);

		void SetVolumeCurveScale(float sca);

		void SetEnvironmentalLoudness(uint8_t val);

		void SetSourceEffectMix(float wet, float dry);
	};

	static hook::thiscall_stub<void(audRequestedSettings*, float)> _audRequestedSettings_SetVolume([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("F3 0F 11 8C D1 90 00 00 00", -0x11);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? F3 44 0F 58 4E"));
#endif
	});

	static hook::thiscall_stub<void(audRequestedSettings*, float)> _audRequestedSettings_SetVolumeCurveScale([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("F3 0F 11 8C D1 A8 00 00 00", -0x11);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? 38 5E 6C"));
#endif
	});

	static hook::thiscall_stub<void(audRequestedSettings*, uint8_t)> _audRequestedSettings_SetEnvironmentalLoudness([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("42 88 94 C1 B1 00 00 00", -0x11);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? C7 87 ? ? ? ? ? ? ? ? 48 8B 83 ? ? ? ? 48 85 C0 74 52"));
#endif
	});

	static hook::thiscall_stub<void(audRequestedSettings*, uint8_t)> _audRequestedSettings_SetSpeakerMask([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("42 88 94 C1 B0 00 00 00", -0x11);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8A 86 ? ? ? ? 48 8B CF"));
#endif
	});

	static hook::thiscall_stub<void(audRequestedSettings*, float, float)> _audRequestedSettings_SetSourceEffectMix([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("F3 0F 11 8C D1 98 00 00 00", -0x11);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? F3 0F 10 0D ? ? ? ? 49 8B CF F3 0F 58 4E"));
#endif
	});

	static hook::thiscall_stub<void(audRequestedSettings*, float[4])> _audRequestedSettings_SetQuadSpeakerLevels([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("B8 00 80 00 00 66 09 84", -0x61);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? 40 8A B5 ? ? ? ? 40 80 FE 01"));
#endif
	});

	void audRequestedSettings::SetVolume(float vol)
	{
		_audRequestedSettings_SetVolume(this, vol);
	}

	void audRequestedSettings::SetQuadSpeakerLevels(float levels[4])
	{
		_audRequestedSettings_SetQuadSpeakerLevels(this, levels);
	}

	void audRequestedSettings::SetVolumeCurveScale(float vol)
	{
		_audRequestedSettings_SetVolumeCurveScale(this, vol);
	}

	void audRequestedSettings::SetEnvironmentalLoudness(uint8_t vol)
	{
		_audRequestedSettings_SetEnvironmentalLoudness(this, vol);
	}

	void audRequestedSettings::SetSourceEffectMix(float wet, float dry)
	{
		_audRequestedSettings_SetSourceEffectMix(this, wet, dry);
	}

	// NOTE: this class *should not have fields* as they'd be before the vtable
	class audEntityBase
	{
	public:
		void CreateSound_PersistentReference(const char* name, audSound** outSound, const audSoundInitParams& params);

		void CreateSound_PersistentReference(uint32_t nameHash, audSound** outSound, const audSoundInitParams& params);
	};

	class audEntityBaseOld : public audEntityBase
	{
	public:
		virtual ~audEntityBaseOld() = default;
	};

	class audEntityBase2802 : public audEntityBase
	{
	private:
		inline virtual void* _2802_1()
		{
			return nullptr;
		}

		inline virtual void* _2802_2()
		{
			return nullptr;
		}

		inline virtual void* _2802_3()
		{
			return nullptr;
		}

		inline virtual void* _2802_4()
		{
			return nullptr;
		}

		inline virtual void* _2802_5()
		{
			return nullptr;
		}

		inline virtual void* _2802_6()
		{
			return nullptr;
		}

	public:
		// the destructor slot shouldn't move or be duplicated, but the base needs to have a vtable
		virtual ~audEntityBase2802() = default;
	};

	template<int Build>
	class audEntityBaseBuild : public std::conditional_t<Build >= 2802, audEntityBase2802, audEntityBaseOld>
	{
	protected:
		uint16_t m_entityId{
			0xffff
		};

		uint16_t m_0A{
			0xffff
		};
	};

	template<int Build>
	class audEntity : public audEntityBaseBuild<Build>
	{
	public:
		audEntity();

		virtual ~audEntity();
#if IS_RDR3
		virtual void unk_0x8()
		{
		}
#endif
		virtual void Init();

		virtual void Shutdown();

		virtual void StopAllSounds(bool);

		virtual void PreUpdateService(uint32_t)
		{
		
		}
#if IS_RDR3
		virtual void PreUpdateServiceInternal(uint32_t a1)
		{

		}
#endif
		virtual void PostUpdate()
		{

		}

		virtual void UpdateSound(rage::audSound*, rage::audRequestedSettings*, uint32_t)
		{

		}
#if IS_RDR3
		virtual bool HasPendingAnimEvents()
		{
			return false;
		}

		virtual bool HasPendingDeferredSounds()
		{
			return false;
		}

		virtual void unk_0x58()
		{

		}
#endif
		virtual bool IsUnpausable()
		{
			return false;
		}

		virtual uint32_t QuerySoundNameFromObjectAndField(const uint32_t*, uint32_t, const rage::audSound*)
		{
			return 0;
		}

		virtual void QuerySpeechVoiceAndContextFromField(uint32_t, uint32_t&, uint32_t&)
		{

		}
#if IS_RDR3
		virtual uint64_t GetEnvironmentGroup(bool a1)
		{
			return 0;
		}

		virtual uint64_t GetEnvironmentGroupReadOnly()
		{
			return 0;
		}
#endif
		virtual rage::Vec3V GetPosition()
		{
			return {0.f, 0.f, 0.f, 0.f};
		}

		virtual rage::audOrientation GetOrientation()
		{
			return { 0.f, 0.f };
		}
#if IS_RDR3
		virtual void unk_0x98()
		{

		}
#endif
		virtual uint32_t InitializeEntityVariables()
		{
			m_0A = -1;
			return -1;
		}

		virtual void* GetOwningEntity()
		{
			return nullptr;
		}

	private:
#if IS_RDR3
		char m_pad[8] = {};
#endif
		uint16_t m_entityId{
			0xffff
		};

		uint16_t m_0A{
			0xffff
		};
#if IS_RDR3
		uint32_t state{
			1
		};
#endif
	};

	template<int Build>
	audEntity<Build>::audEntity()
	{

	}

	template<int Build>
	audEntity<Build>::~audEntity()
	{
		Shutdown();
	}
#ifdef GTA_FIVE
	static hook::cdecl_stub<void(audEntityBaseOld*, const char*, audSound**, const audSoundInitParams&)> _audEntity_CreateSound_PersistentReference_char([]()
	{
		return hook::get_call(hook::get_pattern("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0x14));
	});
#endif
	static hook::cdecl_stub<void(audEntityBaseOld*, uint32_t, audSound**, const audSoundInitParams&)> _audEntity_CreateSound_PersistentReference_uint([]()
	{
#ifdef GTA_FIVE
		return (void*)hook::get_call(hook::get_call(hook::get_pattern<char>("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0x14)) + 0x3F);
#elif IS_RDR3
		return hook::get_pattern("48 89 78 20 41 56 48 81 EC ? ? ? ? 83 79 14 00 49 8B", -0x18);
#endif
	});
#ifdef GTA_FIVE
	void audEntityBase::CreateSound_PersistentReference(const char* name, audSound** outSound, const audSoundInitParams& params)
	{
		return _audEntity_CreateSound_PersistentReference_char(static_cast<audEntityBaseOld*>(this), name, outSound, params);
	}
#endif
	void audEntityBase::CreateSound_PersistentReference(uint32_t nameHash, audSound** outSound, const audSoundInitParams& params)
	{
		return _audEntity_CreateSound_PersistentReference_uint(static_cast<audEntityBaseOld*>(this), nameHash, outSound, params);
	}

	static hook::thiscall_stub<void(void*)> rage__audEntity__Init([]()
	{
#ifdef GTA_FIVE
		return hook::get_call(hook::get_pattern("48 81 EC C0 00 00 00 48 8B D9 E8 ? ? ? ? 45 33 ED", 10));
#elif IS_RDR3
		return hook::get_pattern("48 83 EC 28 80 3D ? ? ? ? ? 74 16 83 79 14 01");
#endif
	});

	static hook::thiscall_stub<void(void*)> rage__audEntity__Shutdown([]()
	{
#ifdef GTA_FIVE
		return hook::get_call(hook::get_pattern("48 83 EC 20 48 8B F9 E8 ? ? ? ? 33 ED", 7));
#elif IS_RDR3
		return hook::get_pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 66 83 7B ? ? 7C 10");
#endif
	});

	static hook::thiscall_stub<void(void*, bool)> rage__audEntity__StopAllSounds([]()
	{
#ifdef GTA_FIVE
		return hook::get_call(hook::get_pattern("0F 82 67 FF FF FF E8 ? ? ? ? 84 C0", 24));
#elif IS_RDR3
		return hook::get_pattern("48 83 EC 28 B8 ? ? ? ? 66 39 41 10 74 12");
#endif
	});

	template<int Build>
	void audEntity<Build>::Init()
	{
		rage__audEntity__Init(this);
	}

	template<int Build>
	void audEntity<Build>::Shutdown()
	{
		rage__audEntity__Shutdown(this);
	}

	template<int Build>
	void audEntity<Build>::StopAllSounds(bool a)
	{
		rage__audEntity__StopAllSounds(this, a);
	}

	audEntityBase* g_frontendAudioEntity;

	audCategoryManager* g_categoryMgr;

	class audCategoryControllerManager
	{
	public:
		char* CreateController(uint32_t hash);

		static audCategoryControllerManager* GetInstance();
	};

	audCategoryControllerManager* audCategoryControllerManager::GetInstance()
	{
#ifdef GTA_FIVE
		static auto patternRef = hook::get_address<audCategoryControllerManager**>(hook::get_pattern("45 33 C0 BA 90 1C E2 44 E8", -4));
#elif IS_RDR3
		static auto patternRef = hook::get_address<audCategoryControllerManager**>(hook::get_pattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 8B 15 ? ? ? ? 48 8D 0D ? ? ? ? E8", 3));
#endif
		return *patternRef;
	}

	static hook::thiscall_stub<char*(audCategoryControllerManager*, uint32_t)> _audCategoryControllerManager_CreateController([]()
	{
#ifdef GTA_FIVE
		return hook::get_call(hook::get_pattern("45 33 C0 BA 90 1C E2 44 E8", 8));
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 89 45 D0 48 8B C8"));
#endif
	});

	char* audCategoryControllerManager::CreateController(uint32_t hash)
	{
		return _audCategoryControllerManager_CreateController(this, hash);
	}

	static HookFunction hookFunction([]()
	{
#ifdef GTA_FIVE
		g_frontendAudioEntity = hook::get_address<audEntityBase*>(hook::get_pattern("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0xC));

		g_categoryMgr = hook::get_address<audCategoryManager*>(hook::get_pattern("48 8B CB BA EA 75 96 D5 E8", -4));

		initParamVal = hook::get_address<uint8_t*>(hook::get_pattern("BA 11 CC 23 C3 E8 ? ? ? ? 48 8D", 0x16));

		audDriver::sm_Mixer = hook::get_address<audMixerDevice**>(hook::get_pattern("75 64 44 0F B7 45 06 48 8B 0D", 10));
#elif IS_RDR3
		g_frontendAudioEntity = hook::get_address<audEntityBase*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 45 84 E4 74 ? 39 1D"), 3, 7);

		g_categoryMgr = hook::get_address<audCategoryManager*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? BE ? ? ? ? 48 8D 0D ? ? ? ? 8B D6"), 3, 7);

		initParamVal = hook::get_address<uint8_t*>(hook::get_pattern("8A 05 ? ? ? ? 48 8B CF F3 0F 11 45 ? 88 45 66"), 2, 6);

		audDriver::sm_Mixer = hook::get_address<audMixerDevice**>(hook::get_pattern("48 8B 05 ? ? ? ? 44 38 8C 01 ? ? ? ? 0F"), 3, 7);
#endif
	});
#ifdef GTA_FIVE
	static hook::cdecl_stub<audWaveSlot*(uint32_t)> _findWaveSlot([]()
	{
		return hook::get_call(hook::get_pattern("0F 85 ? ? ? ? B9 A1 C7 05 92 E8", 11));
	});

	audWaveSlot* audWaveSlot::FindWaveSlot(uint32_t hash)
	{
		return _findWaveSlot(hash);
	}
#endif
	static hook::cdecl_stub<float(float)> _linearToDb([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("8B 4C 24 08 8B C1 81 E1 FF FF 7F 00", -0x14);
#elif IS_RDR3
		return hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8B 73 18"));
#endif
	});

	float GetDbForLinear(float x)
	{
		return _linearToDb(x);
	}

	static uint64_t* _settingsBase;
	static uint32_t* _settingsIdx;

	audRequestedSettings* audSound::GetRequestedSettings()
	{
#ifdef GTA_FIVE
		char* v4 = (char*)this;

		audRequestedSettings* v5 = nullptr;
		uint8_t v7 = *(unsigned __int8*)(v4 + 128);
		if (v7 != 255)
			v5 = (audRequestedSettings*)(*(uint64_t*)(13520i64 * *(unsigned __int8*)(v4 + 98) + *_settingsBase + 13504)
				 + (unsigned int)(size_t(v7) * *_settingsIdx));

		return v5;
#elif IS_RDR3
		int16_t v7 = *reinterpret_cast<int16_t*>(reinterpret_cast<char*>(this) + 0xD6);
		int16_t v8 = *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(this) + 0xA0);
		if (v7 != 255)
			return reinterpret_cast<audRequestedSettings*>(*reinterpret_cast<uint64_t*>(0x2A860 * v8 + *_settingsBase + 0x2A850) + static_cast<uint32_t>(v7 * *_settingsIdx));

		return nullptr;
#endif
	}

	static HookFunction hfRs([]()
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("74 23 0F B6 48 62 0F AF 15");
		_settingsIdx = hook::get_address<uint32_t*>(location + 9);
		_settingsBase = hook::get_address<uint64_t*>(location + 16);
#elif IS_RDR3
		auto location = hook::get_pattern<char>("48 8B 43 EE 66 0F 7F 74 24 ? 0F B7");
		_settingsIdx = hook::get_address<uint32_t*>(location + 0x23);
		_settingsBase = hook::get_address<uint64_t*>(location + 0x31);
#endif
	});

	struct audStreamPlayer
	{
		void* vtbl;
		uint8_t pad[48 - 8];
		audReferencedRingBuffer* ringBuffer; // +48
		void* pad2; // +56
		uint32_t pad3; // +64
		uint32_t size; // +68
		uint8_t pad4[11]; // +72
		uint8_t frameOffset;
	};
}

class naEnvironmentGroup : public rage::audEnvironmentGroupInterface
{
public:
	static naEnvironmentGroup* Create();

	void Init(void* a2, float a3, int a4, int a5, float a6, int a7);

	void SetPosition(const rage::Vec3V& position);

	void SetInteriorLocation(rage::fwInteriorLocation location);
};

static hook::cdecl_stub<naEnvironmentGroup*()> _naEnvironmentGroup_create([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("F6 04 02 01 74 0A 8A 05", -0x24);
#elif IS_RDR3
	return hook::get_pattern("40 53 48 83 EC 20 33 DB 38 1D ? ? ? ? 0F 84 ? ? ? ? 65 48 8B 0C");
#endif
});

static hook::thiscall_stub<void(naEnvironmentGroup*, void* a2, float a3, int a4, int a5, float a6, int a7)> _naEnvironmentGroup_init([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("80 A7 ? 01 00 00 FC F3 0F 10", -0x22);
#elif IS_RDR3
	return hook::get_pattern("F3 0F 59 C0 F3 0F 59 F6 F3 0F 11", -0x41);
#endif
});

static hook::thiscall_stub<void(naEnvironmentGroup*, const rage::Vec3V& position)> _naEnvironmentGroup_setPosition([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("F3 0F 11 41 74 F3 0F 11 49 78 C3", -0x1F);
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 80 7B 76 00"));
#endif
});

static hook::thiscall_stub<void(naEnvironmentGroup*, rage::fwInteriorLocation)> _naEnvironmentGroup_setInteriorLocation([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("3B 91 ? 00 00 00 74 07 80 89", -0x17);
#elif IS_RDR3
	return hook::get_pattern("89 54 24 10 53 48 83 EC 20 80");
#endif
});

naEnvironmentGroup* naEnvironmentGroup::Create()
{
	return _naEnvironmentGroup_create();
}

void naEnvironmentGroup::Init(void* entity, float a3, int a4, int a5, float a6, int a7)
{
	_naEnvironmentGroup_init(this, entity, a3, a4, a5, a6, a7);
}

void naEnvironmentGroup::SetPosition(const rage::Vec3V& position)
{
	_naEnvironmentGroup_setPosition(this, position);
}

void naEnvironmentGroup::SetInteriorLocation(rage::fwInteriorLocation location)
{
	_naEnvironmentGroup_setInteriorLocation(this, location);
}

#ifdef GTA_FIVE
static hook::cdecl_stub<void()> _updateAudioThread([]()
{
	return hook::get_pattern("40 0F 95 C7 40 84 FF 74 05", -0x14);
});
#elif IS_RDR3
static hook::cdecl_stub<void(bool update_envgroups)> _updateAudioThread([]()
{
	return hook::get_pattern("40 8A E9 48 8B 0D", -0x14);
});

static hook::thiscall_stub<void(void*, bool a2, bool a3)> StartMenuMusic([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8D 94 24"));
});
#endif

extern "C"
{
#include <libswresample/swresample.h>
};

class MumbleAudioEntityBase
{
public:
	MumbleAudioEntityBase(const std::wstring& name)
		: m_position(rage::Vec3V{ 0.f, 0.f, 0.f }),
		  m_positionForce(rage::Vec3V{ 0.f, 0.f, 0.f }),
		  m_buffer(nullptr),
		  m_sound(nullptr), m_bufferData(nullptr), m_environmentGroup(nullptr), m_distance(5.0f), m_overrideVolume(-1.0f),
		  m_ped(nullptr),
		  m_name(name)
	{
	}

	virtual ~MumbleAudioEntityBase() = default;

	virtual void MInit(float overrideVolume) = 0;
	virtual void MShutdown() = 0;

	void SetPosition(float position[3], float distance, float overrideVolume)
	{
		if (m_ped)
		{
			auto pedPos = m_ped->GetPosition();

			m_position = {
				pedPos.x, pedPos.y, pedPos.z
			};
		}
		else
		{
			m_position = { position[0],
				position[1], position[2] };
		}

		m_distance = distance;
		m_overrideVolume = overrideVolume;
	}

	void SetBackingEntity(CPed* ped)
	{
		m_ped = ped;
	}
	
	void SetPoller(const std::function<void(int)>& poller)
	{
		m_poller = poller;
	}

	void SetSubmixId(int id)
	{
		m_submixId = id;
	}

	void PushAudio(int16_t* pcm, int len);

	void Poll(int samples);

protected:
	/// <summary>
	/// @FIX(mockingbird-burger-timing): MShutdown is executed on MainThrd while
	/// accessed on NorthAudioUpdate; handle race condition.
	/// </summary>
	std::mutex m_render;

	rage::audExternalStreamSound* m_sound;
	uint8_t m_soundBucket = -1;

	alignas(16) rage::Vec3V m_position;
	float m_distance;
	float m_overrideVolume;

	alignas(16) rage::Vec3V m_positionForce;

	rage::audReferencedRingBuffer* m_buffer;

	uint8_t* m_bufferData;

	naEnvironmentGroup* m_environmentGroup;

	int m_submixId = -1;

	int m_customEntryId = -1;

	CPed* m_ped;

	std::wstring m_name;

	std::function<void(int)> m_poller;
};

template<int Build>
class MumbleAudioEntity : public rage::audEntity<Build>, public MumbleAudioEntityBase, public std::enable_shared_from_this<MumbleAudioEntity<Build>>
{
public:
	MumbleAudioEntity(const std::wstring& name)
		: MumbleAudioEntityBase(name)
	{
	}

	virtual ~MumbleAudioEntity() override;

	virtual void Init() override;

	virtual void Shutdown() override;

	virtual void MInit(float overrideVolume) override;
	
	virtual void MShutdown() override;

	virtual rage::Vec3V GetPosition() override
	{
		if (m_positionForce.x != 0.0f || m_positionForce.y != 0.0f || m_positionForce.z != 0.0f)
		{
			return m_positionForce;
		}

		return m_position;
	}

	virtual void PreUpdateService(uint32_t) override;

	virtual bool IsUnpausable() override
	{
		return true;
	}
};


static void (*g_origGenerateFrame)(rage::audStreamPlayer* self);
static std::shared_mutex g_customEntriesLock;

static std::map<int, std::weak_ptr<MumbleAudioEntityBase>> g_customEntries;

void GenerateFrameHook(rage::audStreamPlayer* self)
{
	if (self->ringBuffer)
	{
		auto buffer = self->ringBuffer;
		if (auto idx = buffer->GetCustomMode(); idx >= 0)
		{
			std::shared_ptr<MumbleAudioEntityBase> entity;

			{
				std::shared_lock _(g_customEntriesLock);
				if (auto entry = g_customEntries.find(idx); entry != g_customEntries.end())
				{
					entity = entry->second.lock();
				}
			}

			if (entity)
			{
				// every read (native frame) is 256 samples
				entity->Poll(256 - self->frameOffset);
			}
		}
	}

	g_origGenerateFrame(self);
}

template<int Build>
MumbleAudioEntity<Build>::~MumbleAudioEntity()
{
	if (m_customEntryId >= 0)
	{
		std::unique_lock _(g_customEntriesLock);
		g_customEntries.erase(m_customEntryId);
	}

	// directly call MShutdown
	// Shutdown will be called on the base object
	MShutdown();
}

template<int Build>
void MumbleAudioEntity<Build>::Init()
{
	rage::audEntity<Build>::Init();

	MInit(m_overrideVolume);
}

template<int Build>
void MumbleAudioEntity<Build>::Shutdown()
{
	MShutdown();

	rage::audEntity<Build>::Shutdown();
}

void MumbleAudioEntityBase::Poll(int samples)
{
	if (m_poller)
	{
		m_poller(samples);
	}
}

static constexpr int kExtraAudioBuckets = 6;
static uint32_t bucketsUsed[kExtraAudioBuckets];

template<int Build>
void MumbleAudioEntity<Build>::MInit(float overrideVolume)
{
	std::lock_guard _(m_render);
	m_environmentGroup = naEnvironmentGroup::Create();
	m_environmentGroup->Init(nullptr, 20.0f, 1000, 4000, 0.5f, 1000);
	m_environmentGroup->SetPosition(m_position);

	rage::audSoundInitParams initValues;

	// set the audio category
	auto category = rage::g_categoryMgr->GetCategoryPtr(HashString("MUMBLE"));

	if (category)
	{
		initValues.SetCategory(category);
	}

#ifdef GTA_FIVE
	initValues.SetPositional(true);

	initValues.SetEnvironmentGroup(m_environmentGroup);
#elif IS_RDR3
	if (overrideVolume < 0.0)
	{
		m_environmentGroup->SetPosition(m_position);
		initValues.SetEnvironmentGroup(m_environmentGroup);
		initValues.SetPositional(true);
	}
#endif

	if (m_submixId >= 0)
	{
		initValues.SetSubmixIndex(m_submixId);
	}

	if (m_soundBucket == 0xFF)
	{
		int curLowestIdx = -1;
		size_t curLowest = SIZE_MAX;

		for (int bucketIdx = 0; bucketIdx < std::size(bucketsUsed); bucketIdx++)
		{
			if (bucketsUsed[bucketIdx] < curLowest)
			{
				curLowestIdx = bucketIdx;
				curLowest = bucketsUsed[bucketIdx];
			}
		}

		if (curLowestIdx >= 0 && curLowestIdx < kExtraAudioBuckets)
		{
			m_soundBucket = curLowestIdx;
			++bucketsUsed[m_soundBucket];
		}
	}

#ifdef GTA_FIVE
	initValues.SetAllocationBucket(12 + m_soundBucket);
#elif IS_RDR3
	initValues.SetAllocationBucket(8 + m_soundBucket);
#endif

	// CreateSound_PersistentReference(0xD8CE9439, (rage::audSound**)&m_sound, initValues);
#ifdef GTA_FIVE
	CreateSound_PersistentReference(0x8460F301, (rage::audSound**)&m_sound, initValues);
#elif IS_RDR3
	CreateSound_PersistentReference(0x0F4A60A9, (rage::audSound**)&m_sound, initValues);
#endif

	//trace("created sound (%s): %016llx\n", ToNarrow(m_name), (uintptr_t)m_sound);

	if (m_sound)
	{
		// have 0.125 second of audio buffer, as it seems the game will consume the full buffer where possible
		auto size = (48000 * sizeof(int16_t) * 1) / 8;
		m_bufferData = (uint8_t*)rage::GetAllocator()->Allocate(size, 16, 0);

		auto buffer = new rage::audReferencedRingBuffer();
		buffer->SetBuffer(m_bufferData, size);

		{
			std::unique_lock _(g_customEntriesLock);
			static std::atomic<int> i;
			int id = ++i;

			auto self = shared_from_this();
			g_customEntries[id] = self;
			m_customEntryId = id;

			buffer->SetCustomMode(id);
		}

		m_sound->InitStreamPlayer(buffer, 1, 48000);
		m_sound->PrepareAndPlay(nullptr, true, -1, false);

		m_buffer = buffer;
	}
}

template<int Build>
void MumbleAudioEntity<Build>::MShutdown()
{
	std::lock_guard _(m_render);
	auto sound = m_sound;

	if (sound)
	{
		//trace("deleting sound (%s): %016llx\n", ToNarrow(m_name), (uintptr_t)sound);

		sound->StopAndForget(false);
		m_sound = nullptr;
	}

	if (m_soundBucket != 0xFF)
	{
		--bucketsUsed[m_soundBucket];
		m_soundBucket = -1;
	}

	// needs to be delayed to when the sound is removed
	//delete m_environmentGroup;
	m_environmentGroup = nullptr;

	auto buffer = m_buffer;

	if (buffer)
	{
		buffer->Release();
		m_buffer = nullptr;
	}
}

#include <scrEngine.h>

static hook::thiscall_stub<void(fwEntity*, rage::fwInteriorLocation&)> _entity_getInteriorLocation([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("EB 19 80 78 10 04 75 05", -0x1F);
#elif IS_RDR3
	return hook::get_pattern("4C 8B C1 75 2A 48 8B 89 ? ? ? ? 48 83 E1 FE", -0x10);
#endif
});

static hook::thiscall_stub<void(fwEntity*, rage::fwInteriorLocation&)> _entity_getAudioInteriorLocation([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("66 89 02 8A 41 28 3C 04 75 09", -0xB);
#elif IS_RDR3
	return hook::get_pattern("83 22 00 83 C8 FF 66 83 4A ? ? 4C 8B C2");
#endif
});

template<int Build>
void MumbleAudioEntity<Build>::PreUpdateService(uint32_t)
{
	std::lock_guard _(m_render);
	if (m_sound)
	{
		auto settings = m_sound->GetRequestedSettings();

		if (m_distance > 0.01f)
		{
			settings->SetVolumeCurveScale(m_distance / 20.0f);
		}
		else
		{
			settings->SetVolumeCurveScale(1.0f);
		}
#ifdef GTA_FIVE
		if (m_overrideVolume >= 0.0f)
		{
			settings->SetVolume(rage::GetDbForLinear(m_overrideVolume));

			if (xbr::IsGameBuildOrGreater<2189>())
			{
				// see initial set around "48 C7 41 68 00 00 80 3F"
				*((char*)settings + 377) &= ~8;
			}
			else
			{
				*((char*)settings + 369) &= ~8;
			}
		}
		else
		{
			settings->SetVolume(rage::GetDbForLinear(1.0f));

			if (xbr::IsGameBuildOrGreater<2189>())
			{
				*((char*)settings + 377) |= 8;
			}
			else
			{
				*((char*)settings + 369) |= 8;
			}
		}
#elif IS_RDR3
		settings->SetVolume(rage::GetDbForLinear(1.0f));
		*((char*)settings + 0x25F) |= 8;
#endif
		if (m_overrideVolume >= 0.0f)
		{
			float levels[4] = { 1.0f,
				1.0f,
				1.0f,
				1.0f };

			auto settings = m_sound->GetRequestedSettings();
			settings->SetQuadSpeakerLevels(levels);

			m_positionForce = { 1.0f,
				1.0f,
				1.0f,
				1.0f };
		}
		else
		{
			m_positionForce = {};
		}

		settings->SetEnvironmentalLoudness(25);
	}

	// debugging position logic
#if 0
	float vec[8];
	if (NativeInvoke::Invoke<0x6C4D0409BA1A2BC2, bool>(NativeInvoke::Invoke<0xD80958FC74E988A6, int>(), &vec))
	{
		m_positionForce = {
			vec[0], vec[2], vec[4]
		};

		// temp temp: use entity's interior location for now
		rage::fwInteriorLocation loc;
		((void (*)(void*, rage::fwInteriorLocation&))0x1408EA678)(*(void**)((*(uint64_t*)0x14247F840) + 8), loc);

		m_environmentGroup->SetPosition(m_positionForce);
		m_environmentGroup->SetInteriorLocation(loc);
	}

	return;
#endif
#ifdef GTA_FIVE
	if (m_environmentGroup)
	{
		m_environmentGroup->SetPosition(m_position);

		if (m_ped)
		{
			rage::fwInteriorLocation interiorLocation;
			_entity_getAudioInteriorLocation(m_ped, interiorLocation);

			// if this isn't an interior, reset the interior pointer thing
			if (interiorLocation.GetInteriorIndex() == 0xFFFF)
			{
				// xbuild: SetInteriorLocation provides a hint as to the voff
				size_t voff = (xbr::IsGameBuildOrGreater<2189>()) ? 8 : 0;

				char* envGroup = (char*)m_environmentGroup;
				*(void**)(envGroup + 240 + voff) = nullptr;
				*(void**)(envGroup + 248 + voff) = nullptr;
			}

			m_environmentGroup->SetInteriorLocation(interiorLocation);
		}
	}
#elif IS_RDR3
	if (m_environmentGroup && m_overrideVolume < 0.0f)
	{
		rage::fwInteriorLocation interiorLocation;

		if (m_ped)
		{
			_entity_getAudioInteriorLocation(m_ped, interiorLocation);

			m_environmentGroup->SetPosition(m_position);

			// Either set to the current Ped's interior location or to invalid
			//
			m_environmentGroup->SetInteriorLocation(interiorLocation);
		}

		// If this isn't an interior, reset the interior pointer thing
		//
		if (interiorLocation.GetInteriorIndex() == 0xFFFF)
		{
			char* envGroup = (char*)m_environmentGroup;
			*(void**)(envGroup + 872) = nullptr;
			*(void**)(envGroup + 880) = nullptr;
		}
	}
#endif
	if (m_poller)
	{
		//m_poller();
	}
}

void MumbleAudioEntityBase::PushAudio(int16_t* pcm, int len)
{
	// Only required if polling happens somewhere other than RageAudioMixThread.
	// std::lock_guard _(m_render);
	if (m_buffer)
	{
		// push audio to the buffer
		m_buffer->PushAudio(pcm, len * sizeof(int16_t) * 1);
	}
}

class MumbleAudioSink : public IMumbleAudioSink
{
public:
	void Process();

	MumbleAudioSink(const std::wstring& name);
	virtual ~MumbleAudioSink() override;

	virtual void SetPollHandler(const std::function<void(int)>& poller) override;
	virtual void SetResetHandler(const std::function<void()>& resetti) override;
	virtual void SetPosition(float position[3], float distance, float overrideVolume) override;
	virtual void PushAudio(int16_t* pcm, int len) override;
	virtual bool IsTalkingAt(float distance) override;

	void Reset();

private:
	std::wstring m_name;
	int m_serverId;

	/// <summary>
	/// Process/MShutdown is executed on MainThrd while PushAudio on NorthAudioUpdate.
	/// </summary>
	std::mutex m_entity_mutex;
	std::shared_ptr<MumbleAudioEntityBase> m_entity;

	alignas(16) rage::Vec3V m_position;
	float m_distance;
	float m_overrideVolume;
	float m_lastOverrideVolume = -1.0f;

	int m_lastSubmixId = -1;
	int m_lastPed = -1;

	std::function<void(int)> m_poller;
	std::function<void()> m_resetti;
};

static std::mutex g_sinksMutex;
static std::set<MumbleAudioSink*> g_sinks;

static std::shared_mutex g_submixMutex;
static std::map<int, int> g_submixIds;

MumbleAudioSink::MumbleAudioSink(const std::wstring& name)
	: m_serverId(-1), m_position(rage::Vec3V{ 0.f, 0.f, 0.f }), m_distance(5.0f), m_overrideVolume(-1.0f), m_name(name)
{
	auto userName = ToNarrow(name);

	if (userName.length() >= 2)
	{
		int serverId = atoi(userName.substr(1, userName.length() - 1).c_str());

		m_serverId = serverId;
	}

	std::lock_guard<std::mutex> _(g_sinksMutex);
	g_sinks.insert(this);
}

MumbleAudioSink::~MumbleAudioSink()
{
	std::lock_guard<std::mutex> _(g_sinksMutex);
	g_sinks.erase(this);
}

void MumbleAudioSink::Reset()
{
	if (m_resetti)
	{
		m_resetti();
	}
}

void MumbleAudioSink::SetPollHandler(const std::function<void(int)>& poller)
{
	m_poller = poller;
}

void MumbleAudioSink::SetResetHandler(const std::function<void()>& resetti)
{
	m_resetti = resetti;
}

bool MumbleAudioSink::IsTalkingAt(float distance)
{
	static float threshold = -80.0f; // anything below -80dB should be unintelligible.

	float userScale = 1.0f;

	if (m_distance > 0.01f)
	{
		userScale = 1.0f / (m_distance / 20.0f);
	}

	return (rage::audCurve::DefaultDistanceAttenuation_CalculateValue(distance * userScale)) > threshold;
}

void MumbleAudioSink::SetPosition(float position[3], float distance, float overrideVolume)
{
	m_position = rage::Vec3V{
		position[0], position[2], position[1]
	};

	m_distance = distance;
	m_overrideVolume = overrideVolume;
}

void MumbleAudioSink::PushAudio(int16_t* pcm, int len)
{
	std::lock_guard _(m_entity_mutex);
	if (m_entity)
	{
		m_entity->PushAudio(pcm, len);
	}
}

template<int Build, typename TFn, typename... TArgs>
static auto MakeMumbleAudioEntityFor(TFn&& fn, TArgs&&... args)
{
	auto entity = std::make_shared<MumbleAudioEntity<Build>>(std::forward<TArgs>(args)...);

	auto entityBase = std::static_pointer_cast<MumbleAudioEntityBase>(entity);
	fn(entityBase.get());
	
	entity->Init();

	return entityBase;
}

template<typename TFn, typename... TArgs>
static auto MakeMumbleAudioEntity(TFn&& fn, TArgs&&... args)
{
	if (xbr::IsGameBuildOrGreater<2802>())
	{
		return MakeMumbleAudioEntityFor<2802>(std::move(fn), std::forward<TArgs>(args)...);
	}

	return MakeMumbleAudioEntityFor<1604>(std::move(fn), std::forward<TArgs>(args)...);
}

void MumbleAudioSink::Process()
{
#ifdef GTA_FIVE
	static auto getByServerId = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_FROM_SERVER_ID"));
	static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(0x43A66C31C68491C0);
#elif IS_RDR3
	static auto getByServerId = fx::ScriptEngine::GetNativeHandler(0x344EA166);
	static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(0x275F255ED201B937);
#endif
	static auto getEntityAddress = fx::ScriptEngine::GetNativeHandler(HashString("GET_ENTITY_ADDRESS"));

#if 0
	if (m_serverId == 0)
	{
		if (!m_entity)
		{
			m_entity = std::make_shared<MumbleAudioEntity>();
			m_entity->Init();
		}

		return;
	}
#endif

	auto playerId = FxNativeInvoke::Invoke<uint32_t>(getByServerId, m_serverId);
	bool isNoPlayer = (playerId > 256 || playerId == -1);

	int submixId = -1;

	{
		std::shared_lock _(g_submixMutex);
		if (auto it = g_submixIds.find(m_serverId); it != g_submixIds.end())
		{
			submixId = it->second;
		}
	}

	// @TODO: Refactor logic to reduce lock scope. Only required at MInit at the moment
	std::lock_guard _(m_entity_mutex);
	if (isNoPlayer && m_overrideVolume <= 0.0f)
	{
		m_entity = {};
		m_lastPed = -1;
	}
	else
	{
		auto ped = (!isNoPlayer) ? FxNativeInvoke::Invoke<int>(getPlayerPed, playerId) : 0;

		// pre-initialize ped
		if (m_lastPed == -1)
		{
			m_lastPed = ped;
		}

		if (!m_entity)
		{
			Reset();

			m_entity = MakeMumbleAudioEntity([this, submixId](MumbleAudioEntityBase* entity)
			{
				entity->SetPoller(m_poller);
				entity->SetSubmixId(submixId);
			}, m_name);

			m_lastSubmixId = submixId;
		}

		if (m_overrideVolume != m_lastOverrideVolume ||
			submixId != m_lastSubmixId ||
			ped != m_lastPed)
		{
			Reset();

			m_lastOverrideVolume = m_overrideVolume;
			m_lastSubmixId = submixId;
			m_lastPed = ped;

			m_entity->MShutdown();
			m_entity->SetSubmixId(submixId);
			m_entity->MInit(m_overrideVolume);
		}

		m_entity->SetPosition((float*)&m_position, m_distance, m_overrideVolume);
		
		if (ped > 0)
		{
			auto address = FxNativeInvoke::Invoke<CPed*>(getEntityAddress, ped);

			m_entity->SetBackingEntity(address);
		}
		else
		{
			m_entity->SetBackingEntity(nullptr);
		}
	}
}

void ProcessAudioSinks()
{
	std::lock_guard<std::mutex> _(g_sinksMutex);

	for (auto sink : g_sinks)
	{
		sink->Process();
	}
}
#ifdef GTA_FIVE
class RageAudioStream : public nui::IAudioStream
{
public:
	RageAudioStream(const nui::AudioStreamParams& params);

	virtual ~RageAudioStream();

	virtual void ProcessPacket(const float** data, int frames, int64_t pts) override;

private:
	bool TryEnsureInitialized();

private:
	rage::audExternalStreamSound* m_sound;

	rage::audReferencedRingBuffer* m_buffer;

	uint8_t* m_bufferData;

	nui::AudioStreamParams m_initParams;

	SwrContext* m_avr;
};

class RageAudioSink : public nui::IAudioSink
{
public:
	virtual std::shared_ptr<nui::IAudioStream> CreateAudioStream(const nui::AudioStreamParams& params) override;
};

RageAudioStream::RageAudioStream(const nui::AudioStreamParams& params)
	: m_initParams(params)
{
	m_sound = nullptr;

	m_avr = swr_alloc();

	// channel layout
	int channelLayout = AV_CH_LAYOUT_MONO;

	if (params.channelLayout != nui::CefChannelLayout::CEF_CHANNEL_LAYOUT_MONO)
	{
		channelLayout = AV_CH_LAYOUT_STEREO;
	}

	// sample format
	AVSampleFormat sampleFormat = AV_SAMPLE_FMT_S16;
	int sampleRate = params.sampleRate;

	m_avr = swr_alloc_set_opts(m_avr, channelLayout, AV_SAMPLE_FMT_S16, 44100, channelLayout, sampleFormat, sampleRate, 0, NULL);

	swr_init(m_avr);
}

RageAudioStream::~RageAudioStream()
{
	if (m_sound)
	{
		m_sound->StopAndForget(false);
	}

	swr_free(&m_avr);
}

void RageAudioStream::ProcessPacket(const float** data, int frames, int64_t pts)
{
	if (TryEnsureInitialized())
	{
		// interleave and normalize the data
		std::vector<int16_t> buffer(frames * m_initParams.channels);
		auto cursor = buffer.data();

		for (int f = 0; f < frames; f++)
		{
			for (int c = 0; c < m_initParams.channels; c++)
			{
				*cursor++ = (int16_t)(data[c][f] * INT16_MAX);
			}
		}

		// resample to 44.1 kHz
		int outSamples = av_rescale_rnd(swr_get_delay(m_avr, m_initParams.sampleRate) + frames, (int)44100, m_initParams.sampleRate, AV_ROUND_UP);

		auto dataRef = buffer.data();

		uint8_t* resampledBytes;
		av_samples_alloc(&resampledBytes, NULL, m_initParams.channels, outSamples, AV_SAMPLE_FMT_S16, 0);
		outSamples = swr_convert(m_avr, &resampledBytes, outSamples, (const uint8_t**)&dataRef, frames);

		// push audio to the buffer
		m_buffer->PushAudio(resampledBytes, outSamples * sizeof(int16_t) * m_initParams.channels);

		// free
		av_freep(&resampledBytes);
	}
}
#endif
static bool audioRunning;
#ifdef GTA_FIVE
bool RageAudioStream::TryEnsureInitialized()
{
	if (m_sound)
	{
		return true;
	}

	if (audioRunning)
	{
		rage::audSoundInitParams initValues;
		
		// set the audio category
		if (!m_initParams.categoryName.empty())
		{
			auto category = rage::g_categoryMgr->GetCategoryPtr(HashString(m_initParams.categoryName.c_str()));

			if (category)
			{
				initValues.SetCategory(category);
			}
		}

		rage::g_frontendAudioEntity->CreateSound_PersistentReference(0xD8CE9439, (rage::audSound**)&m_sound, initValues);

		if (m_sound)
		{
			if (m_initParams.channelLayout == nui::CefChannelLayout::CEF_CHANNEL_LAYOUT_MONO)
			{
				m_initParams.channels = 1;
			}
			else
			{
				m_initParams.channels = 2;
			}

			// have ~1 second of audio buffer room
			auto size = 48000 * sizeof(int16_t) * m_initParams.channels;
			m_bufferData = (uint8_t*)rage::GetAllocator()->Allocate(size, 16, 0);

			m_buffer = new rage::audReferencedRingBuffer();
			m_buffer->SetBuffer(m_bufferData, size);

			m_sound->InitStreamPlayer(m_buffer, m_initParams.channels, 44100);
			m_sound->PrepareAndPlay(nullptr, true, -1, false);

			_updateAudioThread();

			return true;
		}
	}

	return false;
}

std::shared_ptr<nui::IAudioStream> RageAudioSink::CreateAudioStream(const nui::AudioStreamParams& params)
{
	return std::make_shared<RageAudioStream>(params);
}

static RageAudioSink g_audioSink;

DLL_IMPORT void ForceMountDataFile(const std::pair<std::string, std::string>& dataFile);

static uint32_t* g_preferenceArray;

enum AudioPrefs
{
	PREF_SFX_VOLUME = 7,
	PREF_MUSIC_VOLUME = 8,
	PREF_MUSIC_VOLUME_IN_MP = 0x25,
};
#endif
#ifdef GTA_FIVE
static bool (*g_origLoadCategories)(void* a1, const char* why, const char* filename, int a4, int version, bool, void*);

bool LoadCategories(void* a1, const char* why, const char* filename, int a4, int version, bool a6, void* a7)
{
	return g_origLoadCategories(a1, why, "citizen:/platform/audio/config/categories.dat", a4, version, a6, a7);
}
#elif IS_RDR3
static bool (*g_origLoadCategories)(void* a1, int a2, int a3, const char* filename, int version, int a6, int a7, char a8, const char* a9, uint64_t a10, int a11, uint64_t a12, int a13);

bool LoadCategories(void* a1, int a2, int a3, const char* filename, int version, int a6, int a7, char a8, const char* a9, uint64_t a10, int a11, uint64_t a12, int a13)
{
	return g_origLoadCategories(a1, a2, a3, "citizen:/platform/audio/config/categories.dat", version, a6, a7, a8, a9, a10, a11, a12, a13);
}
#endif

static void (*g_origOddFunc)(void*, uint16_t, float, int, int, int, int);
static bool (*g_origaudEnvironmentSound_Init)(void* sound, void* a, void* b, void* params);

static bool audEnvironmentSound_InitStub(char* sound, void* a, void* b, char* params)
{
	auto oldField = params[21] & 0x3F;
	int submixIdx = -1;

	if (oldField >= 32)
	{
		params[21] &= ~0x3F;
		submixIdx = (oldField - 32) + 0x1C;
	}

	bool rv = g_origaudEnvironmentSound_Init(sound, a, b, params);

	if (submixIdx >= 0)
	{
		sound[248] |= 0x80;
		*(int*)(&sound[216]) = submixIdx;
	}

	return rv;
}

static void (*g_origaudMixerDevice_InitClientThread)(void* device, const char* name, uint32_t size);

static void audMixerDevice_InitClientThreadStub(void* device, const char* name, uint32_t size)
{
	return g_origaudMixerDevice_InitClientThread(device, name, size * 3);
}

static bool (*g_orig_audConfig_GetData_uint)(const char*, uint32_t&);

static bool audConfig_GetData_uint(const char* param, uint32_t& out)
{
	if (strcmp(param, "engineSettings_NumBuckets") == 0)
	{
#ifdef GTA_FIVE
		out = 12 + kExtraAudioBuckets;
#elif IS_RDR3
		out = 8 + kExtraAudioBuckets;
#endif
		return true;
	}

	return g_orig_audConfig_GetData_uint(param, out);
}

static HookFunction hookFunction([]()
{
#ifdef GTA_FIVE
	g_preferenceArray = hook::get_address<uint32_t*>(hook::get_pattern("48 8D 15 ? ? ? ? 8D 43 01 83 F8 02 77 2D", 3));
#endif
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern("41 B9 04 00 00 00 C6 44 24 28 01 C7 44 24 20 16 00 00 00 E8", 19);
#elif IS_RDR3
		auto location = hook::get_pattern("E8 ? ? ? ? 84 C0 74 3A 48 8B CB E8 ? ? ? ? 84 C0 74 2E 48 8D 0D");
#endif
		hook::set_call(&g_origLoadCategories, location);
		hook::call(location, LoadCategories);
	}

	{
#ifdef GTA_FIVE
		auto location = hook::get_call(hook::get_pattern("41 B8 00 00 01 00 84 C0 48", -12));
#elif IS_RDR3
		auto location = hook::get_call(hook::get_pattern("41 B8 ? ? ? ? 44 0F 45 44 24", -0x15));
#endif
		MH_Initialize();
		MH_CreateHook(location, audConfig_GetData_uint, (void**)&g_orig_audConfig_GetData_uint);
		MH_EnableHook(location);
	}

	// hook to enable submix index reading

	// add submix value to padding for rage::audEnvironment::UpdateVoiceMetrics
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern("48 8D 54 24 30 89 74 24 20 E8", 9);
#elif IS_RDR3
		auto location = hook::get_pattern("E8 ? ? ? ? 80 8F ? ? ? ? ? F3 0F 10 8D");
#endif
		void* origUpdateVoiceMetrics;
		hook::set_call(&origUpdateVoiceMetrics, location);

		static struct : jitasm::Frontend
		{
			void* origCall;

			virtual void InternalMain() override
			{
#ifdef GTA_FIVE
				test(byte_ptr[rdi + 247], 0x10);	// if ((rdi+247) & 0x10) {
				jz("unsure");
				L("sure");							// sure:
				mov(eax, dword_ptr[rdi + 216]);		//    eax = (rdi + 216)
				cmp(eax, 0x1C);						//    if (eax >= 0x1C) {
				jl("go");
				and(byte_ptr[rdi + 247], ~0x10);    //       (rdi + 247) &= ~0x10
				or (byte_ptr[rdi + 248], 0x80);		//       (rdi + 248) |=  0x80
				jmp("go");							//    }
				L("unsure");						// } else {
				test(byte_ptr[rdi + 248], 0x80);	//    if ((rdi+248) & 0x80) {
				jnz("sure");						//        goto sure;
													//    }
				mov(eax, 0xFFFFFFFF);				//    eax = -1;
				L("go");							// }
				mov(byte_ptr[rdx + 0x6A], al);		// (rdx + 0x6A) = eax
				mov(rax, (uint64_t)origCall);		// return to sender
				jmp(rax);
#elif IS_RDR3
				mov(eax, dword_ptr[rdi + 0x218]);
				cmp(eax, 14);
				jge("go");
				mov(eax, 0xFFFFFFFF);
				L("go");
				mov(byte_ptr[rdx + 0x148], al);
				mov(rax, (uint64_t)origCall);
				jmp(rax);
#endif
			}
		} updateVoiceMetricsStub;

		updateVoiceMetricsStub.origCall = origUpdateVoiceMetrics;

		hook::call(location, updateVoiceMetricsStub.GetCode());
	}

	// intervene in audEnvironment::ComputeVoiceRoutes
	{
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
#ifdef GTA_FIVE
				sub(rsp, 0x28);
				mov(rcx, qword_ptr[rsi]);
				lea(rdx, qword_ptr[rsp + 0x30 + 0x20]);

				mov(rax, (uint64_t)DoVoiceRoute);
				call(rax);

				add(rsp, 0x28);

				mov(rdi, rbx);
				mov(r13, rbx);
#elif IS_RDR3
				push(r14);
				sub(rsp, 0x28);
				mov(rcx, qword_ptr[r15]);
				lea(rdx, qword_ptr[rsp + 0x28 + 0x8 + 0xB0]);
				mov(rax, (uint64_t)DoVoiceRoute);
				call(rax);
				add(rsp, 0x28);
				pop(r14);
				mov(rdi, r14);
				mov(bl, 0x7F);
#endif
				ret();
			}

			static void DoVoiceRoute(uint8_t* voiceData, int* outRoutes)
			{
#ifdef GTA_FIVE
				if (voiceData[0x6A] != 0xFF && voiceData[0x6A] >= 0x1C) // first route we have 'ourselves'
				{
					outRoutes[0] = voiceData[0x6A];
				}
#elif IS_RDR3
				if (voiceData[0x148] != 0xFF && voiceData[0x148] >= 14) // first route we have 'ourselves'
				{
					outRoutes[0] = voiceData[0x148];
					outRoutes[1] = outRoutes[2] = outRoutes[3] = outRoutes[4] = outRoutes[5] = 0xFF;
				}
#endif
			}
		} computeVoiceRoutesStub;

#ifdef GTA_FIVE
		auto location = hook::get_pattern("75 E8 48 8B FB 4C 8B EB 4C 8D", 2);
		hook::nop(location, 6);
#elif IS_RDR3
		auto location = hook::get_pattern("49 8B FE B3 7F 49 8B 07 48 8B");
#endif
		hook::call(location, computeVoiceRoutesStub.GetCode());
	}
#ifdef GTA_FIVE
	// make sure a value that's needed to remove submix flag is set
	{
		auto location = hook::get_pattern<char>("48 8B CB C7 44 24 28 58 CB 00 00 44 88 74 24 20 E8", -0x2C4);
		hook::set_call(&g_origOddFunc, location + 0x2D4);

		MH_Initialize();
		MH_CreateHook(location, audEnvironmentSound_InitStub, (void**)&g_origaudEnvironmentSound_Init);
		MH_EnableHook(location);
	}
#endif

	// triple audio command buffer size
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern("B9 B0 00 00 00 45 8B F0 48 8B FA E8", -0x1C);
#elif IS_RDR3
		auto location = hook::get_pattern("75 EB 89 8B ? ? ? ? 48 89", -0x58);
#endif

		MH_Initialize();
		MH_CreateHook(location, audMixerDevice_InitClientThreadStub, (void**)&g_origaudMixerDevice_InitClientThread);
		MH_EnableHook(location);
	}

	// custom audio poll stuff
	{
#ifdef GTA_FIVE
		auto offset = (xbr::IsGameBuildOrGreater<2372>()) ? -0x14 : -0x11;
		auto location = hook::get_pattern("48 8D 6C 24 30 8B 04 24 0F 29 75 ? 0F 29 7D ? 48 8B D9", offset);
#elif IS_RDR3
		auto location = hook::get_pattern("B8 ? ? ? ? 48 2B E0 4C 8D 6C 24 ? 41 8B 55 00", -0x2F);
#endif

		MH_Initialize();
		MH_CreateHook(location, GenerateFrameHook, (void**)&g_origGenerateFrame);
		MH_EnableHook(location);
	}
});

rage::audDspEffect* MakeRadioFX();

static InitFunction initFunction([]()
{
#if IS_RDR3
	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ADDRESS", [](fx::ScriptContext& context)
	{
		context.SetResult(rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0)));
	});
#endif

	fx::ScriptEngine::RegisterNativeHandler("CREATE_AUDIO_SUBMIX", [](fx::ScriptContext& ctx)
	{
		std::string name = ctx.CheckArgument<const char*>(0);
		
		if (audioRunning)
		{
			static std::map<uint32_t, int> submixesByName;
			auto hash = HashString(name.c_str());

			if (auto it = submixesByName.find(hash); it != submixesByName.end())
			{
				ctx.SetResult(it->second);
				return;
			}

			auto mixer = rage::audDriver::GetMixer();
			if (auto submix = mixer->CreateSubmix(name.c_str(), 6, true); submix)
			{
				int idx = mixer->GetSubmixIndex(submix);
				submixesByName[hash] = idx;

				mixer->FlagThreadCommandBufferReadyToProcess();

				ctx.SetResult(idx);
				return;
			}
		}

		ctx.SetResult(-1);
	});

	fx::ScriptEngine::RegisterNativeHandler("ADD_AUDIO_SUBMIX_OUTPUT", [](fx::ScriptContext& ctx)
	{
		int sourceIdx = ctx.GetArgument<int>(0);
		int destIdx = ctx.GetArgument<int>(1);

		auto mixer = rage::audDriver::GetMixer();
		auto submix = mixer->GetSubmix(sourceIdx);

		if (submix)
		{
			submix->AddOutput(destIdx, true, true);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_AUDIO_SUBMIX_EFFECT_PARAM_INT", [](fx::ScriptContext& ctx)
	{
		int sourceIdx = ctx.GetArgument<int>(0);
		uint32_t effectIdx = ctx.GetArgument<uint32_t>(1);
		uint32_t paramIdx = ctx.GetArgument<uint32_t>(2);
		uint32_t paramValue = ctx.GetArgument<uint32_t>(3);

		auto mixer = rage::audDriver::GetMixer();
		auto submix = mixer->GetSubmix(sourceIdx);

		if (submix)
		{
			submix->SetEffectParam(effectIdx, paramIdx, paramValue);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_AUDIO_SUBMIX_EFFECT_PARAM_FLOAT", [](fx::ScriptContext& ctx)
	{
		int sourceIdx = ctx.GetArgument<int>(0);
		uint32_t effectIdx = ctx.GetArgument<uint32_t>(1);
		uint32_t paramIdx = ctx.GetArgument<uint32_t>(2);
		float paramValue = ctx.GetArgument<float>(3);

		auto mixer = rage::audDriver::GetMixer();
		auto submix = mixer->GetSubmix(sourceIdx);

		if (submix)
		{
			submix->SetEffectParam(effectIdx, paramIdx, paramValue);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_AUDIO_SUBMIX_EFFECT_RADIO_FX", [](fx::ScriptContext& ctx)
	{
		int sourceIdx = ctx.GetArgument<int>(0);
		uint32_t effectIdx = ctx.GetArgument<uint32_t>(1);

		auto mixer = rage::audDriver::GetMixer();
		auto submix = mixer->GetSubmix(sourceIdx);

		if (submix)
		{
			submix->SetEffect(effectIdx, MakeRadioFX());
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_SUBMIX_FOR_SERVER_ID", [](fx::ScriptContext& ctx)
	{
		int idx = ctx.GetArgument<int>(1);

		if (idx < 0 || idx > 40)
		{
			idx = -1;
		}

		std::unique_lock _(g_submixMutex);
		int player = ctx.GetArgument<int>(0);

		if (idx >= 0)
		{
			g_submixIds[player] = idx;
		}
		else
		{
			g_submixIds.erase(player);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_AUDIO_SUBMIX_OUTPUT_VOLUMES", [](fx::ScriptContext& ctx)
	{
		int sourceIdx = ctx.GetArgument<int>(0);
		uint32_t slotIdx = ctx.GetArgument<uint32_t>(1);
		float channel1Volume = ctx.GetArgument<float>(2);
		float channel2Volume = ctx.GetArgument<float>(3);
		float channel3Volume = ctx.GetArgument<float>(4);
		float channel4Volume = ctx.GetArgument<float>(5);
		float channel5Volume = ctx.GetArgument<float>(6);
		float channel6Volume = ctx.GetArgument<float>(7);

		auto mixer = rage::audDriver::GetMixer();
		auto submix = mixer->GetSubmix(sourceIdx);

		if (submix)
		{
			rage::audChannelVoiceVolumes volumes;
			volumes.volumes[0] = channel1Volume;
			volumes.volumes[1] = channel2Volume;
			volumes.volumes[2] = channel3Volume;
			volumes.volumes[3] = channel4Volume;
			volumes.volumes[4] = channel5Volume;
			volumes.volumes[5] = channel6Volume;

			submix->SetOutputVolumes(slotIdx, volumes);
		}
	});

	rage::OnInitFunctionInvoked.Connect([](rage::InitFunctionType type, const rage::InitFunctionData& data)
	{
#ifdef GTA_FIVE
		if (type == rage::InitFunctionType::INIT_CORE && data.funcHash == /*0xE6D408DF*/ 0xF0F5A94D)
#elif IS_RDR3
		if (type == rage::InitFunctionType::INIT_CORE && data.funcHash == 0xE6D408DF)
#endif
		{
#ifdef GTA_FIVE
			std::string packFile;
			std::string soundData;
			std::string wavePack;

			if (xbr::IsGameBuildOrGreater<2372>())
			{
				packFile = "dlcpacks:/mptuner/dlc.rpf";
				soundData = "x64/audio/dlcTuner_sounds.dat";
				wavePack = "x64/audio/sfx/dlc_Tuner_Music";
			}
			else if (xbr::IsGameBuildOrGreater<1734>())
			{
				packFile = "dlcpacks:/mpvinewood/dlc.rpf";
				soundData = "x64/audio/dlcvinewood_sounds.dat";
				wavePack = "x64/audio/sfx/dlc_vinewood";
			}
			else
			{
				packFile = "dlcpacks:/mpchristmas2018/dlc.rpf";
				soundData = "x64/audio/dlcAWXM2018_sounds.dat";
				wavePack = "x64/audio/sfx/dlc_AWXM2018";
			}

			
			rage::fiPackfile* dlcAud = new rage::fiPackfile();
			if (dlcAud->OpenPackfile(packFile.c_str(), true, 3, false))
			{
				dlcAud->Mount("menuAud:/");

				ForceMountDataFile({ "AUDIO_SOUNDDATA", fmt::sprintf("menuAud:/%s", soundData) });
				ForceMountDataFile({ "AUDIO_WAVEPACK", fmt::sprintf("menuaud:/%s", wavePack) });

				audioRunning = true;
			}
#elif IS_RDR3
			audioRunning = true;
#endif
		}
	});

	static NetLibrary* netLibrary;

	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		netLibrary = lib;
	});

	OnGameFrame.Connect([]()
	{
		static ConVar<bool> arenaWarVariable("ui_disableMusicTheme", ConVar_Archive, false);
#ifdef GTA_FIVE
		static ConVar<bool> arenaWarVariableForce("ui_forceMusicTheme", ConVar_Archive, false);
		static ConVar<std::string> musicThemeVariable("ui_selectMusic", ConVar_Archive, "dlc_awxm2018_theme_5_stems");
		static std::string lastSong = musicThemeVariable.GetValue();

		static rage::audSound* g_sound;
		static bool swapSong;
		static bool wasLoading;

		if (audioRunning)
		{
			bool active = nui::HasMainUI() && (!netLibrary || netLibrary->GetConnectionState() == NetLibrary::CS_IDLE) && !arenaWarVariable.GetValue();
			bool viaLoading = false;

			if (launch::IsSDKGuest()) {
				active = false;
			}
			else
			{
				if (!arenaWarVariable.GetValue() && !Instance<ICoreGameInit>::Get()->GetGameLoaded() && Instance<ICoreGameInit>::Get()->HasVariable("noLoadingScreen"))
				{
					if (!wasLoading)
					{
						swapSong = true;
						wasLoading = true;
					}

					active = true;
					viaLoading = true;
				}
				else
				{
					wasLoading = false;
				}

				if (arenaWarVariableForce.GetValue())
				{
					active = true;
				}

				if (ShouldMuteGameAudio())
				{
					active = false;
				}
			}

#if SMTEST
			static uint8_t mySubmix = 0;

			if (!mySubmix)
			{
				auto mixer = rage::audDriver::GetMixer();
				mixer->InitClientThread("RenderThread", 0x8000);
				auto submix = mixer->CreateSubmix("meme", 6, true);
				//auto submix = (rage::audMixerSubmix*)((char*)mixer + 16);
				submix->AddOutput(0, true, true);
				submix->SetEffect(0, MakeRadioFX());
				submix->SetEffectParam(0, HashString("default"), uint32_t(1));
				submix->SetEffectParam(0, 0x1234, 25.f);
				//submix->SetFlag(1, true);

				mixer->ComputeProcessingGraph();
				mixer->FlagThreadCommandBufferReadyToProcess();

				/*rage::audChannelVoiceVolumes volumes;
				volumes.volumes[0] = 500.0f;
				volumes.volumes[1] = 5.0f;
				volumes.volumes[2] = 5.0f;
				volumes.volumes[3] = 5.0f;
				volumes.volumes[4] = 5.0f;
				volumes.volumes[5] = 5.0f;

				submix->SetOutputVolumes(0, volumes);*/

				mySubmix = mixer->GetSubmixIndex(submix);
			}
#endif

			float volume = rage::GetDbForLinear(std::min(std::min({ g_preferenceArray[PREF_MUSIC_VOLUME], g_preferenceArray[PREF_MUSIC_VOLUME_IN_MP], g_preferenceArray[PREF_SFX_VOLUME] }) / 10.0f, 0.75f));
			static float lastVolume = -9999.0f;

			// update volume dynamically (in case it's changed, or wasn't initialized at first)
			if (g_sound && volume != lastVolume)
			{
				g_sound->GetRequestedSettings()->SetVolume(volume);
				lastVolume = volume;
			}

			if (active && !g_sound)
			{
				rage::audSoundInitParams initValues;

				auto musicTheme = musicThemeVariable.GetValue();
				std::string defaultMusicTheme;

				if (xbr::IsGameBuildOrGreater<2372>())
				{
					defaultMusicTheme = "arcade_ch_title_track_1";
				}
				else if (xbr::IsGameBuildOrGreater<1734>())
				{
					defaultMusicTheme = "dlc_vinewood_health_05_slaves_of_fear_aw_rmx";
				}
				else
				{
					defaultMusicTheme = "dlc_awxm2018_theme_5_stems";
				}

				if (musicTheme == "dlc_awxm2018_theme_5_stems")
				{
					musicTheme = defaultMusicTheme;
				}

				rage::g_frontendAudioEntity->CreateSound_PersistentReference(viaLoading ? 0x8D8B11E3 : HashString(musicTheme.c_str()), (rage::audSound**)&g_sound, initValues);

				if (g_sound)
				{
					g_sound->PrepareAndPlay(rage::audWaveSlot::FindWaveSlot(HashString("interactive_music_1")), true, -1, false);
					_updateAudioThread();
				}
				else
				{
					musicThemeVariable.GetHelper()->SetValue("dlc_awxm2018_theme_5_stems");
				}
			}
			else if ((g_sound && (!active || swapSong)) || musicThemeVariable.GetValue() != lastSong)
			{
				if (g_sound)
				{
					g_sound->StopAndForget(false);
					g_sound = nullptr;

					_updateAudioThread();
				}

				lastSong = musicThemeVariable.GetValue();
				swapSong = false;
			}
		}
#elif IS_RDR3 && 0
		static int last_connection_state = -1;
		static bool wait_for_initial_game_init = false;

		if (audioRunning && last_connection_state != NetLibrary::CS_ACTIVE && rage::g_frontendAudioEntity)
		{
			rage::audSound* envelopeSound = *(rage::audSound**)((uintptr_t)rage::g_frontendAudioEntity + 0x358);
			if (!envelopeSound && wait_for_initial_game_init)
			{
				return;
			}
			wait_for_initial_game_init = false;

			if (envelopeSound && arenaWarVariable.GetValue())
			{
				envelopeSound->ActionReleaseRequest(0);
				envelopeSound->StopAndForget(0);
				_updateAudioThread(0);
			}
			else if (!envelopeSound && !arenaWarVariable.GetValue())
			{
				StartMenuMusic(rage::g_frontendAudioEntity, 0, 0);
			}
		}

		if (netLibrary->GetConnectionState() != last_connection_state)
		{
			if (last_connection_state == NetLibrary::CS_CONNECTED || last_connection_state == -1)
			{
				wait_for_initial_game_init = true;
			}

			last_connection_state = netLibrary->GetConnectionState();
		}
#endif
	});

	OnMainGameFrame.Connect([]()
	{
		ProcessAudioSinks();
	});

	OnGetMumbleAudioSink.Connect([](const std::wstring& name, fwRefContainer<IMumbleAudioSink>* sink)
	{
		fwRefContainer<MumbleAudioSink> ref = new MumbleAudioSink(name);
		*sink = ref;
	});

	OnSetMumbleVolume.Connect([](float volume) {
		auto controllerMgr = rage::audCategoryControllerManager::GetInstance();

		if (!controllerMgr)
		{
			return;
		}

		static auto controller = controllerMgr->CreateController(HashString("mumble"));

		if (controller)
		{
#ifdef GTA_FIVE
			*(float*)(&controller[0]) = volume * 2.0f;
			*(float*)(&controller[4]) = 0.0f;
#elif IS_RDR3
			*(float*)(&controller[0x10]) = volume;
#endif
		}
	});

	//nui::SetAudioSink(&g_audioSink);
});

rage::audMixerDevice** rage::audDriver::sm_Mixer;
