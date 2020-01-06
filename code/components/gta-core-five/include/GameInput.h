#pragma once

#if defined(COMPILING_GTA_CORE_FIVE)
#define GAMEINPUT_EXPORT DLL_EXPORT
#else
#define GAMEINPUT_EXPORT DLL_IMPORT
#endif

namespace game
{
	GAMEINPUT_EXPORT void SetBindingTagActive(const std::string& tag, bool active);

	GAMEINPUT_EXPORT void RegisterBindingForTag(const std::string& tag, const std::string& command, const std::string& languageDesc, const std::string& ioms, const std::string& ioParam);
}
