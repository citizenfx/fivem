#include <StdInc.h>
#include <StatusText.h>

#include <Hooking.h>

static hook::cdecl_stub<void(const char*, int, int)> _activateStatus([]()
{
	return hook::get_pattern("48 6B C9 44 48 03 C8 39 11 75 20", -0x28);
});

static hook::cdecl_stub<void(int)> _deactivateStatus([]()
{
	return hook::get_pattern("48 6B C0 44 83 0C 08 FF", -0x16);
});

void ActivateStatusText(const char* string, int type, int priority)
{
	return _activateStatus(string, type, priority);
}

void DeactivateStatusText(int priority)
{
	return _deactivateStatus(priority);
}
