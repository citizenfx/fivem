#include <StdInc.h>

#include "Hooking.h"
#include "Hooking.Stubs.h"

//
// This file patches a few crashes that can occur when overriding the default train tracks with user-generated ones:
// 
// * Track junctions pointing to invalid tracks will now be deleted after the tracks XML is loaded.
// * The base-game native CREATE_MISSION_TRAIN will now fail to create a train if the client it is executed on has no tracks loaded.
// * The base game only has dedicated space for 50 total track objects; the track loading loop will now exit once the pool reaches that maximum.
//

constexpr uint8_t NODE_FLAG_JUNCTION = 0x08;
constexpr uint8_t NODE_FLAG_0x10 = 0x10;
constexpr uint8_t JUNCTION_MAX = 20;
constexpr uint8_t TRAIN_TRACK_MAX = 50;

struct CTrainTrackJunction
{
	char Padding[0x08];
	uint32_t NodeIndex;
	char Padding2[0x0C];
	uint32_t Index;
	char Padding3[0x14];
	uint32_t TrackNameHash;
	char Padding4[0x1C];
};

struct CTrainTrackJunctionPool
{
	CTrainTrackJunction Junctions[JUNCTION_MAX];
	uint32_t Count;
};

struct CTrainTrackNode
{
	uint64_t VTable;
	uint8_t Flags;
	uint8_t Padding[0x27];
};

struct CTrainTrack
{
	bool bEnabled;
	bool bOpen;
	bool bHasJunctions;
	bool bStopsAtStations;
	bool bMPStopsAtStations;

	uint32_t NameHash;
	uint32_t BrakingDistance;

	uint32_t TotalNodeCount;
	uint32_t LinearNodeCount;
	uint32_t CurveNodeCount;

	uint32_t m001C;

	CTrainTrackNode** NodePtrs;

	char Padding[0x290];
	CTrainTrackJunctionPool Junctions;
	char Padding2[0x960];
};

struct CTrainTrackPool
{
	CTrainTrack Tracks[TRAIN_TRACK_MAX];
	uint32_t Count;
};

static CTrainTrackPool* g_trainTrackPool;

// Pointer to the original function that sets up track junctions.
static void (*g_origPostProcessJunctions)(CTrainTrackPool*);
// Internal function for spawning a train as a mission vehicle.
static int64_t (*g_origCreateMissionTrain)(uint32_t, float*, bool, bool, bool, bool);

// Internal function for getting a track's index from its name hash. Returns -1 if the hash was not in the tracks pool.
static hook::cdecl_stub<int8_t(uint32_t)> _getTrackIndexFromHash([]()
{
	return hook::get_pattern("44 8B 0D ? ? ? ? 45 32 C0");
});

static void PostProcessJunctions(CTrainTrackPool* tracks)
{
	for (uint32_t trk = 0; trk < tracks->Count; trk++)
	{
		CTrainTrack& curTrack = tracks->Tracks[trk];
		for (uint32_t jct = 0; jct < curTrack.Junctions.Count; jct++)
		{
			CTrainTrackJunction& curJunct = curTrack.Junctions.Junctions[jct];
			if (_getTrackIndexFromHash(curJunct.TrackNameHash) == -1)
			{
				trace("Removing junction %i from track %i - unknown track name 0x%X.\n", jct, trk, curJunct.TrackNameHash);

				// Clear the flags marking this node as a junction; not doing this causes the game to hang!
				curTrack.NodePtrs[curJunct.NodeIndex]->Flags &= ~(NODE_FLAG_0x10 | NODE_FLAG_JUNCTION);

				// Move all the junctions down one slot in the array (and adjust their internal indices).
				for (uint32_t rmv = jct; rmv < curTrack.Junctions.Count - 1; rmv++)
				{
					memcpy(&curTrack.Junctions.Junctions[rmv], &curTrack.Junctions.Junctions[rmv + 1], sizeof(CTrainTrackJunction));
					curTrack.Junctions.Junctions[rmv].Index--;
				}

				// Clear the now-unreferenced junction at the end of the array, just to be tidy.
				memset(&curTrack.Junctions.Junctions[curTrack.Junctions.Count - 1], 0, sizeof(CTrainTrackJunction));

				curTrack.Junctions.Count--;
				jct--;

				// Make sure the track knows it has no junctions if the only one it had was removed.
				if (curTrack.Junctions.Count == 0)
				{
					curTrack.bHasJunctions = false;
				}
			}
		}
	}

	// Continue on to the original game's processing function.
	g_origPostProcessJunctions(tracks);
}

static int64_t CreateMissionTrain(uint32_t config, float* position, bool direction, bool passengers, bool p4, bool conductor)
{
	if (g_trainTrackPool->Count == 0)
	{
		trace("CreateMissionTrain() failed - track pool is empty!\n");
		return 0;
	}

	return g_origCreateMissionTrain(config, position, direction, passengers, p4, conductor);
}

static bool IsTrackPoolFull()
{
	if (g_trainTrackPool->Count >= TRAIN_TRACK_MAX)
	{
		trace("Track pool is full - no more tracks can be loaded.\n");
		return false;
	}

	return true;
}

static HookFunction hookFunction([]()
{
	g_trainTrackPool = hook::get_address<CTrainTrackPool*>(hook::get_pattern("48 8D 15 ? ? ? ? 49 8B C9 E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D", 3));

	// Fixes crash with junctions that point to invalid tracks
	{
		auto location = hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B CD E8 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 49 8B 6B ? 49 8B 73 ? 49 8B 7B ? 49 8B E3 41 5F"));
		g_origPostProcessJunctions = hook::trampoline(location, PostProcessJunctions);
	}

	// Prevents trains from being spawned when no tracks exist
	{
		auto location = hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 81 EC ? ? ? ? 33 DB 41 8A E9");
		g_origCreateMissionTrain = hook::trampoline(location, CreateMissionTrain);
	}

	static struct : jitasm::Frontend
	{
		intptr_t retSuccess = 0;
		intptr_t retFail = 0;

		void Init(intptr_t success, intptr_t fail)
		{
			this->retSuccess = success;
			this->retFail = fail;
		}

		virtual void InternalMain() override
		{
			// Original check for a valid XML element.
			test(rbx, rbx);
			jz("fail");

			// New check for track count < 50.
			mov(rax, reinterpret_cast<uintptr_t>(IsTrackPoolFull));
			call(rax);
			test(al, al);
			jz("fail");

			// We have a valid XML child and the track pool isn't full, load the next track!
			mov(rax, retSuccess);
			jmp(rax);

			// Can't load another track, exit the loading loop.
			L("fail");
			mov(rax, retFail);
			jmp(rax);
		}
	} patchStub;

	// Prevents the game from loading more than 50 tracks
	{
		auto location = hook::get_pattern<char>("48 85 DB 0F 85 ? ? ? ? 49 8B CF");

		// Grab the offset of the body loop from jnz.
		int32_t loopBodyPos = *(int32_t*)(location + 5);
		// loopBodyPos is relative to the end of jnz, which is at location + test + jnz (location + 3 + 6 bytes).
		const auto successPtr = reinterpret_cast<intptr_t>(location) + 9 + loopBodyPos;

		// Skip the original test + jnz instructions.
		const auto failPtr = reinterpret_cast<intptr_t>(location) + 9;

		patchStub.Init(successPtr, failPtr);
		hook::nop(location, 9);
		hook::jump(location, patchStub.GetCode());
	}
});
