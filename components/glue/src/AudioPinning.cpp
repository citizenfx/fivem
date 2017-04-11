#include <StdInc.h>
#include <Error.h>

static InitFunction initFunction([] ()
{
	HMODULE hXA8 = LoadLibrary(L"xaudio2_8.dll");

	// we don't have to pin if XAudio 2.8 is present as we're loading that ourselves
	if (hXA8)
	{
		return;
	}

    HMODULE hXA_pin;
    HMODULE hXA = LoadLibrary(L"xaudio2_7.dll");

    if (!hXA)
    {
        FatalError("Could not load xaudio2_7.dll.");
    }

    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, L"xaudio2_7.dll", &hXA_pin);

    if (hXA_pin != hXA)
    {
        trace("Failed to pin xaudio2_7.dll - error code %d\n", GetLastError());
    }
});