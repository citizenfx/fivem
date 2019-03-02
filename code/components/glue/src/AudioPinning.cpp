#include <StdInc.h>
#include <Error.h>

static InitFunction initFunction([] ()
{
	using namespace std::string_literals;

	wchar_t sysDir[MAX_PATH];
	GetSystemDirectoryW(sysDir, _countof(sysDir));

    HMODULE hXA_pin;
    HMODULE hXA = LoadLibrary((sysDir + L"\\xaudio2_7.dll"s).c_str());

    if (!hXA)
    {
		return;
    }

    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, L"xaudio2_7.dll", &hXA_pin);

    if (hXA_pin != hXA)
    {
        trace("Failed to pin xaudio2_7.dll - error code %d\n", GetLastError());
    }
});
