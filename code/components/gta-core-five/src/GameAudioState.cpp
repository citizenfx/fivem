#include <StdInc.h>
#include <Hooking.h>

#include <CoreConsole.h>
#include <nutsnbolts.h>

#include <CrossBuildRuntime.h>

#include <MinHook.h>

static bool* audioNotFocused;
static int* muteOnFocusLoss; 

bool DLL_EXPORT ShouldMuteGameAudio()
{
	return *audioNotFocused && *muteOnFocusLoss;
}

namespace rage
{
bool* g_audUseFrameLimiter;

struct wavePlayerStruct
{
	int wavePlayerIndex;
	int wavePlayerState;
	int wavePlayerAreStatesEqual; // +0x1A == +0x1C
}; // size = 12 bytes

template<int Build>
struct audWavePlayer_members
{
	uint16_t unk_8;
	char _pad[(Build >= 2189) ? 16 : 10];
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
	uint8_t members[sizeof(audWavePlayer_members<2189>)];

public:
	template<int Build>
	inline auto& GetMembers()
	{
		return *(audWavePlayer_members<Build>*)&members[0];
	}
};

struct audMixerDevice
{
	// Offsets are the same 1604-2545
	inline int GetMaxWavePlayers()
	{
		return *(int*)(((uintptr_t)this) + 0xEA08);
	}

	inline int GetWavePlayerSize()
	{
		return *(int*)(((uintptr_t)this) + 0xEA0C);
	}

	inline int* GetRefArray()
	{
		return (int*)(((uintptr_t)this) + 0xDE00);
	}

	inline audWavePlayer* GetWavePlayerByIndex(int index)
	{
		uintptr_t wavePlayerArrayStart = *(uintptr_t*)(((uintptr_t)this) + 0xEA00);
		wavePlayerArrayStart += (index * GetWavePlayerSize());
		return (audWavePlayer*)wavePlayerArrayStart;
	}

};

static hook::thiscall_stub<void*(rage::audMixerDevice*, int)> audMixerDevice__FreePcmSourceSlot([]()
{
	return hook::get_call(hook::get_pattern("8B CE E8 ? ? ? ? E9 ? 00 00 00 83 FF FF", 2));
});

namespace audDriver
{
	inline int64_t* m_VoiceManager;
	inline char** sm_Mixer = 0;
}
}
#define BYTEn(x, n) (*((unsigned char*)&(x) + n))
#define BYTE2(x) BYTEn(x, 2)

template<int Build>
static void rage__audMixerDevice__GeneratePcm(rage::audMixerDevice* thisptr)
{
	int maxWavePlayers = thisptr->GetMaxWavePlayers();
	int* refArray = thisptr->GetRefArray();
	int numActivePlayers = 0;

	// original size is 64, problem is that numActivePlayers can go up to the maxWavePlayers(0x300)
	rage::wavePlayerStruct audMixerSyncSignalArray[0x301];

	auto validBits = (int*)((uintptr_t)thisptr + 0xD498);

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
			char* voiceInst = (char*)rage::audDriver::m_VoiceManager + 36 + (i * 20);

			if constexpr (Build < 2189)
			{
				voiceInst = (char*)rage::audDriver::m_VoiceManager + (i * 16);
			}

			if (*(int*)(voiceInst + 4) == -1)
			{
				continue;
			}

			rage::audWavePlayer* wavePlayer = thisptr->GetWavePlayerByIndex(int(i));
			uint16_t state = wavePlayer->GetMembers<Build>().unk_1A;
			if (state != 0xFFFF)
			{
				bool areStatesEqual = (state == wavePlayer->GetMembers<Build>().unk_1C);
				audMixerSyncSignalArray[numActivePlayers].wavePlayerState = (int)state;
				audMixerSyncSignalArray[numActivePlayers].wavePlayerIndex = int(i);
				audMixerSyncSignalArray[numActivePlayers].wavePlayerAreStatesEqual = (int)areStatesEqual;

				++numActivePlayers;

				if (!areStatesEqual)
				{
					continue;
				}
			}

			if (wavePlayer->GetMembers<Build>().unk_8 == 0xFFFF)
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
		}
		else
		{
			rage::audMixerDevice__FreePcmSourceSlot(thisptr, int(i));
		}
	}

	for (int arrIndex = 0; arrIndex < numActivePlayers; arrIndex++)
	{
		rage::audWavePlayer* wavePlayer = thisptr->GetWavePlayerByIndex(audMixerSyncSignalArray[arrIndex].wavePlayerIndex);
		int v18 = *(int*)(*rage::audDriver::sm_Mixer + 4 * audMixerSyncSignalArray[arrIndex].wavePlayerState + 0xF498);
		if ((BYTE2(v18) || HIBYTE(v18)) && wavePlayer->ProcessSyncSignal(&v18) || !audMixerSyncSignalArray[arrIndex].wavePlayerAreStatesEqual)
		{
			if (wavePlayer->GetMembers<Build>().unk_8 == 0xFFFF)
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
				char* voiceInst = (char*)rage::audDriver::m_VoiceManager + 36 + (i * 20);

				if constexpr (Build < 2189)
				{
					voiceInst = (char*)rage::audDriver::m_VoiceManager + (i * 16);
				}

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

					if constexpr (Build >= 2189)
					{
						*(int*)(voiceInst + 8) = *(int*)(*rage::audDriver::sm_Mixer + 0xF260);
						*(uint16_t*)(voiceInst + 12) = wavePlayer->GetNumberOfChannels();
						*(int*)(voiceInst + 16) &= 0xFFF7FFFF;
						*(int*)(voiceInst + 16) |= (wavePlayer->HasStartedPlayback()) << 19;
					}
					else
					{
						*(uint16_t*)(voiceInst + 8) = wavePlayer->GetNumberOfChannels();
						*(int*)(voiceInst + 12) &= 0xFFF7FFFF;
						*(int*)(voiceInst + 12) |= (wavePlayer->HasStartedPlayback()) << 19;
					}
				}
			}
		}
	}
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("75 17 40 38 2D ? ? ? ? 74 0E 39 2D", 5);
		audioNotFocused = hook::get_address<bool*>(location);
		muteOnFocusLoss = hook::get_address<int*>(location + 8);
	}
	
	{
		auto location = hook::get_pattern("80 3D ? ? ? ? 00 0F 84 ? 00 00 00 80 BB", 2);
		rage::g_audUseFrameLimiter = hook::get_address<bool*>(location) + 1;
	}

	{
		// Re-Build rage::audMixerDevice::GeneratePcm() because the stack-buffer for audMixerSyncSignalArray is too small
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
	}

	{
		static bool useSynchronousAudio = false;
		static bool lastUseSynchronousAudio = false;

		static auto asynchronousAudio = hook::get_address<bool*>(hook::get_pattern("E8 ? ? ? ? 40 38 35 ? ? ? ? 75 05", 8));
		static auto audioTimeout = hook::get_address<int*>(hook::get_pattern("8B 15 ? ? ? ? 41 03 D6 3B", 2));

		OnGameFrame.Connect([]()
		{
			if (useSynchronousAudio != lastUseSynchronousAudio)
			{
				if (useSynchronousAudio)
				{
					*asynchronousAudio = false;
					*audioTimeout = 0;
				}
				else
				{
					*asynchronousAudio = true;
					*audioTimeout = 1000;
				}

				lastUseSynchronousAudio = useSynchronousAudio;
			}
		});

		static ConVar<bool> audUseFrameLimiter("game_useSynchronousAudio", ConVar_Archive, false, &useSynchronousAudio);
	}

	static ConVar<bool> audUseFrameLimiter("game_useAudioFrameLimiter", ConVar_Archive, true, rage::g_audUseFrameLimiter);
});
