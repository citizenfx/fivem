#include "StdInc.h"
#include <Hooking.h>
#include <jitasm.h>
#include <mutex>

struct PatternPair;

enum class eSceneGraphPool : size_t
{
	FW_ENTITY_CONTAINER = 0,
	FW_FIXED_ENTITY_CONTAINER = 1,
	FW_SO_A_ENTITY_CONTAINER = 2,
	FW_EXTERIOR_SCENE_GRAPH_NODE = 3,
	FW_STREAMED_SCENE_GRAPH_NODE = 4,
	FW_INTERIOR_SCENE_GRAPH_NODE = 5,
	FW_ROOM_SCENE_GRAPH_NODE = 6,
	FW_PORTAL_SCENE_GRAPH_NODE = 7,
	COUNT
};

struct SceneGraphPoolData
{
	int classSize;
	int poolSize;
};

constexpr size_t POOL_COUNT = static_cast<size_t>(eSceneGraphPool::COUNT);
// Currently only FW_PORTAL_SCENE_GRAPH_NODE is editable
constexpr SceneGraphPoolData pools[POOL_COUNT] = {
	{48, 800},  // FW_ENTITY_CONTAINER
	{48, 2},    // FW_FIXED_ENTITY_CONTAINER(Max pool size is 64)
	{40, 1000}, // FW_SO_A_ENTITY_CONTAINER(This pool size will apply to FW_STREAMED_SCENE_GRAPH_NODE)
	{80, 1},    // FW_EXTERIOR_SCENE_GRAPH_NODE(This pool size can't be changed, due to asm inc instruction)
	{64, 1000}, // FW_STREAMED_SCENE_GRAPH_NODE (This pool size is going to be overwritten by FW_SO_A_ENTITY_CONTAINER)
	{32, 150},  // FW_INTERIOR_SCENE_GRAPH_NODE
	{48, 256},  // FW_ROOM_SCENE_GRAPH_NODE
	{128, 800}  // FW_PORTAL_SCENE_GRAPH_NODE
};

constexpr int FW_ENTITY_POOL_SIZE = pools[static_cast<size_t>(eSceneGraphPool::FW_ENTITY_CONTAINER)].poolSize;
constexpr int FW_FIXED_ENTITY_POOL_SIZE = pools[static_cast<size_t>(eSceneGraphPool::FW_FIXED_ENTITY_CONTAINER)].poolSize;
constexpr int FW_SO_A_ENTITY_POOL_SIZE = pools[static_cast<size_t>(eSceneGraphPool::FW_SO_A_ENTITY_CONTAINER)].poolSize;
constexpr int FW_EXTERIOR_POOL_SIZE = pools[static_cast<size_t>(eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE)].poolSize;
constexpr int FW_STREAMED_POOL_SIZE = pools[static_cast<size_t>(eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE)].poolSize;
constexpr int FW_INTERIOR_POOL_SIZE = pools[static_cast<size_t>(eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE)].poolSize;
constexpr int FW_ROOM_POOL_SIZE = pools[static_cast<size_t>(eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE)].poolSize;
constexpr int FW_PORTAL_POOL_SIZE = pools[static_cast<size_t>(eSceneGraphPool::FW_PORTAL_SCENE_GRAPH_NODE)].poolSize;

constexpr int FIRST_STREAMED_SCENE_NODE_INDEX = FW_EXTERIOR_POOL_SIZE;
constexpr int FIRST_INTERIOR_SCENE_NODE_INDEX = FIRST_STREAMED_SCENE_NODE_INDEX + FW_STREAMED_POOL_SIZE;
constexpr int FIRST_ROOM_SCENE_NODE_INDEX = FIRST_INTERIOR_SCENE_NODE_INDEX + FW_INTERIOR_POOL_SIZE;
constexpr int FIRST_PORTAL_SCENE_NODE_INDEX = FIRST_ROOM_SCENE_NODE_INDEX + FW_ROOM_POOL_SIZE;
constexpr int MAX_SCENE_NODE_COUNT = FIRST_PORTAL_SCENE_NODE_INDEX + FW_PORTAL_POOL_SIZE; // 2207
constexpr int STORED_SCREEN_QUAD_COUNT = MAX_SCENE_NODE_COUNT - FIRST_ROOM_SCENE_NODE_INDEX + 1; // 2207 - 1151 + 1 = 1057

int CalculateTotalSize(bool storage) {
	int total = 0;
	for (size_t i = 0; i < POOL_COUNT; ++i) {
		const auto& pool = pools[i];
		total += storage ? (pool.poolSize * pool.classSize) : pool.poolSize;
	}
	return total;
}

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

namespace rage
{
	struct alignas(16) Vec4V
	{
		float x, y, z, w;
	};
}

static alignas(128) rage::Vec4V screenQuadStorage[4][STORED_SCREEN_QUAD_COUNT];

static HookFunction hookFunction([]()
{
	auto initPoolsLocation = hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? BA ? ? ? ? B9");
	auto assignStorageLocation = hook::get_pattern("48 8B C1 48 89 0D ? ? ? ? 48 81 C1");
	auto assignScratchBufferLocation = hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 48 89 91");
	auto clearVisDataLocation = hook::get_pattern("40 53 48 83 EC ? 48 8B D9 48 8B 89 ? ? ? ? 33 D2 41 B8");
	auto clearTraversalDataLocation = hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B7 81 ? ? ? ? 48 8B F9 48 8B 89");

	const int poolStorageSize = CalculateTotalSize(true);
	const int poolFlagSize = CalculateTotalSize(false);

	// Change the size of poolStorage and poolFlags size
	hook::put<uint32_t>((char*)initPoolsLocation + 44, poolStorageSize);
	hook::put<uint32_t>((char*)initPoolsLocation + 34, poolStorageSize + poolFlagSize);
	
	hook::put((char*)initPoolsLocation + 725, FW_PORTAL_POOL_SIZE);

	// Adjust offsets in AssignStorage function
	hook::put<uint32_t>((char*)assignStorageLocation + 113, FW_PORTAL_POOL_SIZE * pools[static_cast<size_t>(eSceneGraphPool::FW_PORTAL_SCENE_GRAPH_NODE)].classSize); // 102400

	// Adjust offsets in AssignScratchBuffer function
	std::initializer_list<int> assignScratchBufferSizes = {
		160, 177, 194, 211, 228
	};
	for(auto& offset : assignScratchBufferSizes)
	{
		// Combine two 16 bit values into a single 32 bit DWORD:
		// - Upper 16 bits (bitSize)
		// - Size: 69 (Currently unknown value, but seems to be a constant)
		// Result: (bitSize << 16) | size
		hook::put<uint32_t>((char*)assignScratchBufferLocation + offset, (MAX_SCENE_NODE_COUNT << 16) | 69);
	}
	hook::put<uint32_t>((char*)assignScratchBufferLocation + 28, MAX_SCENE_NODE_COUNT * 4);
	hook::put<uint32_t>((char*)assignScratchBufferLocation + 54, STORED_SCREEN_QUAD_COUNT * 16);
	hook::put<uint32_t>((char*)assignScratchBufferLocation + 79, 276);
	
	hook::put<uint32_t>((char*)clearVisDataLocation + 20, MAX_SCENE_NODE_COUNT * 4);
	// Adjust screen quad count
	hook::put<uint32_t>((char*)clearTraversalDataLocation + 236, STORED_SCREEN_QUAD_COUNT * 16);
	hook::put<uint32_t>((char*)clearTraversalDataLocation + 269, STORED_SCREEN_QUAD_COUNT * 16);

	// ScreenQuadStorages
	{
		// GetGbufScreenQuadPair
		{
			auto location = hook::get_pattern("48 0F BF C2 48 8D 15", 0x4);
			static struct : jitasm::Frontend
			{
				uintptr_t continueLocation;

				virtual void InternalMain() override
				{
					movsx(rax, dx);
					mov(rdx, (uintptr_t)screenQuadStorage);
					
					mov(r8, continueLocation);
					jmp(r8);
				}
			} stub;
			stub.continueLocation = (uintptr_t)location + 7;
			hook::nop(location, 7);
			hook::jump(location, stub.GetCode());
		}
		
		// GetGBuffExteriorScreenQuad
		{
			auto location = hook::get_pattern("48 8D 15 ? ? ? ? 48 03 C0 0F 28 0C C2");
			static struct : jitasm::Frontend
			{
				uintptr_t continueLocation;

				virtual void InternalMain() override
				{
					mov(rdx, (uintptr_t)screenQuadStorage);
					
					mov(r8, continueLocation);
					jmp(r8);
				}
			} stub;
			stub.continueLocation = (uintptr_t)location + 7;
			hook::nop(location, 7);
			hook::jump_reg<3>(location, stub.GetCode());
		}

		// fwScanEntities::RunFromDependency
		{
			auto location = hook::get_pattern("4C 8D 05 ? ? ? ? 48 C1 E0 ? 41 C1 E9");
			static struct : jitasm::Frontend
			{
				uintptr_t continueLocation;

				virtual void InternalMain() override
				{
					mov(r8, (uintptr_t)screenQuadStorage);
					
					mov(rcx, continueLocation);
					jmp(rcx);
				}
			} stub;
			stub.continueLocation = (uintptr_t)location + 7;
			hook::nop(location, 7);
			hook::jump_rcx(location, stub.GetCode());
		}

		// fwScanNodes::Run
		{
			auto location = hook::get_pattern("48 8D 05 ? ? ? ? 4A 8B 94 CB");
			static struct : jitasm::Frontend
			{
				uintptr_t continueLocation;

				virtual void InternalMain() override
				{
					mov(rax, (uintptr_t)screenQuadStorage);
					
					mov(rdx, continueLocation);
					jmp(rdx);
				}
			} stub;
			stub.continueLocation = (uintptr_t)location + 7;
			hook::nop(location, 7);
			hook::jump(location, stub.GetCode());
		}

		// Patch STORED_SCREEN_QUAD_COUNT
		hook::put<uint32_t>(hook::get_pattern("41 B8 ? ? ? ? E8 ? ? ? ? 41 8B C6", 0x2), STORED_SCREEN_QUAD_COUNT); // fwScanNodes::Run
		hook::put<uint32_t>(hook::get_pattern("48 8D 80 ? ? ? ? 0F 29 01", 0x3), STORED_SCREEN_QUAD_COUNT * 16); // fwScanEntities::RunFromDependency
		auto scanNodesRunQuad = hook::get_pattern<char>("48 69 C9 ? ? ? ? 48 03 C8 48 8B C2 48 0B C1 83 E0 ? 75 ? B8");
		hook::put<uint32_t>(scanNodesRunQuad + 0x3, STORED_SCREEN_QUAD_COUNT * 16);
		hook::put<uint32_t>(scanNodesRunQuad + 0x16, STORED_SCREEN_QUAD_COUNT / 8);
		/*
		*  The value (0xFC) is derived from the offset difference between 
		*  screenQuads and screenQuadPairs in the compiled class layout.
		*  This is compiler-dependent and may change if the class structure is modified.
		*  This can't be calculated based in STORED_SCREEN_QUAD_COUNT
		*/
		hook::put<char>(scanNodesRunQuad + 0x1D, 0xFC);
	}

	// Force ids to be up 1807, this can be added to the debug menu as offset id to test
	// hook::put<uint32_t>(hook::get_pattern("49 81 C0 ? ? ? ? EB ? 4C 2B 05 ? ? ? ? 48 B8", 0x3), 1807);
});