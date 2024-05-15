 #include "StdInc.h"
#include <Hooking.h>
#include <MinHook.h>
#include <ScriptEngine.h>
#include <ICoreGameInit.h>
#include <GameInit.h>
#include "GamePrimitives.h"

#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>
#include <VFSManager.h>

#define MAX_TRACKS 50
#define DEFAULT_TRACK_FILE  "common:/data/levels/rdr3/traintracks.xml"

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

	char Padding[0x1244];
};

// Pointer to the main train tracks array. Fixed-sized array of size 50.
static sTrainTrack* g_trainTracks;
// Pointer to the number of loaded tracks in the main tracks array.
static int* g_trainTrackCount;

std::mutex g_loaderLock;

// Internal function that loads the XML file at the given path into the given tracks array.
static hook::cdecl_stub<void(const char* path, sTrainTrack* tracksArray)> _loadTracks([]()
{
	return hook::get_call(hook::get_pattern("48 8D 15 ? ? ? ? 49 8B C9 E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D", 10));
});

// Internal function that does some post-processing on the given tracks array; primarily for setting up junctions?
static hook::cdecl_stub<void(sTrainTrack* tracksArray)> _postProcessTracks1([]()
{
	return hook::get_pattern("48 89 4C 24 ? 53 55 56 57 41 54 41 55 41 56 41 57 48 63 99");
});

// Internal function that does some post-processing on the given tracks array; purpose not clear.
static hook::cdecl_stub<void(sTrainTrack* tracksArray)> _postProcessTracks2([]()
{
	return hook::get_pattern("48 89 4C 24 ? 53 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 8B 81");
});

// Internal function that clears the main tracks array.
static hook::cdecl_stub<void()> _unloadTracks([]()
{
	return hook::get_pattern
	(
		"48 89 5C 24 ? 57 48 83 EC ? 48 8D 1D ? ? ? ? BF ? ? ? ? 48 8B CB E8 ? ? ? ? 48 81 C3 ? ? ? ? 48 83 EF ? 75 ? 21 3D "
		"? ? ? ? 48 8D 0D ? ? ? ? 48 8B 5C 24 ? 48 83 C4 ? 5F E9 ? ? ? ? CC 48 89 5C 24 ? 57 48 83 EC ? 48 8D 1D ? ? ? ? BF "
		"? ? ? ? 48 8B CB E8 ? ? ? ? 48 81 C3 ? ? ? ? 48 83 EF ? 75 ? 21 3D ? ? ? ? 48 8D 0D ? ? ? ? 48 8B 5C 24 ? 48 83 C4 "
		"? 5F E9 ? ? ? ? CC 0F 28 05"
	);
});

void LoadTracks(const char* path)
{
	if (path != nullptr)
	{
		_loadTracks(path, g_trainTracks);
	}
	else
	{
		_loadTracks(DEFAULT_TRACK_FILE, g_trainTracks);
	}

	_postProcessTracks1(g_trainTracks);
	_postProcessTracks2(g_trainTracks);
}

static HookFunction trackNativesFunc([]()
{
	g_trainTracks = hook::get_address<sTrainTrack*>(hook::get_pattern("48 8D 15 ? ? ? ? 49 8B C9 E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D", 3));
	g_trainTrackCount = hook::get_address<int*>(hook::get_pattern("8B 05 ? ? ? ? 89 75 ? 89 45", 2));

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_COUNT", [](fx::ScriptContext& scriptContext)
	{
		scriptContext.SetResult<int>(*g_trainTrackCount);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_TRACK_FROM_INDEX", [](fx::ScriptContext& scriptContext)
	{
		int trackCount = *g_trainTrackCount;
		int targetIdx = scriptContext.GetArgument<int>(0);

		if (trackCount == 0 || targetIdx >= trackCount)
		{
			scriptContext.SetResult<int>(0);
		}
		else
		{
			scriptContext.SetResult<int>(g_trainTracks[targetIdx].NameHash);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("LOAD_TRACKS_FROM_FILE", [](fx::ScriptContext& scriptContext)
	{
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(scriptContext.GetArgument<const char*>(0));

		if (!resource.GetRef())
		{
			scriptContext.SetResult(false);
			return;
		}

		std::string filePath = resource->GetPath();

		// Make sure path separator exists or add it before combining path with file name
		char c = filePath[filePath.length() - 1];
		if (c != '/' && c != '\\')
		{
			filePath += '/';
		}

		filePath += scriptContext.GetArgument<const char*>(1);

		fwRefContainer<vfs::Stream> stream = vfs::OpenRead(filePath);
		if (!stream.GetRef())
		{
			trace("unable to find traintracks.xml at %s\n", filePath.c_str());
			scriptContext.SetResult(false);

			return;
		}

		std::lock_guard _(g_loaderLock);

		_unloadTracks();
		LoadTracks(filePath.c_str());

		scriptContext.SetResult(true);
	});

	OnKillNetworkDone.Connect([]()
	{
		std::lock_guard _(g_loaderLock);

		_unloadTracks();
		LoadTracks(nullptr);
	});
});
