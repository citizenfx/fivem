#include "StdInc.h"
#include "Hooking.h"

#include <Error.h>

#include <ICoreGameInit.h>

#include <Hooking.Stubs.h>
#include <jitasm.h>

#include <nutsnbolts.h>

namespace rage
{
struct sysMemSimpleAllocator;

struct sysCriticalSectionToken
{
	CRITICAL_SECTION Impl;

	void Lock()
	{
		EnterCriticalSection(&Impl);
	}

	void Unlock()
	{
		LeaveCriticalSection(&Impl);
	}
};

struct sysSmallocator
{
#ifdef GTA_FIVE
	struct Chunk
	{
		Chunk* Prev;
		Chunk* Next;
		int FreeCount;
		void* FirstFree;
		sysSmallocator* Owner;
		char padding[20];
	};

	Chunk* First;
	uint16_t EntrySize;
	uint16_t ChunkCount;
#elif IS_RDR3
	struct Chunk
	{
		uint64_t Status;
		Chunk* NextChunk;
		Chunk* PrevChunk;
		sysSmallocator* Owner;
		bool IsFull;
		char padding[15];
	};

	uint64_t Status;
	void* Sema;
	Chunk* HazardPtrs[4];
	sysCriticalSectionToken Token;
	Chunk* FreeChunks;
	Chunk* LastFreeChunk;
	Chunk* FullChunks;
	Chunk* LastFullChunk;
	Chunk* ActiveChunk;
	Chunk* EmptyChunks;
	sysMemSimpleAllocator* Owner;
	uint16_t NumChunks;
	uint16_t ChunkSize;
#endif
};

struct sysMemSimpleAllocator
{
	struct Node
	{
		uint32_t Guard;
		uint32_t Size;
		uint32_t PrevOffs;
		uint32_t AllocId : 26;
		uint32_t Visited : 1;
		uint32_t Used : 1;
		uint32_t Bucket : 4;

		uint8_t* GetData()
		{
			return (uint8_t*)this + sizeof(*this);
		}

		Node* GetPrevNode()
		{
			return PrevOffs ? (Node*)((uint8_t*)this - PrevOffs) : 0;
		}

		Node* GetNextNode()
		{
			return (Node*)(GetData() + Size);
		}

		bool CheckGuard()
		{
			// Repurpose AllocId as a second guard
			return (Guard == (uint32_t)(uintptr_t)this) && (!Used || (AllocId == 0x2adbeef));
		}
	};

	struct FreeNode : Node
	{
		FreeNode* PrevFree;
		FreeNode* NextFree;
	};

#ifdef GTA_FIVE
	static const size_t NumFreeLists = 32;
	static const size_t NumPageBits = 65544;
#elif IS_RDR3
	static const size_t NumFreeLists = 288;
	static const size_t NumPageBits = 262176;
#endif

	virtual ~sysMemSimpleAllocator() = 0;

	void* Heap;
	void* OrigHeap;

	FreeNode* FreeLists[NumFreeLists];
	uint64_t HeapSize;
	uint64_t OrigHeapSize;
	uint64_t MemoryUsed;
	uint64_t MemoryUsedByBucket[16];
	uint64_t MemoryUsedByBucketSnapshot[16];
	uint64_t MemoryAvailable;
	uint64_t LeastMemoryAvailable;
	uint32_t AllocIdByLayer[8];
	int32_t LayerCount;
	bool IsLocked;
	bool LockStackTracebacks;
	bool OwnHeap;
	bool EnableSmallocator;
	void* StackFile;
	int32_t AllocId;
	sysSmallocator SmallAllocator[8];
	void* PageBase;
	uint32_t PageArray[(NumPageBits + 31) / 32];

	uint8_t Flags;
	sysCriticalSectionToken Token;

	void Lock()
	{
		Token.Lock();
	}

	void Unlock()
	{
		Token.Unlock();
	}

	Node* GetStartNode()
	{
		return (Node*)Heap;
	}

	Node* GetEndNode()
	{
		return (Node*)((char*)Heap + HeapSize);
	}

	enum class SanityCheckReason
	{
		None,
		NodeAlreadyFree,
		NodeCorruptThis,
		NodeCorruptNext,
	};

	void DoSanityCheck(Node* node, SanityCheckReason reason);
	void SanityCheck();
};

}

static rage::sysMemSimpleAllocator* gameVirtualAllocatorSimple;

void rage::sysMemSimpleAllocator::SanityCheck()
{
	Lock();

	if (Heap)
	{
		DoSanityCheck(nullptr, SanityCheckReason::None);
	}

	Unlock();
}

void rage::sysMemSimpleAllocator::DoSanityCheck(rage::sysMemSimpleAllocator::Node* node, SanityCheckReason reason)
{
	static int counter = 0;

	const auto error_log = [&](auto format, auto... args) {
		auto msg = va(format, args...);
		AddCrashometry(va("heap_error_%i", counter++), "%s", msg);
		trace("Heap: %s\n", msg);
	};

	Node* const start = GetStartNode();
	Node* const end = GetEndNode();

	bool found = false;
	const char* error = nullptr;

	Node* last_seen[4]{};
	size_t num_seen = 0;

	const auto node_error = [&](auto msg) {
		error = msg;
	};

	for (Node *n = start, *prev = nullptr; n != end; prev = n, n = n->GetNextNode())
	{
		if (n > end)
		{
			node_error("Node after end");
			break;
		}

		last_seen[num_seen++ % std::size(last_seen)] = n;

		if (n == node)
		{
			found = true;
		}

		if (!n->CheckGuard())
		{
			node_error("Corrupt guard");
			break;
		}
		
		if (n->GetPrevNode() != prev)
		{
			node_error("Corrupt Prev");
			break;
		}

		if (!n->Used && (n != node) /* Our node isn't properly freed yet */)
		{
			if (auto prev_free = static_cast<FreeNode*>(n)->PrevFree)
			{
				if (prev_free < start || prev_free >= end)
				{
					node_error("Invalid PrevFree");
					break;
				}
			}
			
			if (auto next_free = static_cast<FreeNode*>(n)->NextFree)
			{
				if (next_free < start || next_free >= end)
				{
					node_error("Invalid NextFree");
					break;
				}
			}
		}
	}

	for (FreeNode* n : FreeLists)
	{
		if (error)
			break;

		FreeNode* prev = nullptr;

		for (; n; prev = n, n = n->NextFree)
		{
			if (n < GetStartNode() || n >= GetEndNode())
			{
				node_error("FreeNode out of bounds");
				break;
			}

			last_seen[num_seen++ % std::size(last_seen)] = n;

			if (!n->CheckGuard())
			{
				node_error("Corrupt guard");
				break;
			}

			if (n->PrevFree != prev)
			{
				node_error("Invalid PrevFree");
				break;
			}
		}
	}

	if (error)
	{
		error_log("Heap: size=%X, used=%X, start=%p, end=%p %s", HeapSize, MemoryUsed, (void*)start, (void*)end, error);

		for (size_t i = num_seen - std::min<size_t>(num_seen, std::size(last_seen)); i < num_seen; ++i)
		{
			auto n = last_seen[i % std::size(last_seen)];

			std::string data;

			for (int i = -16; i < 48; ++i)
			{
				if (i == 0)
					data.push_back('|');

				fmt::format_to(std::back_inserter(data), "{:02X}", n->GetData()[i]);
			}

			error_log("Node(%p): %4X %s [%s]", (void*)n, n->Size, n->Used ? "used" : "free", data);
		}
	}
	
	if (node)
	{
		error_log("Freeing Node(%p): size=%X, used=%i, prev=%p, next=%p", (void*)node, node->Size, node->Used, (void*)node->GetPrevNode(), (void*)node->GetNextNode());

		if (!found)
		{
			error_log("Node not found");
		}

		if (node->CheckGuard())
		{
			if (reason == SanityCheckReason::NodeAlreadyFree)
			{
				error_log("Node already freed");
			}

			if (auto prev = node->GetPrevNode())
			{
				if (prev >= node)
				{
					error_log("Prev node after current");
				}
				else if (prev < start)
				{
					error_log("Prev node before start");
				}
				else if (!prev->CheckGuard())
				{
					error_log("Corrupt prev guard");
				}
				else if (prev->GetNextNode() != node)
				{
					error_log("Corrupt prev->next");
				}
			}

			if (auto next = node->GetNextNode(); next != end)
			{
				if (next > end)
				{
					error_log("Next node after end");
				}
				else if (!next->CheckGuard())
				{
					error_log("Corrupt next guard");
				}
				else if (next->GetPrevNode() != node)
				{
					error_log("Corrupt next->prev");
				}
			}
		}
		else
		{
			error_log("Corrupt guard");
		}
	}

	if (error)
	{
		FatalError("Game Heap Corruption Detected");
	}
}

static bool EnableMemoryChecks = false;

__declspec(dllexport) void ValidateHeaps()
{
	if (EnableMemoryChecks)
	{
		if (!HeapValidate(GetProcessHeap(), 0, NULL))
		{
			FatalError("Process Heap Corruption Detected");
		}

		if (gameVirtualAllocatorSimple && gameVirtualAllocatorSimple->Heap)
		{
			gameVirtualAllocatorSimple->SanityCheck();
		}
	}
}

static void (*orig_sysMemSimpleAllocator__InitHeap)(rage::sysMemSimpleAllocator* self, void* heap, uint64_t heapSize, bool allowSmallAllocator);

static void sysMemSimpleAllocator__InitHeap(rage::sysMemSimpleAllocator* self, void* heap, uint64_t heapSize, bool allowSmallAllocator)
{
	allowSmallAllocator = allowSmallAllocator && !EnableMemoryChecks;
	orig_sysMemSimpleAllocator__InitHeap(self, heap, heapSize, allowSmallAllocator);
	self->AllocId = 0x2adbeef;
}

static void OnFreeNodeError(rage::sysMemSimpleAllocator* self, rage::sysMemSimpleAllocator::Node* node, rage::sysMemSimpleAllocator::SanityCheckReason reason)
{
	self->DoSanityCheck(node, reason);

	FatalError("Invalid Pointer Free");
}

static HookFunction hookFunction([]()
{
	EnableMemoryChecks = GetPrivateProfileInt(L"Game", L"EnableHeapValidation", 0, MakeRelativeCitPath(L"CitizenFX.ini").c_str()) != 0;

	if (EnableMemoryChecks)
	{
		OnMainGameFrame.Connect([] {
			static DWORD next_check = 0;

			if (GetTickCount() >= next_check)
			{
				ValidateHeaps();

				next_check = GetTickCount() + 10000;
			}
		});
	}

#ifdef GTA_FIVE
	gameVirtualAllocatorSimple = hook::get_address<rage::sysMemSimpleAllocator*>(hook::get_pattern<char>("48 8D 0D ? ? ? ? 41 B1 ? 45 33 C0 BA", 3));
	orig_sysMemSimpleAllocator__InitHeap = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F0 48 8B DA"), &sysMemSimpleAllocator__InitHeap);
#elif IS_RDR3
	gameVirtualAllocatorSimple = hook::get_address<rage::sysMemSimpleAllocator*>(hook::get_pattern<char>("4C 8D 35 ? ? ? ? 48 8B 0C F8", 3));
	orig_sysMemSimpleAllocator__InitHeap = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 89 51 ? 49 8B F8"), &sysMemSimpleAllocator__InitHeap);
#endif

	using Reason = rage::sysMemSimpleAllocator::SanityCheckReason;

	static struct FreeNodeStub : jitasm::Frontend
	{
		FreeNodeStub(Reason reason)
			: reason(reason)
		{}

		Reason reason;

		virtual void InternalMain() override
		{
			mov(rcx, rdi); // this
			mov(rdx, rbx); // node
			mov(r8, (uint32_t)reason); // reason

			mov(rax, reinterpret_cast<uintptr_t>(&OnFreeNodeError));
			jmp(rax);
		}
	} already_free(Reason::NodeAlreadyFree), corrupt_this(Reason::NodeCorruptThis), corrupt_next(Reason::NodeCorruptNext);

#ifdef GTA_FIVE
	char* ptr = hook::get_pattern<char>("F7 43 ? ? ? ? ? 75 ? B9");
	
	hook::call(ptr + 0xE, already_free.GetCode());
	hook::call(ptr + 0x21, corrupt_this.GetCode());
	hook::call(ptr + 0x75, corrupt_next.GetCode());
#elif IS_RDR3
	char* ptr = hook::get_pattern<char>("F7 46 ? ? ? ? ? 48 8D 5E");

	hook::call(ptr + 0x18, already_free.GetCode());
	hook::call(ptr + 0x31, corrupt_this.GetCode());
	hook::call(ptr + 0xA1, corrupt_next.GetCode());
#endif
});
