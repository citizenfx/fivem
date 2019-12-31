#pragma once

#include <json.hpp>

namespace fx
{
class MinModeManifest
{
public:
	inline MinModeManifest()
		: m_json(nlohmann::json::object())
	{

	}

	inline MinModeManifest(const nlohmann::json& json)
		: m_json(json)
	{
		json.get_to(m_strings);
	}

	inline bool IsEnabled()
	{
		return !m_json.empty();
	}

	inline std::string Get(const std::string& key, const std::string& defaultValue = "")
	{
		auto it = m_strings.find(key);

		return (it == m_strings.end()) ? defaultValue : it->second;
	}

	inline std::string GetRaw()
	{
		return m_json.dump();
	}

private:
	std::map<std::string, std::string> m_strings;
	nlohmann::json m_json;
};
}

#ifdef COMPILING_CORE
extern "C" DLL_EXPORT fx::MinModeManifest* CoreGetMinModeManifest();
extern "C" void DLL_EXPORT CoreSetMinModeManifest(const char* str);
#elif _WIN32
inline fx::MinModeManifest* CoreGetMinModeManifest()
{
	static fx::MinModeManifest*(*func)();

	if (!func)
	{
		func = (fx::MinModeManifest*(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreGetMinModeManifest");
	}

	return (!func) ? false : func();
}
#endif
