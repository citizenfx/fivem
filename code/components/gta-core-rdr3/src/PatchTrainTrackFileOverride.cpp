#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.Patterns.h>
#include <GameInit.h>

//
// The tracks that vehicles like trains and the Saint Denis trolley follow, as well as the cables dangling
// above the trolley's route, are defined in a plain-text format stored in several *.dat files. These files
// are in turn referenced by filepath by an XML file - traintracks.xml for vehicles, trolleyCableTracks.xml for the cables.
// 
// There is currently no way to fully replace the track data within the *.dat files, nor to modify or override the XML files.
// This patch introduces a way to replace the XML files, allowing for fully custom rail (and trolley cable?) networks.
// We intercept all calls to LoadTrackXML(), and if one of the override filepaths is valid, we use it instead of the default
// path. To set these override filenames, the resource must define either (or both) of the following statements:
// 
// * replace_traintrack_file 'path/to/traintracks.xml'
// 
// * replace_trolley_cable_file 'path/to/trolleyCableTracks.xml'
//

#define DEFAULT_TRACK_FILE  "common:/data/levels/rdr3/traintracks.xml"
#define DEFAULT_CABLES_FILE "common:/data/levels/rdr3/trolleyCableTracks.xml"

static std::string g_overrideTrainTrackFilePath   = "";
static std::string g_overrideTrolleyCableFilePath = "";

namespace streaming
{
	// Sets a resource override for traintracks.xml and enables train track replacement.
	void DLL_EXPORT SetTrainTrackFilePath(const std::string& path)
	{
		g_overrideTrainTrackFilePath = path;
	}

	// Sets a resource override for trolleyCableTracks.xml and enables trolley cable replacement.
	void DLL_EXPORT SetTrolleyCableFilePath(const std::string& path)
	{
		g_overrideTrolleyCableFilePath = path;
	}
}

typedef __int64 CTrainTrackPool; // Typedef for clarity on LoadTrackXML()'s parameters.

static void (*g_origLoadTrackXML)(const char*, CTrainTrackPool*);
static void LoadTrackXML(const char* origXmlFileName, CTrainTrackPool* dstPool)
{
    if (!g_overrideTrainTrackFilePath.empty() && strcmp(origXmlFileName, DEFAULT_TRACK_FILE) == 0)
    {
		trace("Replacing default traintracks.xml with %s\n", g_overrideTrainTrackFilePath.data());
        g_origLoadTrackXML(g_overrideTrainTrackFilePath.data(), dstPool);
    }
	else if (!g_overrideTrolleyCableFilePath.empty() && strcmp(origXmlFileName, DEFAULT_CABLES_FILE) == 0)
	{
		trace("Replacing default trolleyCableTracks.xml with %s\n", g_overrideTrolleyCableFilePath.data());
		g_origLoadTrackXML(g_overrideTrolleyCableFilePath.data(), dstPool);
	}
	else
	{
		g_origLoadTrackXML(origXmlFileName, dstPool);
	}
}

static HookFunction hookFunction([]()
{
	{
        auto location = hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? EB 44"));
        g_origLoadTrackXML = hook::trampoline(location, LoadTrackXML);
	}

	OnKillNetworkDone.Connect([]()
	{
		g_overrideTrainTrackFilePath.clear();
		g_overrideTrolleyCableFilePath.clear();
	});
});
