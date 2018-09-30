#include <StdInc.h>
#include <ScriptEngine.h>
#include <ServerTime.h>

namespace fx
{
	static std::unordered_map<uint64_t, TNativeHandler> g_nativeHandlers;

	void ScriptEngine::RegisterNativeHandler(const std::string& nativeName, TNativeHandler function)
	{
		RegisterNativeHandler(HashString(nativeName.c_str()), function);
	}

	void ScriptEngine::RegisterNativeHandler(uint64_t nativeIdentifier, TNativeHandler function)
	{
		g_nativeHandlers.insert({ nativeIdentifier, function });
	}

	boost::optional<TNativeHandler> ScriptEngine::GetNativeHandler(uint64_t nativeIdentifier)
	{
		auto it = g_nativeHandlers.find(nativeIdentifier);

		return (it == g_nativeHandlers.end()) ? boost::optional<TNativeHandler>() : it->second;
	}
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_GAME_TIMER", [](fx::ScriptContext& ctx)
	{
		ctx.SetResult(msec().count() + 
#ifdef _DEBUG
			0x100000000
#else
			0
#endif
		);
	});
});
