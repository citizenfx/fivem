#include "StdInc.h"

#include <ScriptEngine.h>
#include <NetLibrary.h>

static InitFunction initFunction([]()
{
	static NetLibrary* netLibrary;

	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* library)
	{
		netLibrary = library;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CURRENT_SERVER_ENDPOINT", [](fx::ScriptContext& context)
	{
		if (netLibrary && netLibrary->GetConnectionState() == NetLibrary::CS_ACTIVE)
		{
			static std::string addressString = netLibrary->GetCurrentPeer().ToString();
			context.SetResult(addressString.c_str());

			return;
		}

		context.SetResult(nullptr);
	});
});
