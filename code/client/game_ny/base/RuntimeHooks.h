#pragma once

class GAMESPEC_EXPORT RuntimeHooks
{
public:
	static bool InstallRuntimeHook(const char* key);

	static bool SetLimit(const char* limit, int value);

	static bool SetWorldDefinition(const char* worldDefinition);
};