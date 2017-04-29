#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "RenderCallbacks.h"

static std::multimap<std::string, void(*)()> g_callbacks;

void RenderCallbacks::AddRenderCallback(const char* name, void(*callback)())
{
	g_callbacks.insert(std::make_pair(name, callback));
}

static InitFunction initFunction([] ()
{
	g_hooksDLL->SetHookCallback(StringHash("renderCB"), [] (void* cbNamePtr)
	{
		const char* cbName = reinterpret_cast<const char*>(cbNamePtr);

		auto range = g_callbacks.equal_range(cbName);

		std::for_each(range.first, range.second, [] (std::pair<std::string, void(*)()> pair)
		{
			pair.second();
		});
	});
});