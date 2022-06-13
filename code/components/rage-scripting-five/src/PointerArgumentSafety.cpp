#include <StdInc.h>

#include "PointerArgumentHints.h"
#include "scrEngine.h"

#include <ICoreGameInit.h>

#include <psapi.h>

static ptrdiff_t GetMainImageSize()
{
	MODULEINFO mi;
	auto mainModule = GetModuleHandle(NULL);
	if (GetModuleInformation(GetCurrentProcess(), mainModule, &mi, sizeof(mi)))
	{
		return mi.SizeOfImage;
	}

	return 0x68000000;
}

static uintptr_t rangeStart = (uintptr_t)GetModuleHandle(NULL);
static uintptr_t rangeEnd = rangeStart + GetMainImageSize();
static ICoreGameInit* icgi;
bool storyMode;

static bool ValidateArg(void* arg)
{
	if (storyMode)
	{
		return true;
	}

	if ((uintptr_t)arg >= rangeStart && (uintptr_t)arg < rangeEnd)
	{
		return false;
	}

	return true;
}

static void NullifyAny(void*& arg)
{
	if (!storyMode)
	{
		arg = nullptr;
	}
}

static void NullifyVoid(rage::scrNativeCallContext* cxt)
{
	if (!storyMode)
	{
		cxt->SetResult(0, intptr_t(0));
	}
}

template<typename T>
static void ResultCleaner(void* results, fx::scripting::ResultType hint)
{
	using fx::scripting::ResultType;

	if constexpr (std::is_same_v<char*, std::decay_t<T>>)
	{
		if (hint != ResultType::String)
		{
			*(uintptr_t*)results = 0;
			return;
		}
	}
	else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<std::decay_t<T>, bool>)
	{
		if (hint == ResultType::String)
		{
			*(uintptr_t*)results = 0;
			return;
		}
	}
}

static std::unordered_map<uint64_t, decltype(&ResultCleaner<int>)> g_resultCleaners;

static void AddResultCleaner(uint64_t hash, decltype(g_resultCleaners)::mapped_type cleaner)
{
	g_resultCleaners[hash] = cleaner;
}

namespace fx::scripting
{
void PointerArgumentHints::CleanNativeResult(uint64_t nativeIdentifier, ResultType resultType, void* resultBuffer)
{
	if (resultType == fx::scripting::ResultType::None)
	{
		return;
	}

	if (auto cleaner = g_resultCleaners.find(nativeIdentifier); cleaner != g_resultCleaners.end())
	{
		cleaner->second(resultBuffer, resultType);
	}
	else if (auto handler = rage::scrEngine::GetNativeHandler(nativeIdentifier); !handler)
	{
		*(uintptr_t*)resultBuffer = 0;
	}
}
}

#include "PASGen.h"

void PointerArgumentSafety()
{
	PointerArgumentSafety_Impl();
}

static HookFunction hookFunction([]
{
	icgi = Instance<ICoreGameInit>::Get();
	icgi->OnSetVariable.Connect([](const std::string& name, bool value)
	{
		if (name == "storyMode")
		{
			storyMode = value;
		}
	});
});
