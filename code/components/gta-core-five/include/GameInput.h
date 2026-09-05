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

	GAMEINPUT_EXPORT bool IsControlKeyDown(int control);

	GAMEINPUT_EXPORT void SetKeyMappingHideResources(bool hide);

	struct BindingKeyInfo
	{
		std::string command;
		std::string tag;
		std::string description;
		std::string keyName;
		std::string sourceName;
		uint32_t parameter;
		bool found;
		bool hasKey;
	};

	GAMEINPUT_EXPORT BindingKeyInfo GetBindingInfo(const std::string& command);

	GAMEINPUT_EXPORT bool RebindCommand(const std::string& command, const std::string& sourceName, const std::string& keyName);

	GAMEINPUT_EXPORT bool UnbindCommand(const std::string& command);
}
