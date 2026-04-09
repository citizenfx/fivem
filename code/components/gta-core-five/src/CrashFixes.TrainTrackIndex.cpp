#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.Patterns.h>

class CTrainTrack
{
public:
	char m_Padding[0x10];
	int32_t m_NumNodes;

	char m_Padding2[0x239];
};
static_assert(sizeof(CTrainTrack) == 0x250, "CTrainTrack has wrong size!");

static CTrainTrack (*g_TrainTracks)[27] = nullptr;

static uint32_t g_TrainTrackOffset = 0x0;

static void (*g_CNetObjTrain_SetTrainGameState)(hook::FlexStruct*, hook::FlexStruct*);

static void CNetObjTrain_SetTrainGameState(hook::FlexStruct* thisPtr, hook::FlexStruct* dataNode)
{
	int8_t& trackIndex = dataNode->At<int8_t>(g_TrainTrackOffset);

	// NOTE: this should be fine since b3788 does exactly the same for carriage and config
	//		 and yes, -1 is correct, the game uses that for default / uninitialized train tracks
	if (trackIndex < -1 || trackIndex >= 27)
	{
		trackIndex = -1;
	}
	else if (trackIndex >= 0)
	{
		// fixes another exploit, where a cheater can serialize a train track with no initialized nodes
		if (!g_TrainTracks[trackIndex] || g_TrainTracks[trackIndex]->m_NumNodes <= 0)
		{
			trackIndex = -1;
		}
	}
	
	g_CNetObjTrain_SetTrainGameState(thisPtr, dataNode);
}

static HookFunction hookFunction([]()
{
	// An invalid out-of-bounds track index can be sent cheaters.
	// The game uses this value without proper validation, leading to bad memory access and a crash.
	//
	// This hook clamps the track index to the valid range preventing out-of-bounds access and avoiding the crash.

	auto netObjTrainVtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 81 ? ? ? ? 75", 3));

	g_TrainTrackOffset = *hook::get_pattern<uint32_t>("88 82 ? ? ? ? 8B 87 ? ? ? ? 89 82", 2);

	g_CNetObjTrain_SetTrainGameState = (decltype(g_CNetObjTrain_SetTrainGameState))netObjTrainVtable[8];
	hook::put(&netObjTrainVtable[8], (uintptr_t)CNetObjTrain_SetTrainGameState);

	g_TrainTracks = hook::get_address<CTrainTrack(*)[27]>(hook::get_pattern("4C 8D 15 ? ? ? ? 4D 69 C9", 3));
});
