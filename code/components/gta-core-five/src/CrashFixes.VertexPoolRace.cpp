#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

static struct VertexSlotPopFixStub : jitasm::Frontend
{
	uintptr_t retryAddr = 0;
	uintptr_t continueAddr = 0;

	void InternalMain() override
	{
		//   48 8B 45 20     MOV RAX, [RBP+20h]
		//   89 4D B8        MOV [RBP-48h], ECX
		//   45 8D 43 01     LEA R8D, [R11+1]
		mov(rax, qword_ptr[rbp + 0x20]);
		mov(dword_ptr[rbp - 0x48], ecx);
		lea(r8d, dword_ptr[r11 + 0x1]);

		// Validate RAX before dereference
		bt(rax, 47);
		jc("stale_pointer");

		// Safe to dereference: MOV RCX, [RAX]
		mov(rcx, qword_ptr[rax]);

		mov(rax, continueAddr);
		jmp(rax);

		// atomic snapshot, same as the original CAS failure path.
		L("stale_pointer");
		mov(rax, retryAddr);
		jmp(rax);
	}
} g_vertexSlotPopFixStub;

static HookFunction hookFunction([]()
{
	auto callSite = hook::get_pattern<char>("48 8D 4B 30 48 8D 54 24 61 E8 ? ? FF FF");
	char* popFn = hook::get_call(callSite + 9);

	auto patchPoint = hook::range_pattern((uintptr_t)popFn, (uintptr_t)popFn + 0x100,
		"48 8B 45 20 89 4D B8 45 8D 43 01 48 8B 08").get_one().get<char>();

	auto cmpAfterCas = hook::range_pattern((uintptr_t)popFn, (uintptr_t)popFn + 0x100,
		"48 3B 55 E8 0F 85").get_one().get<char>();
	char* jnzInsn = cmpAfterCas + 4;                          // points to 0F 85
	int32_t rel32 = *(int32_t*)(jnzInsn + 2);                 // read rel32
	char* loopTop = jnzInsn + 6 + rel32;

	constexpr size_t kPatchSize = 14;  // 4 instructions, 14 bytes

	g_vertexSlotPopFixStub.retryAddr = (uintptr_t)loopTop;
	g_vertexSlotPopFixStub.continueAddr = (uintptr_t)(patchPoint + kPatchSize);

	hook::nop(patchPoint, kPatchSize);
	hook::jump(patchPoint, g_vertexSlotPopFixStub.GetCode());
});
