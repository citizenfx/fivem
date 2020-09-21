#include <StdInc.h>
#include <CefOverlay.h>

#include <gameSkeleton.h>

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

#include <ScriptEngine.h>

static concurrency::concurrent_queue<std::function<void()>> g_mainQueue;

namespace rage
{
	class audWaveSlot
	{
	public:
		static audWaveSlot* FindWaveSlot(uint32_t hash);
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

		void StopAndForget(void* a1);

		audRequestedSettings* GetRequestedSettings();

	public:
		char pad[141 - 8];
		uint8_t unkBitFlag : 3;
	};

	static hook::cdecl_stub<void(audSound*, void*, bool, int, bool)> _audSound_PrepareAndPlay([]()
	{
		return hook::get_pattern("0F 85 D3 00 00 00 41 83 CB FF", -0x35);
	});

	static hook::cdecl_stub<void(audSound*, void*)> _audSound_StopAndForget([]()
	{
		return hook::get_pattern("74 24 45 0F B6 41 62", -0x10);
	});

	void audSound::PrepareAndPlay(audWaveSlot* a1, bool a2, int a3, bool a4)
	{
		_audSound_PrepareAndPlay(this, a1, a2, a3, a4);
	}

	void audSound::StopAndForget(void* a1)
	{
		_audSound_StopAndForget(this, a1);
	}

	class audReferencedRingBuffer : public sysUseAllocator
	{
	public:
		audReferencedRingBuffer();

		~audReferencedRingBuffer();

		inline void SetBuffer(void* buffer, uint32_t size)
		{
			m_data = buffer;
			m_size = size;
			m_initialized = true;
		}

		uint32_t PushAudio(const void* data, uint32_t size);

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
		DeleteCriticalSection(&m_lock);
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
		*(uint8_t*)(&m_pad[152]) = submix;
	}

	void audSoundInitParams::SetUnk()
	{
		m_pad[155] = 27;
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

	void audRequestedSettings::SetVolume(float vol)
	{
		_audRequestedSettings_SetVolume(this, vol);
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
	return hook::get_pattern("80 A7 10 01 00 00 FC", -0x22);
});

static hook::thiscall_stub<void(naEnvironmentGroup*, const rage::Vec3V& position)> _naEnvironmentGroup_setPosition([]()
{
	return hook::get_pattern("F3 0F 11 41 74 F3 0F 11 49 78 C3", -0x1F);
});

static hook::thiscall_stub<void(naEnvironmentGroup*, rage::fwInteriorLocation)> _naEnvironmentGroup_setInteriorLocation([]()
{
	return hook::get_pattern("3B 91 EC 00 00 00 74 07 80 89", -0x17);
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

class MumbleAudioEntity : public rage::audEntity
{
public:
	MumbleAudioEntity()
		: m_position(rage::Vec3V{ 0.f, 0.f, 0.f }),
		  m_positionForce(rage::Vec3V{ 0.f, 0.f, 0.f }),
		  m_buffer(nullptr),
		  m_sound(nullptr), m_bufferData(nullptr), m_environmentGroup(nullptr), m_distance(5.0f), m_overrideVolume(-1.0f),
		  m_ped(nullptr)
	{
	}

	virtual void Init() override;

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
		m_position = { position[0],
			position[1], position[2] };

		m_distance = distance;
		m_overrideVolume = overrideVolume;
	}

	void SetBackingEntity(CPed* ped)
	{
		m_ped = ped;
	}

	void PushAudio(int16_t* pcm, int len);

private:
	rage::audExternalStreamSound* m_sound;

	alignas(16) rage::Vec3V m_position;
	float m_distance;
	float m_overrideVolume;

	alignas(16) rage::Vec3V m_positionForce;

	rage::audReferencedRingBuffer* m_buffer;

	uint8_t* m_bufferData;

	naEnvironmentGroup* m_environmentGroup;

	CPed* m_ped;
};

void MumbleAudioEntity::Init()
{
	rage::audEntity::Init();

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
	//initValues.SetSubmixIndex(12);

	//CreateSound_PersistentReference(0xD8CE9439, (rage::audSound**)&m_sound, initValues);
	CreateSound_PersistentReference(0x8460F301, (rage::audSound**)&m_sound, initValues);

	if (m_sound)
	{
		// have ~1 second of audio buffer room
		auto size = 48000 * sizeof(int16_t) * 1;
		m_bufferData = (uint8_t*)rage::GetAllocator()->Allocate(size, 16, 0);

		m_buffer = new rage::audReferencedRingBuffer();
		m_buffer->SetBuffer(m_bufferData, size);

		m_sound->InitStreamPlayer(m_buffer, 1, 48000);
		m_sound->PrepareAndPlay(nullptr, true, -1, false);

		_updateAudioThread();
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

		settings->SetEnvironmentalLoudness(60);
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

	virtual void SetPosition(float position[3], float distance, float overrideVolume) override;
	virtual void PushAudio(int16_t* pcm, int len) override;

private:
	int m_serverId;
	std::shared_ptr<MumbleAudioEntity> m_entity;

	alignas(16) rage::Vec3V m_position;
	float m_distance;
	float m_overrideVolume;
};

static std::mutex g_sinksMutex;
static std::set<MumbleAudioSink*> g_sinks;

MumbleAudioSink::MumbleAudioSink(const std::wstring& name)
	: m_serverId(-1), m_position(rage::Vec3V{ 0.f, 0.f, 0.f }), m_distance(5.0f), m_overrideVolume(-1.0f)
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

	if (isNoPlayer && m_overrideVolume <= 0.0f)
	{
		m_entity = {};
	}
	else
	{
		if (!m_entity)
		{
			m_entity = std::make_shared<MumbleAudioEntity>();
			m_entity->Init();
		}

		m_entity->SetPosition((float*)&m_position, m_distance, m_overrideVolume);
		
		auto ped = (!isNoPlayer) ? FxNativeInvoke::Invoke<int>(getPlayerPed, playerId) : 0;

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
		m_sound->StopAndForget(nullptr);
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

static HookFunction hookFunction([]()
{
	g_preferenceArray = hook::get_address<uint32_t*>(hook::get_pattern("48 8D 15 ? ? ? ? 8D 43 01 83 F8 02 77 2D", 3));

	{
		auto location = hook::get_pattern("41 B9 04 00 00 00 C6 44 24 28 01 C7 44 24 20 16 00 00 00 E8", 19);
		hook::set_call(&g_origLoadCategories, location);
		hook::call(location, LoadCategories);
	}
});

static InitFunction initFunction([]()
{
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

			if (arenaWarVariableForce.GetValue())
			{
				active = true;
			}

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
				g_sound->StopAndForget(nullptr);
				g_sound = nullptr;

				_updateAudioThread();

				lastSong = musicThemeVariable.GetValue();
			}
		}

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
			*(float*)(&controller[0]) = volume * 10.0f;
			*(float*)(&controller[4]) = 0.0f;
		}
	});

	//nui::SetAudioSink(&g_audioSink);
});
