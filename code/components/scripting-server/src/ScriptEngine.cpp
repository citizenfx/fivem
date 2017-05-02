#include <StdInc.h>
#include <ScriptEngine.h>

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