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

#include <GameAudioState.h>

#include <CL2LaunchMode.h>

static concurrency::concurrent_queue<std::function<void()>> g_mainQueue;

namespace rage
{
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
		return hook::get_pattern("44 88 4C 24 29 0D 28 00 00 03", -0x1B);
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, int slot, audDspEffect* effect, uint32_t mask)> _audMixerSubmix_SetEffect([]
	{
		return hook::get_pattern("0D 08 00 00 06 89 44 24 20", -0x20);
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, int slot, uint32_t hash, uint32_t value)> _audMixerSubmix_SetEffectParam_int([]
	{
		return hook::get_pattern("44 89 44 24 24 44 89 4C 24 28 0D 10 00 00", -0x16);
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, int slot, uint32_t hash, float value)> _audMixerSubmix_SetEffectParam_float([]
	{
		return hook::get_pattern("F3 0F 11 5C 24 28 25 07 F8 3F 00 44 89", -0x11);
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, int id, bool value)> _audMixerSubmix_SetFlag([]
	{
		return hook::get_pattern("0D 20 00 00 03 89 44 24 20", -0x1B);
	});

	static hook::thiscall_stub<void(audMixerSubmix* self, uint32_t slot, const audChannelVoiceVolumes& volumes)> _audMixerSubmix_SetOutputVolumes([]
	{
		return hook::get_pattern("25 07 F8 3F 00 89 54 24 24 48 8D", -0x11);
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
		uint8_t m_submixes[40][256];
		uint32_t m_numSubmixes;
	};

	static hook::thiscall_stub<audMixerSubmix*(audMixerDevice* self, const char* name, int numOutputChannels, bool a3)> _audMixerDevice_CreateSubmix([]
	{
		return hook::get_pattern("48 C1 E3 08 89 82 00 28 00", -0x17);
	});

	static hook::thiscall_stub<uint8_t(audMixerDevice* self, audMixerSubmix* submix)> _audMixerDevice_GetSubmixIndex([]
	{
		return hook::get_pattern("83 C8 FF C3 48 2B D1", -0x5);
	});

	static hook::thiscall_stub<void(audMixerDevice* self)> _audMixerDevice_ComputeProcessingGraph([]
	{
		return hook::get_pattern("48 63 83 00 28 00 00 48 8B CB 41 BD", -0x28);
	});

	static hook::thiscall_stub<void(audMixerDevice* self, uint32_t)> _audMixerDevice_FlagThreadCommandBufferReadyToProcess([]
	{
		return hook::get_pattern("48 8B 81 68 F2 00 00 4E 8B", -0x25);
	});

	static hook::thiscall_stub<void(audMixerDevice* self, const char*, uint32_t)> _audMixerDevice_InitClientThread([]
	{
		return hook::get_pattern("B9 B0 00 00 00 45 8B F0 48 8B FA E8", -0x1C);
	});

	audMixerSubmix* audMixerDevice::CreateSubmix(const char* name, int numOutputChannels, bool a3)
	{
		return _audMixerDevice_CreateSubmix(this, name, numOutputChannels, a3);
	}

	uint8_t audMixerDevice::GetSubmixIndex(audMixerSubmix* submix)
	{
		return _audMixerDevice_GetSubmixIndex(this, submix);
	}

	void audMixerDevice::ComputeProcessingGraph()
	{
		return _audMixerDevice_ComputeProcessingGraph(this);
	}

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
		virtual ~audSound() = 0;

		virtual void m_8() = 0;

		virtual void m_10() = 0;

		virtual void m_18() = 0;

		virtual void Init() = 0;

		virtual void m_28() = 0;

		void PrepareAndPlay(audWaveSlot* waveSlot, bool a2, int a3, bool a4);

		void StopAndForget(bool a1);

		audRequestedSettings* GetRequestedSettings();

	public:
		char pad[141 - 8];
		uint8_t unkBitFlag : 3;
	};

	static hook::cdecl_stub<void(audSound*, void*, bool, int, bool)> _audSound_PrepareAndPlay([]()
	{
		return hook::get_pattern("0F 85 ? 00 00 00 41 83 CB FF 45 33 C0", -0x35);
	});

	static hook::cdecl_stub<void(audSound*, bool)> _audSound_StopAndForget([]()
	{
		return hook::get_pattern("74 24 45 0F B6 41 62", -0x10);
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
		return hook::get_pattern("8B 71 08 48 8B 29 2B F0 48 8B D9 41 8B", -0x1B);
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
		return hook::get_pattern("F0 FF 42 50 48 89 11 44 89 41", -0x2A);
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
		return hook::get_call(hook::get_pattern("48 8B CB BA EA 75 96 D5 E8", 8));
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
		uint8_t m_pad[0xB0];
	};

	void audSoundInitParams::SetCategory(rage::audCategory* category)
	{
		*(audCategory**)(&m_pad[88]) = category;
	}

	void audSoundInitParams::SetEnvironmentGroup(rage::audEnvironmentGroupInterface* environmentGroup)
	{
		*(audEnvironmentGroupInterface**)(&m_pad[96]) = environmentGroup;
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
		*(float*)(&m_pad[48]) = volume;
	}

	void audSoundInitParams::SetTracker(audTracker* parent)
	{
		*(audTracker**)(&m_pad[72]) = parent;
	}

	void audSoundInitParams::SetPositional(bool positional)
	{
		if (positional)
		{
			m_pad[158] |= 1;
		}
		else
		{
			m_pad[158] &= ~1;
		}
	}

	void audSoundInitParams::SetSubmixIndex(uint8_t submix)
	{
		m_pad[155] = (submix - 0x1C) | 0x20;
		// this field does really weird stuff in game
		//*(uint8_t*)(&m_pad[0x98]) = submix;
		//*(uint16_t*)(&m_pad[0x9B]) = 3;
	}

	void audSoundInitParams::SetUnk()
	{
		m_pad[155] = 27;
	}

	void audSoundInitParams::SetAllocationBucket(uint8_t bucket)
	{
		m_pad[154] = bucket;
	}

	static hook::cdecl_stub<void(rage::audSoundInitParams*)> _audSoundInitParams_ctor([]()
	{
		return hook::get_pattern("0F 57 C0 48 8D 41 10 BA 03 00 00 00");
	});

	static uint8_t* initParamVal;

	audSoundInitParams::audSoundInitParams()
	{
		_audSoundInitParams_ctor(this);

		m_pad[0x9A] = *initParamVal;
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
		return hook::get_pattern("F3 0F 11 8C D1 90 00 00 00", -0x11);
	});

	static hook::thiscall_stub<void(audRequestedSettings*, float)> _audRequestedSettings_SetVolumeCurveScale([]()
	{
		return hook::get_pattern("F3 0F 11 8C D1 A8 00 00 00", -0x11);
	});

	static hook::thiscall_stub<void(audRequestedSettings*, uint8_t)> _audRequestedSettings_SetEnvironmentalLoudness([]()
	{
		return hook::get_pattern("42 88 94 C1 B1 00 00 00", -0x11);
	});

	static hook::thiscall_stub<void(audRequestedSettings*, uint8_t)> _audRequestedSettings_SetSpeakerMask([]()
	{
		return hook::get_pattern("42 88 94 C1 B0 00 00 00", -0x11);
	});

	static hook::thiscall_stub<void(audRequestedSettings*, float, float)> _audRequestedSettings_SetSourceEffectMix([]()
	{
		return hook::get_pattern("F3 0F 11 8C D1 98 00 00 00", -0x11);
	});

	static hook::thiscall_stub<void(audRequestedSettings*, float[4])> _audRequestedSettings_SetQuadSpeakerLevels([]()
	{
		return hook::get_pattern("B8 00 80 00 00 66 09 84", -0x61);
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

	class audEntity
	{
	public:
		audEntity();

		virtual ~audEntity();

		virtual void Init();

		virtual void Shutdown();

		virtual void StopAllSounds(bool);

		virtual void PreUpdateService(uint32_t)
		{
		
		}

		virtual void PostUpdate()
		{
		
		}

		virtual void UpdateSound(rage::audSound*, rage::audRequestedSettings*, uint32_t)
		{
		
		}

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

		virtual rage::Vec3V GetPosition()
		{
			return {0.f, 0.f, 0.f, 0.f};
		}

		virtual rage::audOrientation GetOrientation()
		{
			return { 0.f, 0.f };
		}

		virtual uint32_t InitializeEntityVariables()
		{
			m_0A = -1;
			return -1;
		}

		virtual void* GetOwningEntity()
		{
			return nullptr;
		}

		void CreateSound_PersistentReference(const char* name, audSound** outSound, const audSoundInitParams& params);

		void CreateSound_PersistentReference(uint32_t nameHash, audSound** outSound, const audSoundInitParams& params);

	private:
		uint16_t m_entityId{
			0xffff
		};

		uint16_t m_0A{
			0xffff
		};
	};

	audEntity::audEntity()
	{
		
	}

	audEntity::~audEntity()
	{
		Shutdown();
	}

	static hook::cdecl_stub<void(audEntity*, const char*, audSound**, const audSoundInitParams&)> _audEntity_CreateSound_PersistentReference_char([]()
	{
		return hook::get_call(hook::get_pattern("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0x14));
	});

	static hook::cdecl_stub<void(audEntity*, uint32_t, audSound**, const audSoundInitParams&)> _audEntity_CreateSound_PersistentReference_uint([]()
	{
		return (void*)hook::get_call(hook::get_call(hook::get_pattern<char>("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0x14)) + 0x3F);
	});

	void audEntity::CreateSound_PersistentReference(const char* name, audSound** outSound, const audSoundInitParams& params)
	{
		return _audEntity_CreateSound_PersistentReference_char(this, name, outSound, params);
	}

	void audEntity::CreateSound_PersistentReference(uint32_t nameHash, audSound** outSound, const audSoundInitParams& params)
	{
		return _audEntity_CreateSound_PersistentReference_uint(this, nameHash, outSound, params);
	}

	static hook::thiscall_stub<void(audEntity*)> rage__audEntity__Init([]()
	{
		return hook::get_call(hook::get_pattern("48 81 EC C0 00 00 00 48 8B D9 E8 ? ? ? ? 45 33 ED", 10));
	});

	static hook::thiscall_stub<void(audEntity*)> rage__audEntity__Shutdown([]()
	{
		return hook::get_call(hook::get_pattern("48 83 EC 20 48 8B F9 E8 ? ? ? ? 33 ED", 7));
	});

	static hook::thiscall_stub<void(audEntity*, bool)> rage__audEntity__StopAllSounds([]()
	{
		return hook::get_call(hook::get_pattern("0F 82 67 FF FF FF E8 ? ? ? ? 84 C0", 24));
	});

	void audEntity::Init()
	{
		rage__audEntity__Init(this);
	}

	void audEntity::Shutdown()
	{
		rage__audEntity__Shutdown(this);
	}

	void audEntity::StopAllSounds(bool a)
	{
		rage__audEntity__StopAllSounds(this, a);
	}

	audEntity* g_frontendAudioEntity;

	audCategoryManager* g_categoryMgr;

	class audCategoryControllerManager
	{
	public:
		char* CreateController(uint32_t hash);

		static audCategoryControllerManager* GetInstance();
	};

	audCategoryControllerManager* audCategoryControllerManager::GetInstance()
	{
		static auto patternRef = hook::get_address<audCategoryControllerManager**>(hook::get_pattern("45 33 C0 BA 90 1C E2 44 E8", -4));

		return *patternRef;
	}

	static hook::thiscall_stub<char*(audCategoryControllerManager*, uint32_t)> _audCategoryControllerManager_CreateController([]()
	{
		return hook::get_call(hook::get_pattern("45 33 C0 BA 90 1C E2 44 E8", 8));
	});

	char* audCategoryControllerManager::CreateController(uint32_t hash)
	{
		return _audCategoryControllerManager_CreateController(this, hash);
	}

	static HookFunction hookFunction([]()
	{
		g_frontendAudioEntity = hook::get_address<audEntity*>(hook::get_pattern("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0xC));

		g_categoryMgr = hook::get_address<audCategoryManager*>(hook::get_pattern("48 8B CB BA EA 75 96 D5 E8", -4));

		initParamVal = hook::get_address<uint8_t*>(hook::get_pattern("BA 11 CC 23 C3 E8 ? ? ? ? 48 8D", 0x16));

		audDriver::sm_Mixer = hook::get_address<audMixerDevice**>(hook::get_pattern("75 64 44 0F B7 45 06 48 8B 0D", 10));
	});

	static hook::cdecl_stub<audWaveSlot*(uint32_t)> _findWaveSlot([]()
	{
		return hook::get_call(hook::get_pattern("B9 A1 C7 05 92 E8", 5));
	});

	audWaveSlot* audWaveSlot::FindWaveSlot(uint32_t hash)
	{
		return _findWaveSlot(hash);
	}

	static hook::cdecl_stub<float(float)> _linearToDb([]()
	{
		return hook::get_pattern("8B 4C 24 08 8B C1 81 E1 FF FF 7F 00", -0x14);
	});

	float GetDbForLinear(float x)
	{
		return _linearToDb(x);
	}

	static uint64_t* _settingsBase;
	static uint32_t* _settingsIdx;

	audRequestedSettings* audSound::GetRequestedSettings()
	{
		char* v4 = (char*)this;

		audRequestedSettings* v5 = nullptr;
		uint8_t v7 = *(unsigned __int8*)(v4 + 128);
		if (v7 != 255)
			v5 = (audRequestedSettings*)(*(uint64_t*)(13520i64 * *(unsigned __int8*)(v4 + 98) + *_settingsBase + 13504)
				 + (unsigned int)(size_t(v7) * *_settingsIdx));

		return v5;
	}

	static HookFunction hfRs([]()
	{
		auto location = hook::get_pattern<char>("74 23 0F B6 48 62 0F AF 15");
		_settingsIdx = hook::get_address<uint32_t*>(location + 9);
		_settingsBase = hook::get_address<uint64_t*>(location + 16);
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
	return hook::get_pattern("F6 04 02 01 74 0A 8A 05", -0x24);
});

static hook::thiscall_stub<void(naEnvironmentGroup*, void* a2, float a3, int a4, int a5, float a6, int a7)> _naEnvironmentGroup_init([]()
{
	return hook::get_pattern("80 A7 ? 01 00 00 FC F3 0F 10", -0x22);
});

static hook::thiscall_stub<void(naEnvironmentGroup*, const rage::Vec3V& position)> _naEnvironmentGroup_setPosition([]()
{
	return hook::get_pattern("F3 0F 11 41 74 F3 0F 11 49 78 C3", -0x1F);
});

static hook::thiscall_stub<void(naEnvironmentGroup*, rage::fwInteriorLocation)> _naEnvironmentGroup_setInteriorLocation([]()
{
	return hook::get_pattern("3B 91 ? 00 00 00 74 07 80 89", -0x17);
});

naEnvironmentGroup* naEnvironmentGroup::Create()
{
	return _naEnvironmentGroup_create();
}

void naEnvironmentGroup::Init(void* a2, float a3, int a4, int a5, float a6, int a7)
{
	_naEnvironmentGroup_init(this, a2, a3, a4, a5, a6, a7);
}

void naEnvironmentGroup::SetPosition(const rage::Vec3V& position)
{
	_naEnvironmentGroup_setPosition(this, position);
}

void naEnvironmentGroup::SetInteriorLocation(rage::fwInteriorLocation location)
{
	_naEnvironmentGroup_setInteriorLocation(this, location);
}

static hook::cdecl_stub<void()> _updateAudioThread([]()
{
	return hook::get_pattern("40 0F 95 C7 40 84 FF 74 05", -0x14);
});

extern "C"
{
#include <libswresample/swresample.h>
};

class MumbleAudioEntity : public rage::audEntity, public std::enable_shared_from_this<MumbleAudioEntity>
{
public:
	MumbleAudioEntity(const std::wstring& name)
		: m_position(rage::Vec3V{ 0.f, 0.f, 0.f }),
		  m_positionForce(rage::Vec3V{ 0.f, 0.f, 0.f }),
		  m_buffer(nullptr),
		  m_sound(nullptr), m_bufferData(nullptr), m_environmentGroup(nullptr), m_distance(5.0f), m_overrideVolume(-1.0f),
		  m_ped(nullptr),
		  m_name(name)
	{
	}

	virtual ~MumbleAudioEntity() override;

	virtual void Init() override;

	virtual void Shutdown() override;

	void MInit();
	
	void MShutdown();

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

	void PushAudio(int16_t* pcm, int len);

	void SetPoller(const std::function<void(int)>& poller)
	{
		m_poller = poller;
	}

	void SetSubmixId(int id)
	{
		m_submixId = id;
	}

	void Poll(int samples);

private:
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


static void (*g_origGenerateFrame)(rage::audStreamPlayer* self);
static std::shared_mutex g_customEntriesLock;

static std::map<int, std::weak_ptr<MumbleAudioEntity>> g_customEntries;

void GenerateFrameHook(rage::audStreamPlayer* self)
{
	if (self->ringBuffer)
	{
		auto buffer = self->ringBuffer;
		if (auto idx = buffer->GetCustomMode(); idx >= 0)
		{
			std::shared_ptr<MumbleAudioEntity> entity;

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

MumbleAudioEntity::~MumbleAudioEntity()
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

void MumbleAudioEntity::Init()
{
	rage::audEntity::Init();

	MInit();
}

void MumbleAudioEntity::Shutdown()
{
	MShutdown();

	rage::audEntity::Shutdown();
}

void MumbleAudioEntity::Poll(int samples)
{
	if (m_poller)
	{
		m_poller(samples);
	}
}

static constexpr int kExtraAudioBuckets = 4;
static uint32_t bucketsUsed[kExtraAudioBuckets];

void MumbleAudioEntity::MInit()
{
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

	initValues.SetPositional(true);

	initValues.SetEnvironmentGroup(m_environmentGroup);

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

	initValues.SetAllocationBucket(12 + m_soundBucket);

	//CreateSound_PersistentReference(0xD8CE9439, (rage::audSound**)&m_sound, initValues);
	CreateSound_PersistentReference(0x8460F301, (rage::audSound**)&m_sound, initValues);

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

void MumbleAudioEntity::MShutdown()
{
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
	return hook::get_pattern("EB 19 80 78 10 04 75 05", -0x1F);
});

static hook::thiscall_stub<void(fwEntity*, rage::fwInteriorLocation&)> _entity_getAudioInteriorLocation([]()
{
	return hook::get_pattern("66 89 02 8A 41 28 3C 04 75 09", -0xB);
});

void MumbleAudioEntity::PreUpdateService(uint32_t)
{
	if (m_sound)
	{
		auto settings = m_sound->GetRequestedSettings();

		if (m_distance > 0.01f)
		{
			settings->SetVolumeCurveScale(m_distance / 10.0f);
		}
		else
		{
			settings->SetVolumeCurveScale(1.0f);
		}
		
		if (m_overrideVolume >= 0.0f)
		{
			settings->SetVolume(rage::GetDbForLinear(m_overrideVolume));
			*((char*)settings + 369) &= ~8;
		}
		else
		{
			settings->SetVolume(rage::GetDbForLinear(1.0f));
			*((char*)settings + 369) |= 8;
		}

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
				char* envGroup = (char*)m_environmentGroup;
				*(void**)(envGroup + 240) = nullptr;
				*(void**)(envGroup + 248) = nullptr;
			}

			m_environmentGroup->SetInteriorLocation(interiorLocation);
		}
	}

	if (m_poller)
	{
		//m_poller();
	}
}

void MumbleAudioEntity::PushAudio(int16_t* pcm, int len)
{
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
	virtual void SetPosition(float position[3], float distance, float overrideVolume) override;
	virtual void PushAudio(int16_t* pcm, int len) override;

private:
	std::wstring m_name;
	int m_serverId;
	std::shared_ptr<MumbleAudioEntity> m_entity;

	alignas(16) rage::Vec3V m_position;
	float m_distance;
	float m_overrideVolume;
	float m_lastOverrideVolume = -1.0f;

	int m_lastSubmixId = -1;
	int m_lastPed = -1;

	std::function<void(int)> m_poller;
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

void MumbleAudioSink::SetPollHandler(const std::function<void(int)>& poller)
{
	m_poller = poller;
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
	if (m_entity)
	{
		m_entity->PushAudio(pcm, len);
	}
}

class FxNativeInvoke
{
private:
	static inline void Invoke(fx::ScriptContext& cxt, const boost::optional<fx::TNativeHandler>& handler)
	{
		(*handler)(cxt);
	}

public:
	template<typename R, typename... Args>
	static inline R Invoke(const boost::optional<fx::TNativeHandler>& handler, Args... args)
	{
		fx::ScriptContextBuffer cxt;

		pass{ ([&]()
		{
			cxt.Push(args);
		}(),
		1)... };

		Invoke(cxt, handler);

		return cxt.GetResult<R>();
	}
};

void MumbleAudioSink::Process()
{
	static auto getByServerId = fx::ScriptEngine::GetNativeHandler(HashString("GET_PLAYER_FROM_SERVER_ID"));
	static auto getPlayerPed = fx::ScriptEngine::GetNativeHandler(0x43A66C31C68491C0);
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
			m_entity = std::make_shared<MumbleAudioEntity>(m_name);
			m_entity->SetPoller(m_poller);
			m_entity->SetSubmixId(submixId);
			m_entity->Init();

			m_lastSubmixId = submixId;
		}

		if (m_overrideVolume != m_lastOverrideVolume ||
			submixId != m_lastSubmixId ||
			ped != m_lastPed)
		{
			m_lastOverrideVolume = m_overrideVolume;
			m_lastSubmixId = submixId;
			m_lastPed = ped;

			m_entity->MShutdown();
			m_entity->SetSubmixId(submixId);
			m_entity->MInit();
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

static bool audioRunning;

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

static bool (*g_origLoadCategories)(void* a1, const char* why, const char* filename, int a4, int version, bool, void*);

bool LoadCategories(void* a1, const char* why, const char* filename, int a4, int version, bool a6, void* a7)
{
	return g_origLoadCategories(a1, why, "citizen:/platform/audio/config/categories.dat", a4, version, a6, a7);
}

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
		out = 12 + kExtraAudioBuckets;
		return true;
	}

	return g_orig_audConfig_GetData_uint(param, out);
}

static HookFunction hookFunction([]()
{
	g_preferenceArray = hook::get_address<uint32_t*>(hook::get_pattern("48 8D 15 ? ? ? ? 8D 43 01 83 F8 02 77 2D", 3));

	{
		auto location = hook::get_pattern("41 B9 04 00 00 00 C6 44 24 28 01 C7 44 24 20 16 00 00 00 E8", 19);
		hook::set_call(&g_origLoadCategories, location);
		hook::call(location, LoadCategories);
	}

	{
		auto location = hook::get_call(hook::get_pattern("41 B8 00 00 01 00 84 C0 48", -12));
		MH_Initialize();
		MH_CreateHook(location, audConfig_GetData_uint, (void**)&g_orig_audConfig_GetData_uint);
		MH_EnableHook(location);
	}

	// hook to enable submix index reading

	// add submix value to padding for rage::audEnvironment::UpdateVoiceMetrics
	{
		auto location = hook::get_pattern("48 8D 54 24 30 89 74 24 20 E8", 9);
		void* origUpdateVoiceMetrics;
		hook::set_call(&origUpdateVoiceMetrics, location);

		static struct : jitasm::Frontend
		{
			void* origCall;

			virtual void InternalMain() override
			{
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
				sub(rsp, 0x28);
				mov(rcx, qword_ptr[rsi]);
				lea(rdx, qword_ptr[rsp + 0x30 + 0x20]);

				mov(rax, (uint64_t)DoVoiceRoute);
				call(rax);

				add(rsp, 0x28);

				mov(rdi, rbx);
				mov(r13, rbx);

				ret();
			}

			static void DoVoiceRoute(uint8_t* voiceData, int* outRoutes)
			{
				if (voiceData[0x6A] != 0xFF && voiceData[0x6A] >= 0x1C) // first route we have 'ourselves'
				{
					outRoutes[0] = voiceData[0x6A];
				}
			}
		} computeVoiceRoutesStub;

		auto location = hook::get_pattern("75 E8 48 8B FB 4C 8B EB 4C 8D", 2);
		hook::nop(location, 6);
		hook::call(location, computeVoiceRoutesStub.GetCode());
	}

	// make sure a value that's needed to remove submix flag is set
	{
		auto location = hook::get_pattern<char>("48 8B CB C7 44 24 28 58 CB 00 00 44 88 74 24 20 E8", -0x2C4);
		hook::set_call(&g_origOddFunc, location + 0x2D4);

		MH_Initialize();
		MH_CreateHook(location, audEnvironmentSound_InitStub, (void**)&g_origaudEnvironmentSound_Init);
		MH_EnableHook(location);
	}

	// triple audio command buffer size
	{
		auto location = hook::get_pattern("B9 B0 00 00 00 45 8B F0 48 8B FA E8", -0x1C);

		MH_Initialize();
		MH_CreateHook(location, audMixerDevice_InitClientThreadStub, (void**)&g_origaudMixerDevice_InitClientThread);
		MH_EnableHook(location);
	}

	// custom audio poll stuff
	{
		auto location = hook::get_pattern("48 8D 6C 24 30 8B 04 24 0F 29 75 30 0F 29 7D", -0x11);

		MH_Initialize();
		MH_CreateHook(location, GenerateFrameHook, (void**)&g_origGenerateFrame);
		MH_EnableHook(location);
	}
});

rage::audDspEffect* MakeRadioFX();

static InitFunction initFunction([]()
{
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

	rage::OnInitFunctionInvoked.Connect([](rage::InitFunctionType type, const rage::InitFunctionData& data)
	{
		if (type == rage::InitFunctionType::INIT_CORE && data.funcHash == /*0xE6D408DF*/0xF0F5A94D)
		{
			rage::fiPackfile* xm18 = new rage::fiPackfile();
			if (xm18->OpenPackfile("dlcpacks:/mpchristmas2018/dlc.rpf", true, 3, false))
			{
				xm18->Mount("xm18:/");

				ForceMountDataFile({ "AUDIO_SOUNDDATA", "xm18:/x64/audio/dlcAWXM2018_sounds.dat" });
				ForceMountDataFile({ "AUDIO_WAVEPACK", "xm18:/x64/audio/sfx/dlc_AWXM2018" });

				audioRunning = true;
			}
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
		static ConVar<bool> arenaWarVariableForce("ui_forceMusicTheme", ConVar_Archive, false);
		static ConVar<std::string> musicThemeVariable("ui_selectMusic", ConVar_Archive, "dlc_awxm2018_theme_5_stems");
		static std::string lastSong = musicThemeVariable.GetValue();

		static rage::audSound* g_sound;

		if (audioRunning)
		{
			bool active = nui::HasMainUI() && (!netLibrary || netLibrary->GetConnectionState() == NetLibrary::CS_IDLE) && !arenaWarVariable.GetValue();

			if (launch::IsSDKGuest()) {
				active = false;
			}
			else
			{
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

			if (active && !g_sound)
			{
				rage::audSoundInitParams initValues;

				float volume = rage::GetDbForLinear(std::min(std::min({ g_preferenceArray[PREF_MUSIC_VOLUME], g_preferenceArray[PREF_MUSIC_VOLUME_IN_MP], g_preferenceArray[PREF_SFX_VOLUME] }) / 10.0f, 0.75f));
				initValues.SetVolume(volume);

				rage::g_frontendAudioEntity->CreateSound_PersistentReference(HashString(musicThemeVariable.GetValue().c_str()), (rage::audSound**)&g_sound, initValues);

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
			else if ((g_sound && !active) || musicThemeVariable.GetValue() != lastSong)
			{
				g_sound->StopAndForget(false);
				g_sound = nullptr;

				_updateAudioThread();

				lastSong = musicThemeVariable.GetValue();
			}
		}
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
			*(float*)(&controller[0]) = volume * 2.0f;
			*(float*)(&controller[4]) = 0.0f;
		}
	});

	//nui::SetAudioSink(&g_audioSink);
});

rage::audMixerDevice** rage::audDriver::sm_Mixer;
