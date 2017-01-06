#include <StdInc.h>

static InitFunction initFunction([] ()
{
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