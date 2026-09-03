#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

// crmtComposer binary-operator stack underflow.
//
// Faulting instruction observed on game build 3258 at GTA5.exe+0x156FCC5:
//
//     lea  eax, [rsi - 1]                       ; rsi = operand-stack count
//     mov  r8d, [rcx + rax*4 + 0xA20]           ; <-- AV: reads table[-1]
//
// The target function is a "pop two, produce one" binary-operator factory
// in the animation-tree evaluator. It unconditionally decrements two
// parallel stack counters at [this+countA_offset] and [this+countB_offset]
// and indexes two parallel arrays with the pre-decremented values. When
// either stack is empty on entry, the -1 index sign-extends to 0xFFFFFFFF
// and the rcx+rax*4+disp math reads unmapped memory.
//
// Guard: if either operand stack is empty, skip the combine and return.
//
// The two counter displacements are disp32 immediates inside the matched
// prologue (`mov esi, [rcx+disp32]` / `mov edi, [rcx+disp32]`) and are
// read from the matched bytes rather than hardcoded so minor-revision
// layout tweaks don't silently desync the guard from the function body.

static uint32_t g_countAOffset = 0;
static uint32_t g_countBOffset = 0;

static void (*orig_CrmtComposerBinaryOp)(void* self, int op, float f, bool b,
                                         int a5, uint64_t a6);

static void CrmtComposerBinaryOp_Fix(void* self, int op, float f, bool b,
                                     int a5, uint64_t a6)
{
	const auto* s = static_cast<const uint8_t*>(self);
	const uint32_t countA = *reinterpret_cast<const uint32_t*>(s + g_countAOffset);
	const uint32_t countB = *reinterpret_cast<const uint32_t*>(s + g_countBOffset);

	if (countA == 0 || countB == 0)
	{
		return;
	}

	orig_CrmtComposerBinaryOp(self, op, f, b, a5, a6);
}

static HookFunction hookFunction([]()
{
	// Byte offsets of the disp32 operands within the matched prologue:
	//   ... 48 83 EC ??         ; 22..24 : sub rsp, imm8   (24 = imm8)
	//   8B B1 ?? ?? ?? ??       ; 25..30 : mov esi, [rcx+disp32]  (27..30)
	//   8B B9 ?? ?? ?? ??       ; 31..36 : mov edi, [rcx+disp32]  (33..36)
	constexpr std::size_t kDisp32_A = 27;
	constexpr std::size_t kDisp32_B = 33;

	auto* match = hook::get_pattern<uint8_t>(
		"4C 8B DC 49 89 5B 08 49 89 6B 10 49 89 73 18 49 89 7B 20 "
		"41 56 48 83 EC ? 8B B1 ? ? ? ? 8B B9 ? ? ? ?");

	const uint32_t dispA = *reinterpret_cast<const uint32_t*>(match + kDisp32_A);
	const uint32_t dispB = *reinterpret_cast<const uint32_t*>(match + kDisp32_B);

	// Check the extracted offsets. If the pattern matched a different function coincidentally,
	// the disp32s in its prologue would almost certainly fall outside the plausible counter-field range
	// then skip the hook install rather than patch the wrong function.
	if (dispA < 0x20 || dispA > 0x10000 || dispB < 0x20 || dispB > 0x10000)
	{
		return;
	}

	g_countAOffset = dispA;
	g_countBOffset = dispB;

	orig_CrmtComposerBinaryOp = hook::trampoline(match, CrmtComposerBinaryOp_Fix);
});
