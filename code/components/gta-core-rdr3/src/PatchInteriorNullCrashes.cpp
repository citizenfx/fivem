#include "StdInc.h"

#include <jitasm.h>
#include "Hooking.Patterns.h"

static void ApplyInteriorEntityLocationFix()
{
	auto location = hook::get_pattern<char>("48 8B 8F E0 00 00 00 48 8D 54 24 48 E8 ? ? ? ? 48 8D 4C 24 48 E8 ? ? ? ? FF C0 83 F8 20 0F 87");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			mov(rcx, qword_ptr[rdi + 0xE0]); // original instruction

			test(rcx, rcx);
			jz("fail");

			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			mov(r11, failLocation);
			jmp(r11);
		}
	} stub;

	stub.Init((uintptr_t)location + 0x7, (uintptr_t)location + 0xC7);

	hook::nop(location, 7);
	hook::jump_rcx(location, stub.GetCode());

	// drop the increment and reject on greater-or-equal so the invalid room index is caught too
	hook::nop(location + 0x1B, 2);
	hook::put<uint8_t>(location + 0x21, 0x83);
}

static void ApplyEntityCompareLocationFix()
{
	// compares the other entity's location against its own
	auto location = hook::get_pattern<char>("48 8B 89 E0 00 00 00 E8 ? ? ? ? 48 8D 54 24 40 48 8B CF");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			mov(rcx, qword_ptr[rcx + 0xE0]); // original instruction

			test(rcx, rcx);
			jz("fail");

			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			mov(r11, failLocation);
			jmp(r11);
		}
	} stub;

	stub.Init((uintptr_t)location + 0x7, (uintptr_t)location + 0x50);

	hook::nop(location, 7);
	hook::jump(location, stub.GetCode());
}

static void ApplyEntityTypeFix()
{
	// reads the other entity's type before anything else, then gets location
	auto location = hook::get_pattern<char>("48 8B 81 E0 00 00 00 48 8B F9 80 78 30 05");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			mov(rax, qword_ptr[rcx + 0xE0]); // original instruction

			test(rax, rax);
			jz("fail");

			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			mov(r11, failLocation);
			jmp(r11);
		}
	} stub;

	stub.Init((uintptr_t)location + 0x7, (uintptr_t)location + 0xD3);

	hook::nop(location, 7);
	hook::jump(location, stub.GetCode());
}

static void ApplyInteriorRoomIndexFix()
{
	// the other arm of the same function uses the room index without bounding it or testing the
	// room it selects, and rooms are emptied while entities still reference them
	auto location = hook::get_pattern<char>("48 8B 86 48 01 00 00 48 8D 0C 49 48 C1 E1 05 48 8B 0C 01 48 8B 49 20");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			mov(rax, qword_ptr[rsi + 0x148]); // original instruction

			test(rax, rax);
			jz("fail");

			movzx(r11d, word_ptr[rsi + 0x150]);
			cmp(ecx, r11d);
			jae("fail");

			mov(r11d, ecx);
			lea(r11, qword_ptr[r11 + r11 * 2]);
			shl(r11, 5);
			mov(r11, qword_ptr[r11 + rax]);
			test(r11, r11);
			jz("fail");

			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			mov(r11, failLocation);
			jmp(r11);
		}
	} stub;

	stub.Init((uintptr_t)location + 0x7, (uintptr_t)location + 0x2D);

	hook::nop(location, 7);
	hook::jump(location, stub.GetCode());
}

static void ApplyEntityContainerIndexFix()
{
	// the node's second entity container is used without testing it; report no location instead
	auto location = hook::get_pattern<char>("48 8B 49 48 49 8B D0 E8 ? ? ? ? 8B C8 48 8B 47 60");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			mov(rcx, qword_ptr[rcx + 0x48]); // original instruction

			test(rcx, rcx);
			jz("fail");

			mov(rdx, r8); // original instruction

			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			// the caller leaves the location uninitialised, so write the same invalid one its own
			// failure paths write
			mov(dword_ptr[rbx], 0xFFFCFFFF);
			mov(r11, failLocation);
			jmp(r11);
		}
	} stub;

	stub.Init((uintptr_t)location + 0x7, (uintptr_t)location + 0x17);

	hook::nop(location, 7);
	hook::jump(location, stub.GetCode());
}

static void ApplyInteriorRoomIndexRemoveFix()
{
	// removing an entity from its interior uses the room index the same way
	auto location = hook::get_pattern<char>("48 8B 85 48 01 00 00 48 8D 0C 49 48 C1 E1 05 48 8B 0C 01 48 8B 49 20");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			mov(rax, qword_ptr[rbp + 0x148]); // original instruction

			test(rax, rax);
			jz("fail");

			movzx(r11d, word_ptr[rbp + 0x150]);
			cmp(ecx, r11d);
			jae("fail");

			mov(r11d, ecx);
			lea(r11, qword_ptr[r11 + r11 * 2]);
			shl(r11, 5);
			mov(r11, qword_ptr[r11 + rax]);
			test(r11, r11);
			jz("fail");

			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			mov(r11, failLocation);
			jmp(r11);
		}
	} stub;

	// the same exit this function's own location rejections use, so the entity flag is left alone
	stub.Init((uintptr_t)location + 0x7, (uintptr_t)location + 0xA5);

	hook::nop(location, 7);
	hook::jump(location, stub.GetCode());
}

static void ApplyInteriorRoomIndexRemoveFix2()
{
	// the same removal reached through the other entry point
	auto location = hook::get_pattern<char>("48 8B 87 48 01 00 00 48 8D 0C 49 48 C1 E1 05 48 8B 0C 01 48 8B 49 20");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			mov(rax, qword_ptr[rdi + 0x148]); // original instruction

			test(rax, rax);
			jz("fail");

			movzx(r11d, word_ptr[rdi + 0x150]);
			cmp(ecx, r11d);
			jae("fail");

			mov(r11d, ecx);
			lea(r11, qword_ptr[r11 + r11 * 2]);
			shl(r11, 5);
			mov(r11, qword_ptr[r11 + rax]);
			test(r11, r11);
			jz("fail");

			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			mov(r11, failLocation);
			jmp(r11);
		}
	} stub;

	stub.Init((uintptr_t)location + 0x7, (uintptr_t)location + 0xAC);

	hook::nop(location, 7);

	hook::jump(location, stub.GetCode());
}

static void ApplyAudioRoomIndexFix()
{
	// stores the room index instead of using it, the consumers dereference it without a test
	auto location = hook::get_pattern<char>("49 8B 86 48 01 00 00 48 8D 0C 49 48 C1 E1 05 48 8B 0C 01 48 89 8B E0 05 00 00");

	static struct : jitasm::Frontend
	{
		uintptr_t successLocation;
		uintptr_t failLocation;

		void Init(uintptr_t success, uintptr_t fail)
		{
			successLocation = success;
			failLocation = fail;
		}

		virtual void InternalMain() override
		{
			mov(rax, qword_ptr[r14 + 0x148]); // original instruction

			test(rax, rax);
			jz("fail");

			movzx(r11d, word_ptr[r14 + 0x150]);
			cmp(ecx, r11d);
			jae("fail");

			mov(r11d, ecx);
			lea(r11, qword_ptr[r11 + r11 * 2]);
			shl(r11, 5);
			mov(r11, qword_ptr[r11 + rax]);
			test(r11, r11);
			jz("fail");

			mov(r11, successLocation);
			jmp(r11);

			L("fail");
			mov(r11, failLocation);
			jmp(r11);
		}
	} stub;

	// the function already reports failure when it cannot resolve the interior
	stub.Init((uintptr_t)location + 0x7, (uintptr_t)location + 0x13A);

	hook::nop(location, 7);
	hook::jump(location, stub.GetCode());
}

static HookFunction hookFunction([]()
{
	ApplyInteriorEntityLocationFix();
	ApplyEntityCompareLocationFix();
	ApplyEntityTypeFix();
	ApplyInteriorRoomIndexFix();
	ApplyInteriorRoomIndexRemoveFix();
	ApplyInteriorRoomIndexRemoveFix2();
	ApplyAudioRoomIndexFix();
	ApplyEntityContainerIndexFix();
});
