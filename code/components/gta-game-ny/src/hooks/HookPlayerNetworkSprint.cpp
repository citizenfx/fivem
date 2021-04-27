#include "StdInc.h"
#include "GameFlags.h"

static bool CTaskSimplePlayerMove__DoNetworkPlayerMove()
{
	return !GameFlags::GetFlag(GameFlag::NetworkWalkMode);
}

static void TapSprintIfNeeded(char* esi)
{
	if (CTaskSimplePlayerMove__DoNetworkPlayerMove())
	{
		esi[12938] = 1;
		esi[12941] = 1;
	}
}

static void __declspec(naked) TapSprintIfNeededStub()
{
	__asm
	{
		push esi
		call TapSprintIfNeeded
		add esp, 4h

		retn
	}
}

static HookFunction hookFunction([] ()
{
	auto _isInMpTutorialAddr = *(uintptr_t*)(hook::get_call(*hook::get_pattern<char*>("68 91 09 75 13", -4)) + 2);

	auto patterns = hook::pattern(fmt::sprintf("38 05 %02X %02X %02X %02X",
		_isInMpTutorialAddr & 0xFF,
		(_isInMpTutorialAddr >> 8) & 0xFF,
		(_isInMpTutorialAddr >> 16) & 0xFF,
		(_isInMpTutorialAddr >> 24) & 0xFF));

	if (patterns.size() > 0)
	{
		for (int i = 0; i < patterns.size(); i++)
		{
			auto take1 = patterns.get(i).get<unsigned char>(-9);

			if (*take1 == 0xE8)
			{
				hook::call(take1, CTaskSimplePlayerMove__DoNetworkPlayerMove);
			}
			else
			{
				auto take2 = take1 - 4;

				if (*take2 == 0xE8)
				{
					hook::call(take2, CTaskSimplePlayerMove__DoNetworkPlayerMove);
				}
				else
				{
					assert(!"not a match");
				}
			}

		}
	}

	// jump over some 'key_sprint' raw keyboard check which happens to define automatic tapping for the sprint button if lshift is used
	auto location = hook::get_pattern("C6 86 8A 32 00 00 01");
	hook::nop(location, 14);
	hook::call(location, TapSprintIfNeededStub);
});
