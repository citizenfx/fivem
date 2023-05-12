#include <StdInc.h>
#include <Hooking.h>

#include <CoreConsole.h>
#include <nutsnbolts.h>

#include <CrossBuildRuntime.h>

#include <gameSkeleton.h>
#include <ICoreGameInit.h>

#include <MinHook.h>

static bool* audioNotFocused;
#ifdef GTA_FIVE
static int* muteOnFocusLoss;
#elif IS_RDR3
static bool muteOnFocusLoss = false;
#endif

bool DLL_EXPORT ShouldMuteGameAudio()
{
#if GTA_FIVE
	return *audioNotFocused && *muteOnFocusLoss;
#elif IS_RDR3
	return !(*audioNotFocused) && muteOnFocusLoss;
#endif
}

bool g_audUseFrameLimiterConVar;

namespace rage
{
bool* g_audUseFrameLimiter;

struct wavePlayerStruct
{
	int wavePlayerIndex;
	int wavePlayerState;
	int wavePlayerAreStatesEqual; // +0x1A == +0x1C
}; // size = 12 bytes

#ifdef GTA_FIVE
template<int Build>
#endif
struct audWavePlayer_members
{
	uint16_t unk_8;
#ifdef GTA_FIVE
	char _pad[(Build >= 2189) ? 16 : 10];
#elif IS_RDR3
	char _pad[10];
#endif
	uint16_t unk_1A;
	uint16_t unk_1C;

	// struct cont'd below, but not needed
};

struct audWavePlayer
{
	virtual void GenerateFrame(void) = 0;
	virtual void SkipFrame(void) = 0;
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void StartPhys() = 0;
	virtual void StopPhys() = 0;
	virtual void StartFrame() = 0;
	virtual void EndFrame() = 0;
	virtual void SetParam() = 0;
	virtual void SetParam_() = 0;
	virtual void nullsub_17() = 0;  // HandleCustomCommandPacket() = 0;
	virtual void GetHeadroom() = 0;
	virtual void GetLengthSamples() = 0;
	virtual bool IsLooping(void) = 0;
	virtual unsigned int GetPlayPositionSamples(void) = 0;
	virtual bool IsFinished(void) = 0;
	virtual bool HasStartedPlayback(void) = 0;
	virtual int64_t ProcessSyncSignal(void* syncSignal) = 0;
	virtual void Shutdown() = 0;
	virtual uint16_t GetNumberOfChannels() = 0;
	virtual void DESTROY() = 0;
	virtual void sub_141277F88() = 0;

private:
#ifdef GTA_FIVE
	uint8_t members[sizeof(audWavePlayer_members<2189>)];
#elif IS_RDR3
	uint8_t members[sizeof(audWavePlayer_members)];
#endif

public:
#ifdef GTA_FIVE
	template<int Build>
	inline auto& GetMembers()
	{
		return *(audWavePlayer_members<Build>*)&members[0];
	}
#elif IS_RDR3
	inline auto& GetMembers()
	{
		return *(audWavePlayer_members*)&members[0];
	}
#endif

};

struct audMixerDevice
{
	// Offsets are the same 1604-2545
	inline int GetMaxWavePlayers()
	{
#ifdef GTA_FIVE
		return *(int*)(((uintptr_t)this) + 0xEA08);
#elif IS_RDR3
		return *(int*)(((uintptr_t)this) + 0x1E8B8);
#endif
	}

	inline int GetWavePlayerSize()
	{
#ifdef GTA_FIVE
		return *(int*)(((uintptr_t)this) + 0xEA0C);
#elif IS_RDR3
		return *(int*)(((uintptr_t)this) + 0x1E8BC);
#endif
	}

	inline int* GetRefArray()
	{
#ifdef GTA_FIVE
		return (int*)(((uintptr_t)this) + 0xDE00);
#elif IS_RDR3
		return (int*)(((uintptr_t)this) + 0x1DCB0);
#endif
	}

	inline audWavePlayer* GetWavePlayerByIndex(int index)
	{
#ifdef GTA_FIVE
		uintptr_t wavePlayerArrayStart = *(uintptr_t*)(((uintptr_t)this) + 0xEA00);
#elif IS_RDR3
		uintptr_t wavePlayerArrayStart = *(uintptr_t*)(((uintptr_t)this) + 0x1E8B0);
#endif
		wavePlayerArrayStart += (index * GetWavePlayerSize());
		return (audWavePlayer*)wavePlayerArrayStart;
	}

};

static hook::thiscall_stub<void*(rage::audMixerDevice*, int)> audMixerDevice__FreePcmSourceSlot([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("8B CE E8 ? ? ? ? E9 ? 00 00 00 83 FF FF", 2));
#elif IS_RDR3
	return hook::get_pattern("8B DA 48 8B 89 ? ? ? ? 44 0F AF C2 49 03 C8", -0xC);
#endif
});

namespace audDriver
{
	inline int64_t* m_VoiceManager;
	inline char** sm_Mixer = 0;
}
}
#define BYTEn(x, n) (*((unsigned char*)&(x) + n))
#define BYTE2(x) BYTEn(x, 2)

#ifdef GTA_FIVE
template<int Build>
#endif
static void rage__audMixerDevice__GeneratePcm(rage::audMixerDevice* thisptr)
{
	int maxWavePlayers = thisptr->GetMaxWavePlayers();
	int* refArray = thisptr->GetRefArray();
	int numActivePlayers = 0;

	// original size is 64, problem is that numActivePlayers can go up to the maxWavePlayers(0x300)
	rage::wavePlayerStruct audMixerSyncSignalArray[0x301];

#ifdef GTA_FIVE
	auto validBits = (int*)((uintptr_t)thisptr + 0xD498);
#elif IS_RDR3
	auto validBits = (int*)((uintptr_t)thisptr + 0x1D348);
#endif

	for (size_t i = 0; i < maxWavePlayers; i++)
	{
		if (i >= thisptr->GetMaxWavePlayers())
		{
			continue;
		}

		int thisBit = validBits[i / 32];
		if ((thisBit & (1 << (i % 32))) == 0)
		{
			continue;
		}

		if (refArray[i])
		{
#ifdef GTA_FIVE
			char* voiceInst = (char*)rage::audDriver::m_VoiceManager + 36 + (i * 20);
			if constexpr (Build < 2189)
			{
				voiceInst = (char*)rage::audDriver::m_VoiceManager + (i * 16);
			}
#elif IS_RDR3
			char* voiceInst = (char*)rage::audDriver::m_VoiceManager + (i * 16);
#endif
			if (*(int*)(voiceInst + 4) == -1)
			{
				continue;
			}

			rage::audWavePlayer* wavePlayer = thisptr->GetWavePlayerByIndex(int(i));
#ifdef GTA_FIVE
			uint16_t state = wavePlayer->GetMembers<Build>().unk_1A;
#elif IS_RDR3
			uint16_t state = wavePlayer->GetMembers().unk_1A;
#endif
			if (state != 0xFFFF)
			{
#ifdef GTA_FIVE
				bool areStatesEqual = (state == wavePlayer->GetMembers<Build>().unk_1C);
#elif IS_RDR3
				bool areStatesEqual = (state == wavePlayer->GetMembers().unk_1C);
#endif
				audMixerSyncSignalArray[numActivePlayers].wavePlayerState = (int)state;
				audMixerSyncSignalArray[numActivePlayers].wavePlayerIndex = int(i);
				audMixerSyncSignalArray[numActivePlayers].wavePlayerAreStatesEqual = (int)areStatesEqual;

				++numActivePlayers;

				if (!areStatesEqual)
				{
					continue;
				}
			}
#ifdef GTA_FIVE
			if (wavePlayer->GetMembers<Build>().unk_8 == 0xFFFF)
#elif IS_RDR3
			if (wavePlayer->GetMembers().unk_8 == 0xFFFF)
#endif
			{
				wavePlayer->SkipFrame();
			}
			else
			{
				wavePlayer->GenerateFrame();
			}

			if (wavePlayer->IsFinished())
			{
				*(int*)(voiceInst + 4) = -1;
			}
			else
			{
				*(int*)(voiceInst + 4) = wavePlayer->GetPlayPositionSamples();
			}
#ifdef GTA_FIVE
			if constexpr (Build >= 2189)
			{
				*(int*)(voiceInst + 8) = *(int*)(*rage::audDriver::sm_Mixer + 0xF260); // update tickcount
				*(uint16_t*)(voiceInst + 12) = wavePlayer->GetNumberOfChannels(); // not in 1604
				*(int*)(voiceInst + 16) &= 0xFFF7FFFF; // clear hasStarted flag
				*(int*)(voiceInst + 16) |= (wavePlayer->HasStartedPlayback()) << 19;
			}
			else
			{
				*(uint16_t*)(voiceInst + 8) = wavePlayer->GetNumberOfChannels();
				*(int*)(voiceInst + 12) &= 0xFFF7FFFF; // clear hasStarted flag
				*(int*)(voiceInst + 12) |= (wavePlayer->HasStartedPlayback()) << 19;
			}
#elif IS_RDR3
			*(uint16_t*)(voiceInst + 8) = wavePlayer->GetNumberOfChannels();
			*(int*)(voiceInst + 12) &= 0xFFF7FFFF; // clear hasStarted flag
			*(int*)(voiceInst + 12) |= (wavePlayer->HasStartedPlayback()) << 19;
#endif
		}
		else
		{
			rage::audMixerDevice__FreePcmSourceSlot(thisptr, int(i));
		}
	}

	for (int arrIndex = 0; arrIndex < numActivePlayers; arrIndex++)
	{
		rage::audWavePlayer* wavePlayer = thisptr->GetWavePlayerByIndex(audMixerSyncSignalArray[arrIndex].wavePlayerIndex);
#ifdef GTA_FIVE
		int v18 = *(int*)(*rage::audDriver::sm_Mixer + 4 * audMixerSyncSignalArray[arrIndex].wavePlayerState + 0xF498);
#elif IS_RDR3
		int v18 = *(int*)(*rage::audDriver::sm_Mixer + 4 * audMixerSyncSignalArray[arrIndex].wavePlayerState + 0x1FB3C);
#endif
		if ((BYTE2(v18) || HIBYTE(v18)) && wavePlayer->ProcessSyncSignal(&v18) || !audMixerSyncSignalArray[arrIndex].wavePlayerAreStatesEqual)
		{
#ifdef GTA_FIVE
			if (wavePlayer->GetMembers<Build>().unk_8 == 0xFFFF)
#elif IS_RDR3
			if (wavePlayer->GetMembers().unk_8 == 0xFFFF)
#endif
			{
				wavePlayer->SkipFrame();
			}
			else
			{
				wavePlayer->GenerateFrame();
			}
			if (audMixerSyncSignalArray[arrIndex].wavePlayerIndex != -1)
			{
				auto i = audMixerSyncSignalArray[arrIndex].wavePlayerIndex;

#ifdef GTA_FIVE
				char* voiceInst = (char*)rage::audDriver::m_VoiceManager + 36 + (i * 20);
				if constexpr (Build < 2189)
				{
					voiceInst = (char*)rage::audDriver::m_VoiceManager + (i * 16);
				}
#elif IS_RDR3
				char* voiceInst = (char*)rage::audDriver::m_VoiceManager + (i * 16);
#endif
				// this chunk is not inlined on 2189+, function name is unknown however
				{
					if (wavePlayer->IsFinished())
					{
						*(int*)(voiceInst + 4) = -1;
					}
					else
					{
						*(int*)(voiceInst + 4) = wavePlayer->GetPlayPositionSamples();
					}
#ifdef GTA_FIVE
					if constexpr (Build >= 2189)
					{
						*(int*)(voiceInst + 8) = *(int*)(*rage::audDriver::sm_Mixer + 0xF260); // update tickcount
						*(uint16_t*)(voiceInst + 12) = wavePlayer->GetNumberOfChannels(); // not in 1604
						*(int*)(voiceInst + 16) &= 0xFFF7FFFF; // clear hasStarted flag
						*(int*)(voiceInst + 16) |= (wavePlayer->HasStartedPlayback()) << 19;
					}
					else
					{
						*(uint16_t*)(voiceInst + 8) = wavePlayer->GetNumberOfChannels();
						*(int*)(voiceInst + 12) &= 0xFFF7FFFF; // clear hasStarted flag
						*(int*)(voiceInst + 12) |= (wavePlayer->HasStartedPlayback()) << 19;
					}
#elif IS_RDR3
					*(uint16_t*)(voiceInst + 8) = wavePlayer->GetNumberOfChannels();
					*(int*)(voiceInst + 12) &= 0xFFF7FFFF; // clear hasStarted flag
					*(int*)(voiceInst + 12) |= (wavePlayer->HasStartedPlayback()) << 19;
#endif
				}
			}
		}
	}
}

static HookFunction hookFunction([]()
{
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("75 17 40 38 2D ? ? ? ? 74 0E 39 2D", 5);
		audioNotFocused = hook::get_address<bool*>(location);
		muteOnFocusLoss = hook::get_address<int*>(location + 8);
#elif IS_RDR3
		audioNotFocused = hook::get_address<bool*>(hook::get_pattern<char>("80 3D ? ? ? ? ? 74 04 B3 01 EB 08"), 2, 7);
#endif
	}
	
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern("80 3D ? ? ? ? 00 0F 84 ? 00 00 00 80 BB", 2);
		rage::g_audUseFrameLimiter = hook::get_address<bool*>(location) + 1;
#elif IS_RDR3
		auto location = hook::get_pattern("80 3D ? ? ? ? ? 74 6F 48 8D");
		rage::g_audUseFrameLimiter = hook::get_address<bool*>(location, 2, 7);
#endif
	}

	{
		// Re-Build rage::audMixerDevice::GeneratePcm() because the stack-buffer for audMixerSyncSignalArray is too small
#ifdef GTA_FIVE
		auto funcStart = hook::get_pattern("48 8D A8 78 FD FF FF 48 81 EC 60 03 00 00 ? 8B F1", -0x18);

		if (!xbr::IsGameBuildOrGreater<2189>())
		{
			MH_CreateHook(funcStart, rage__audMixerDevice__GeneratePcm<1604>, nullptr);
		}
		else
		{
			MH_CreateHook(funcStart, rage__audMixerDevice__GeneratePcm<2189>, nullptr);
		}
		
		MH_EnableHook(funcStart);

		auto line = hook::get_pattern("48 8D 15 ? ? ? ? 41 B9 FF FF 00 00 45 85 E4");
		rage::audDriver::m_VoiceManager = hook::get_address<int64_t*>(line, 3, 7);
		auto line2 = hook::get_pattern("48 8B 05 ? ? ? ? 0F AF ? ? 03");
		rage::audDriver::sm_Mixer = hook::get_address<char**>(line2, 3, 7);
#elif IS_RDR3
		auto location = hook::get_pattern("E8 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 45 8B 87 ? ? ? ? 48", -0x2E);

		MH_Initialize();

		auto status = MH_CreateHook(location, rage__audMixerDevice__GeneratePcm, nullptr);
		MH_EnableHook(location);

		auto location_voiceManager = hook::get_pattern("48 8D 15 ? ? ? ? 33 F6 89 BD ? ? ? ? 45 85 C0");
		auto location_smMixer = hook::get_pattern("48 8B 0D ? ? ? ? 48 85 C9 40 0F 95 C7 48 85 C9 74 66");
		rage::audDriver::m_VoiceManager = hook::get_address<int64_t*>(location_voiceManager, 3, 7);
		rage::audDriver::sm_Mixer = hook::get_address<char**>(location_smMixer, 3, 7);
#endif
	}

	{
		static bool useSynchronousAudio = false;
		static bool lastUseSynchronousAudio = false;

#ifdef GTA_FIVE
		static auto asynchronousAudio = hook::get_address<bool*>(hook::get_pattern("E8 ? ? ? ? 40 38 35 ? ? ? ? 75 05", 8));
		static auto audioTimeout = hook::get_address<int*>(hook::get_pattern("8B 15 ? ? ? ? 41 03 D6 3B", 2));
#elif IS_RDR3
		static auto asynchronousAudio = hook::get_address<bool*>(hook::get_pattern("80 3D ? ? ? ? ? 74 38 33 DB 40 84 FF 74 19"));
		static auto audioTimeout = hook::get_address<int*>(hook::get_pattern("8B 15 ? ? ? ? 41 03 D6 3B", 2));
#endif
		// See https://github.com/citizenfx/fivem/issues/1446 comments for a more viable solution:
		//
		// > Took a look at this to see if there was an elegant solution besides force-enabling (e.g., similar to
		// > disq's windscreen stub) g_audUseFrameLimiter while editorMode is active.
		// >
		// > On editor export, there is a circular dependency(Mutexes and Semaphores and Deadlocks, Oh My !) between
		// > RageAudioEngineThread, MainThread, and Fake Audio Mixer, that requires(or seems to require) RageAudioEngineThread
		// > to sleep through at least the first frame render so those other threads can initialize(catch - up).
		//
		// For the time being, we'll just take the lazy fix.
		static bool editorMode = false;

		rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
		{
			if (type == rage::INIT_SESSION)
			{
				// editorMode is set only during loading (the init script will unset it once it opened the editor menu)
				// as such we save it on init-session end
				editorMode = Instance<ICoreGameInit>::Get()->HasVariable("editorMode");
			}
		});

		OnGameFrame.Connect([]()
		{
			bool targetUseSynchronousAudio = !editorMode && useSynchronousAudio;

			if (targetUseSynchronousAudio != lastUseSynchronousAudio)
			{
				if (targetUseSynchronousAudio)
				{
					*asynchronousAudio = false;
					*audioTimeout = 0;
				}
				else
				{
					*asynchronousAudio = true;
					*audioTimeout = 1000;
				}

				lastUseSynchronousAudio = targetUseSynchronousAudio;
			}

			*rage::g_audUseFrameLimiter = editorMode || g_audUseFrameLimiterConVar;
		});

		static ConVar<bool> audUseFrameLimiter("game_useSynchronousAudio", ConVar_Archive, false, &useSynchronousAudio);
	}

	static ConVar<bool> audUseFrameLimiter("game_useAudioFrameLimiter", ConVar_Archive, true, &g_audUseFrameLimiterConVar);
#if IS_RDR3
	static ConVar<bool> uiMuteOnFocusLoss("ui_muteOnFocusLoss", ConVar_Archive, muteOnFocusLoss, &muteOnFocusLoss);
#endif
});
