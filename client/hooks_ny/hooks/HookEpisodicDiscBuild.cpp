#include "StdInc.h"

void WRAPPER AudioGetSongHook()
{
	__asm
	{
		test eax, eax
		jnz returnUsually

		mov eax, dword ptr ds:[166804Ch]

	returnUsually:
		retn 4
	}
}

static HookFunction hookFunction([] ()
{
	// some function that crashes relating to audio, test jump by returning just news/adverts/something
	//hook::nop(0xAC4018, 2);

	// a very stomp hook in the above function to prevent crashing by returning something valid instead of 0 (at least until we load episodic radio)
	hook::jump(0xAC4072, AudioGetSongHook);
});