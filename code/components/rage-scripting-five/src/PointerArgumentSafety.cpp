#include <StdInc.h>
#include <scrEngine.h>

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

#include "PASGen.h"

void PointerArgumentSafety()
{
	icgi = Instance<ICoreGameInit>::Get();
	icgi->OnSetVariable.Connect([](const std::string& name, bool value)
	{
		if (name == "storyMode")
		{
			storyMode = value;
		}
	});

	PointerArgumentSafety_Impl();
}
