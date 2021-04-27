#pragma once

#include <tlhelp32.h>

struct DisableToolHelpScope
{
	inline DisableToolHelpScope()
	{
		// write code to disable hooks
		static auto target = (char*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateToolhelp32Snapshot");
		VirtualProtect(target, 16, PAGE_EXECUTE_READWRITE, &oldProtect);

#ifdef _M_AMD64
		const uint8_t disableStub[] = { 0x48, 0xc7, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xc3 };
#elif defined(_M_IX86)
		const uint8_t disableStub[] = { 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xC2, 0x08, 0x00 };
#else
#error Unsupported architecture.
#endif

		memcpy(oldCode, target, sizeof(oldCode));
		memcpy(target, disableStub, sizeof(disableStub));
	}

	inline ~DisableToolHelpScope()
	{
		auto target = (char*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateToolhelp32Snapshot");
		memcpy(target, oldCode, sizeof(oldCode));

		VirtualProtect(target, 16, oldProtect, &oldProtect);
	}

private:
	uint8_t oldCode[16];
	DWORD oldProtect;
};

