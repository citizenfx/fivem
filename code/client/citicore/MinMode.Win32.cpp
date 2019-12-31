#include <StdInc.h>
#include <MinMode.h>

static fx::MinModeManifest* g_manifest;

extern "C" void DLL_EXPORT CoreSetMinModeManifest(const char* str)
{
	try
	{
		g_manifest = new fx::MinModeManifest(nlohmann::json::parse(str));
	}
	catch (std::exception& e)
	{
		g_manifest = new fx::MinModeManifest();
	}
}

extern "C" DLL_EXPORT fx::MinModeManifest* CoreGetMinModeManifest()
{
	return g_manifest;
}
