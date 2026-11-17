#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

#include <fiCollectionWrapper.h>

#ifdef GTA_FIVE
#include <string_view>
#include <unordered_map>

//
// rage::pgRawStreamer::GetEntryIndex linearly strcmps the normalized name against every known raw entry
// (1..count-1) before appending a new one, which makes registering N raw streaming files O(N^2) (~95k files on big servers).
//

namespace
{
constexpr int32_t kNormNameOffset = 0x20;
constexpr ptrdiff_t kNotFoundBranch = 0x12;
constexpr ptrdiff_t kFoundBranch = 0x4F;

struct RawStreamerIndex
{
	void* firstChunk = nullptr;
	uint32_t indexedUpTo = 1;
	std::unordered_map<std::string_view, int> byName;
};

std::unordered_map<rage::fiCollection*, RawStreamerIndex> g_indices;

void SyncIndex(rage::fiCollection* self, RawStreamerIndex& index)
{
	auto& entries = self->m_entries;
	const uint32_t count = entries.GetCount();
	void* firstChunk = entries.memory[0];

	if (index.firstChunk != firstChunk || count < index.indexedUpTo)
	{
		index.byName.clear();
		index.byName.reserve(4096);
		index.indexedUpTo = 1;
		index.firstChunk = firstChunk;
	}

	for (; index.indexedUpTo < count; index.indexedUpTo++)
	{
		const char* entryName = entries[index.indexedUpTo].fileName;

		if (!entryName)
		{
			break;
		}

		index.byName.emplace(std::string_view{ entryName }, int(index.indexedUpTo));
	}
}

int LookupEntry(rage::fiCollection* self, const char* normName)
{
	auto& index = g_indices[self];
	SyncIndex(self, index);

	if (auto it = index.byName.find(std::string_view{ normName }); it != index.byName.end())
	{
		return it->second;
	}

	return -1;
}

struct ScanLoopStub : jitasm::Frontend
{
	uintptr_t foundPath = 0;
	uintptr_t notFoundPath = 0;

	void Init(uintptr_t found, uintptr_t notFound)
	{
		foundPath = found;
		notFoundPath = notFound;
	}

	void InternalMain() override
	{
		mov(rbx, rax);

		mov(rcx, rdi);
		lea(rdx, qword_ptr[rsp + kNormNameOffset]);

		mov(rax, (uintptr_t)&LookupEntry);
		call(rax);

		test(eax, eax);
		js("notFound");

		mov(ebx, eax);
		mov(rcx, foundPath);
		jmp(rcx);

		L("notFound");

		mov(r8, rbx);
		mov(rcx, notFoundPath);
		jmp(rcx);
	}
};

inline uint8_t* GetShortBranchTarget(uint8_t* branch)
{
	return branch + 2 + *reinterpret_cast<int8_t*>(branch + 1);
}
}

static HookFunction hookFunction([]()
{
	auto loop = hook::get_pattern<uint8_t>("44 8B 97 ? ? ? ? BB");

	auto notFound = GetShortBranchTarget(loop + kNotFoundBranch);
	auto found = GetShortBranchTarget(loop + kFoundBranch);

	static ScanLoopStub stub;
	stub.Init(uintptr_t(found), uintptr_t(notFound));

	hook::nop(loop, 7);
	hook::jump_rcx(loop, stub.GetCode());
});
#endif
