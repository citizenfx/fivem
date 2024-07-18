#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.Patterns.h>
#include <GameInit.h>

//
// Trains, trolleys, and minecarts all follow "tracks" made of splines. These tracks are defined by several *.dat files,
// which are in turn grouped together via the metadata at common:/data/levels/rdr3/traintracks.xml.
// 
// This patch adds a global override path for traintracks.xml, which can be set by using the new `replace_traintrack_file`
// manifest statement in the fxmanifest file. This allows creators to make fully customized railway networks, without
// having to work around the base game's existing tracks.
//

// Path to the default train tracks file.
constexpr char DEFAULT_TRACKS_XML[] = "common:/data/levels/rdr3/traintracks.xml";

struct CTrainTrackPool;
static std::string g_trainTrackOverridePath = "";

namespace streaming
{
	// RDR3 only: Set the path for a train track XML file to load instead of the default one.
	void DLL_EXPORT SetTrainTrackOverridePath(const std::string& path)
	{
		g_trainTrackOverridePath = path;
	}
}

// Pointer to the original function that loads train tracks from the XML file.
static void (*g_origLoadTrackXML)(const char*, CTrainTrackPool*);

static void LoadTrackXML(const char* fileName, CTrainTrackPool* dstPool)
{
	// If the file name we were given is the default one for train tracks, and the override path isn't empty,
	// replace the default file name with our override.
	// (We have to check for the train tracks file name because this same function is also used to load the trolley cables.)
    if (strcmp(fileName, DEFAULT_TRACKS_XML) == 0 && !g_trainTrackOverridePath.empty())
    {
		trace("Replacing %s with %s\n", DEFAULT_TRACKS_XML, g_trainTrackOverridePath.data());
		fileName = g_trainTrackOverridePath.data();
    }

	g_origLoadTrackXML(fileName, dstPool);
}

static HookFunction hookFunction([]()
{
	// Intercept calls to LoadTrackXML() to insert our override check.
	{
        auto location = hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? EB 44"));
        g_origLoadTrackXML = hook::trampoline(location, LoadTrackXML);
	}

	OnKillNetworkDone.Connect([]()
		// Clear the override path so that the client loads the default tracks
		// the next time they connect to a server.
		{
			g_trainTrackOverridePath.clear();
		}
	);
});
