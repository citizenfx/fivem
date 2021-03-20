#include <StdInc.h>
#include <Hooking.h>

#include <Streaming.h>

#include <nutsnbolts.h>

static std::vector<std::tuple<std::string, std::string, rage::ResourceFlags>> g_customStreamingFiles;
std::set<std::string> g_customStreamingFileRefs;
static std::map<std::string, std::vector<std::string>, std::less<>> g_customStreamingFilesByTag;
static bool g_reloadStreamingFiles;

void DLL_EXPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags)
{
	auto baseName = std::string(strrchr(fileName.c_str(), '/') + 1);

	g_customStreamingFilesByTag[tag].push_back(fileName);
	g_customStreamingFiles.push_back({ fileName, tag, flags });
	g_customStreamingFileRefs.insert(baseName);

	g_reloadStreamingFiles = true;
}

void DLL_EXPORT CfxCollection_RemoveStreamingTag(const std::string& tag)
{
}

void DLL_EXPORT CfxCollection_BackoutStreamingTag(const std::string& tag)
{
}

void DLL_EXPORT CfxCollection_SetStreamingLoadLocked(bool locked)
{
}

void LoadStreamingFiles()
{
	for (auto it = g_customStreamingFiles.begin(); it != g_customStreamingFiles.end();)
	{
		auto [file, tag, flags] = *it;
		it = g_customStreamingFiles.erase(it);


	}
}

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		if (g_reloadStreamingFiles)
		{
			LoadStreamingFiles();

			g_reloadStreamingFiles = false;
		}
	});
});
