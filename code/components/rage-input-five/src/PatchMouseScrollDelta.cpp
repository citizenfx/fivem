#include "StdInc.h"

#include <jitasm.h>
#include "Hooking.h"
#include <CrossBuildRuntime.h>

void DoPatchMouseScrollDelta()
{
	// remove the `/ 120` in window procedure code handling mouse wheel delta
	auto matches = hook::pattern("F7 E9 03 D1 C1 FA 06 8B C2 C1 E8 1F 03 D0 01").count(2);

	for (int i = 0; i < matches.size(); i++)
	{
		auto location = matches.get(i).get<char>(0);
		hook::nop(location, 14);
		hook::put<uint16_t>(location, 0xCA89); // mov edx, ecx
	}

	// same, but in DInput mouse wheel delta
	{
		auto location = hook::get_pattern<char>("B8 89 88 88 88 F7 2D");
		hook::nop(location, 0x1B);
		hook::nop(location + 0x2E, 6);
	}

	// shared divisor scale
	static alignas(16) const float scale[4] = { 1 / 120.f, 1 / 120.f, 1 / 120.f, 1 / 120.f };

	// rage::ioMapper::Update, int->float conversion for rage::ioMouse::m_dZ (add a / 120.0 there)
	{
		static struct : jitasm::Frontend
		{
			uintptr_t returnAddr = 0;

			virtual void InternalMain() override
			{
				mov(rax, (uintptr_t)(void*)scale);

				movaps(xmm0, xmmword_ptr[rax]);
				cvtdq2ps(xmm6, xmm6);
				mulps(xmm6, xmm0);

				mov(rax, returnAddr);
				jmp(rax);
			}
		} patch;

		auto location = hook::get_pattern<char>("0F 5B F6 E9 ? ? ? ? 8D 87");
		patch.returnAddr = hook::get_address<uintptr_t>(location + 3, 1, 5);
		hook::nop(location, 3 + 5);
		hook::jump(location, patch.GetCode());
	}

	// int 'stacking' for non-float parts (breaks sniper zoom otherwise)
	{
		static struct : jitasm::Frontend
		{
			uintptr_t dZAddr = 0;
			uintptr_t returnAddr = 0;

			virtual void InternalMain() override
			{
				// eax = *dZAddr
				mov(rax, dZAddr);
				mov(eax, dword_ptr[rax]);

				// ecx = 120
				mov(ecx, 120);

				// rax = (int64_t)eax
				cdq();

				// rax /= ecx
				idiv(ecx);

				// r8d = eax
				mov(r8d, eax);

				// return
				mov(rax, returnAddr);
				jmp(rax);
			}
		} patch;

		auto location = hook::get_pattern<char>("41 89 7E 20 44 8B 05", 4);
		patch.returnAddr = (uintptr_t)(location + 7);
		patch.dZAddr = hook::get_address<uintptr_t>(location, 3, 7);
		hook::nop(location, 7);
		hook::jump(location, patch.GetCode());
	}

	// adding / 120.0 to CGalleryMenu

	// 1st
	{
		static struct : jitasm::Frontend
		{
			uintptr_t returnAddr = 0;
			bool useXmm9 = false;

			virtual void InternalMain() override
			{
				mov(byte_ptr[rbp + 0x1C8], dl);

				mov(rax, (uintptr_t)(void*)scale);

				auto xmmReg = useXmm9 ? xmm9 : xmm8;

				movaps(xmm0, xmmword_ptr[rax]);
				cvtdq2ps(xmmReg, xmmReg);
				mulps(xmmReg, xmm0);

				mov(rax, returnAddr);
				jmp(rax);
			}
		} patch;

		auto location = hook::get_pattern<char>("48 8B 05 ? ? ? ? 40 38 74 24 ? 74 ? F3 44 0F 10 90", -0x2B);
		patch.returnAddr = (uintptr_t)(location + 10);
		xbr::IsGameBuildOrGreater<xbr::Build::Summer_2025>() ? patch.useXmm9 = true : patch.useXmm9 = false;
		hook::nop(location, 10);
		hook::jump(location, patch.GetCode());
	}

	// 2nd
	{
		static struct : jitasm::Frontend
		{
			uintptr_t returnAddr = 0;

			virtual void InternalMain() override
			{
				mov(byte_ptr[rsp + 0x50], cl);

				mov(rax, (uintptr_t)(void*)scale);

				movaps(xmm0, xmmword_ptr[rax]);
				cvtdq2ps(xmm6, xmm6);
				mulps(xmm6, xmm0);

				mov(rax, returnAddr);
				jmp(rax);
			}
		} patch;

		auto location = hook::get_pattern<char>("88 4C 24 50 0F 5B F6");
		patch.returnAddr = (uintptr_t)(location + 7);
		hook::nop(location, 7);
		hook::jump(location, patch.GetCode());
	}

	// adding / 120.0 to CMapMenu
	{
		static struct : jitasm::Frontend
		{
			uintptr_t returnAddr = 0;

			virtual void InternalMain() override
			{
				mov(rbx, rcx);

				mov(rax, (uintptr_t)(void*)scale);

				movaps(xmm3, xmmword_ptr[rax]);
				cvtdq2ps(xmm0, xmm0);
				mulps(xmm0, xmm3);

				mov(rax, returnAddr);
				jmp(rax);
			}
		} patch;

		auto location = hook::get_pattern<char>("48 8B D9 0F 5B C0 F3 0F");
		patch.returnAddr = (uintptr_t)(location + 6);
		hook::nop(location, 6);
		hook::jump(location, patch.GetCode());
	}

	// adding / 120.0 to scaleform input submission
	{
		static struct : jitasm::Frontend
		{
			uintptr_t returnAddr = 0;

			virtual void InternalMain() override
			{
				mov(qword_ptr[rbp - 0x29], r12);

				mov(rcx, (uintptr_t)(void*)scale);

				movaps(xmm3, xmmword_ptr[rcx]);
				cvtdq2ps(xmm0, xmm0);
				mulps(xmm0, xmm3);

				mov(rcx, returnAddr);

				// return jump to avoid using rcx after we restore it
				push(rcx);
				mov(rcx, rdi);

				ret();
			}
		} patch;

		auto location = hook::get_pattern<char>("4C 89 65 D7 0F 5B C0");
		patch.returnAddr = (uintptr_t)(location + 7);
		hook::nop(location, 7);
		hook::jump_rcx(location, patch.GetCode());
	}
}

// no HookFunction, this is in InputHook.cpp's setOffsetsHookFunction
