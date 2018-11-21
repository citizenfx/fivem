#pragma once

#include <memory>
#include <concurrent_unordered_set.h>

class
#ifdef COMPILING_SCRIPTING_GTA
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	NativeHandlerLogging
{
public:
	static void PushLog(const std::shared_ptr<concurrency::concurrent_unordered_set<uint64_t>>& ptr);

	static std::shared_ptr<concurrency::concurrent_unordered_set<uint64_t>> PopLog();

	static void CountNative(uint64_t identifier);
};
