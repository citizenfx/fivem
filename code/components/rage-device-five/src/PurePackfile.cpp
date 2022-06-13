#include <StdInc.h>
#include <Hooking.h>

#include <Error.h>

#include <PureModeState.h>

static bool (*g_validationRoutine)(const uint8_t* data, size_t size);

DLL_EXPORT void SetPackfileValidationRoutine(bool (*routine)(const uint8_t*, size_t))
{
	if (!g_validationRoutine)
	{
		g_validationRoutine = routine;
	}
}

static bool (*g_origPackfileOpen)(void* self, const char* path, bool a3, uint32_t a4, void* a5);

static bool PackfileOpen(char* self, const char* path, bool a3, uint32_t a4, void* a5)
{
	auto rv = g_origPackfileOpen(self, path, a3, a4, a5);

	if (rv)
	{
		if (g_validationRoutine)
		{
			auto entries = *(const uint8_t**)(self + 32);
			size_t numEntries = *(uint32_t*)(self + 40);

			if (!g_validationRoutine(entries, numEntries * 16))
			{
				FatalError("Invalid modified game files (%s)\nThe server you are trying to join has enabled 'pure mode', but you have modified game files. Please verify your GTA V installation (see http://rsg.ms/verify) and try again. Alternately, ask the server owner for help.", path);
			}
		}
	}

	return rv;
}

extern void* WrapPackfile(void*);

static HookFunction hookFunction([]
{
	if (fx::client::GetPureLevel() == 0)
	{
		return;
	}

	g_origPackfileOpen = (decltype(g_origPackfileOpen))WrapPackfile(PackfileOpen);
});
