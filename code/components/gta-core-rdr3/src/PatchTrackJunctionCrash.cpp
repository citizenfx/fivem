#include <StdInc.h>

#include "Hooking.h"
#include "Hooking.Stubs.h"

//
// When the game loads junctions for train tracks, it will construct the junction with default values. Only later,
// when all the tracks have been loaded, does it set up the actual connections between the junctions that
// allow trains to utilize them. If a junction doesn't refer to an existing track, the post-processing function rightly
// ignores it and leaves it with its default values. However, custom train tracks have revealed a flaw with this:
// if a train attempts to interact with a junction that was ignored during post-processing, a crash occurs as the train
// attempts to use its invalid data to determine what track it should switch to.
// 
// The solution presented below intercepts the original PostProcessJunctions() function. It checks the junctions that
// were loaded from the track files, and removes those that do not refer to a valid track. The game is then allowed
// to proceed into the original version of the function to continue setting up the junction data.
//

constexpr uint8_t NODE_FLAG_JUNCTION = 0x08;
constexpr uint8_t NODE_FLAG_0x10 = 0x10;

struct sTrainTrackJunction
{
	char Padding[0x08];
	uint32_t NodeIndex;
	char Padding2[0x0C];
	uint32_t Index;
	char Padding3[0x14];
	uint32_t TrackNameHash;
	char Padding4[0x1C];
};

struct sTrainTrackJunctionPool
{
	sTrainTrackJunction Junctions[20];
	uint32_t Count;
};

struct sTrainTrackNode
{
	uint64_t VTable;
	uint8_t Flags;
	uint8_t Padding[0x27];
};

struct sTrainTrack
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

	sTrainTrackNode** NodePtrs;

	char Padding[0x290];
	sTrainTrackJunctionPool Junctions;
	char Padding2[0x960];
};

struct sTrainTrackPool
{
	sTrainTrack Tracks[50];
	uint32_t Count;
};

// Pointer to the train tracks pool.
static sTrainTrackPool* g_trainTrackPool;
// Pointer to the original function that sets up track junctions.
static void (*g_origPostProcessJunctions)(sTrainTrackPool*);

// Internal function for getting a track's index from its name hash. Returns -1 if the hash was not in the tracks pool.
static hook::cdecl_stub<int8_t(uint32_t)> _getTrackIndexFromHash([]()
{
	return hook::get_pattern("44 8B 0D ? ? ? ? 45 32 C0");
});

static void PostProcessJunctions(sTrainTrackPool* tracks)
{
	for (uint32_t trk = 0; trk < tracks->Count; trk++)
	{
		sTrainTrack& curTrack = tracks->Tracks[trk];
		for (uint32_t jct = 0; jct < curTrack.Junctions.Count; jct++)
		{
			sTrainTrackJunction& curJunct = curTrack.Junctions.Junctions[jct];
			if (_getTrackIndexFromHash(curJunct.TrackNameHash) == -1)
			{
				trace("Removing junction %i from track %i (Unknown track name %X)\n", jct, trk, curJunct.TrackNameHash);

				// Clear the flags marking this node as a junction; not doing this causes the game to hang!
				curTrack.NodePtrs[curJunct.NodeIndex]->Flags &= ~(NODE_FLAG_0x10 | NODE_FLAG_JUNCTION);

				// Move all the junctions down one slot in the array (and adjust their internal indices).
				for (uint32_t rmv = jct; rmv < curTrack.Junctions.Count - 1; rmv++)
				{
					memcpy(&curTrack.Junctions.Junctions[rmv], &curTrack.Junctions.Junctions[rmv + 1], sizeof(sTrainTrackJunction));
					curTrack.Junctions.Junctions[rmv].Index--;
				}

				// Clear the now-unreferenced junction at the end of the array, just to be tidy.
				memset(&curTrack.Junctions.Junctions[curTrack.Junctions.Count - 1], 0, sizeof(sTrainTrackJunction));
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

static HookFunction hookFunction([]()
{
	auto location = hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B CD E8 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B ? 49 8B 6B ? 49 8B 73 ? 49 8B 7B ? 49 8B E3 41 5F"));
	g_origPostProcessJunctions = hook::trampoline(location, PostProcessJunctions);
});
