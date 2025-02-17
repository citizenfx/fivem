#include "StdInc.h"
#include <Hooking.h>
#include <mutex>

struct PatternPair;

enum class eSceneGraphPool
{
	FW_ENTITY_CONTAINER = 0,
	FW_FIXED_ENTITY_CONTAINER = 1,
	FW_SO_A_ENTITY_CONTAINER = 2,
	FW_EXTERIOR_SCENE_GRAPH_NODE = 3,
	FW_STREAMED_SCENE_GRAPH_NODE = 4,
	FW_INTERIOR_SCENE_GRAPH_NODE = 5,
	FW_ROOM_SCENE_GRAPH_NODE = 6,
	FW_PORTAL_SCENE_GRAPH_NODE = 7,
};

struct SceneGraphPoolData
{
	int classSize;
	int poolSize;
};

std::unordered_map<eSceneGraphPool, SceneGraphPoolData> pools = {
    {eSceneGraphPool::FW_ENTITY_CONTAINER, {48, 800}},
    {eSceneGraphPool::FW_FIXED_ENTITY_CONTAINER, {48, 2}}, // Max pool size is 64
    {eSceneGraphPool::FW_SO_A_ENTITY_CONTAINER, {40, 1000}}, // This pool size will apply to FW_STREAMED_SCENE_GRAPH_NODE
    {eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE, {80, 1}}, // This pool size can't be changed, due to asm inc instruction
    {eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE, {64, 1000}}, // This pool size is going to be overwritten by FW_SO_A_ENTITY_CONTAINER
    {eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE, {32, 150}},
    {eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE, {48, 256}},
    {eSceneGraphPool::FW_PORTAL_SCENE_GRAPH_NODE, {128, 800}}
};

int CalculateTotalSize(bool storage) {
	int total = 0;
	for (const auto& pool : pools) {
		total += storage ? (pool.second.poolSize * pool.second.classSize) : pool.second.poolSize;
	}
	return total;
}

struct PatternPair
{
	std::string_view pattern;
	int offset;
};

static HookFunction hookFunction([]()
{
	// +85
	assert(pools[eSceneGraphPool::FW_FIXED_ENTITY_CONTAINER].poolSize <= 64 && "The pool size of FW_FIXED_ENTITY_CONTAINER can't be greater than 64");
	auto initPoolsLocation = hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? BA ? ? ? ? B9"); // 48 8D B3 ? ? ? ? 48 8B FB
	auto assignStorageLocation = hook::get_pattern("48 8B C1 48 89 0D ? ? ? ? 48 81 C1");
	auto computeSceneNodeLocation = hook::get_pattern("4C 8B C1 0F B6 49 ? 85 C9");
	auto assignScratchBufferLocation = hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 48 89 91");
	auto clearVisDataLocation = hook::get_pattern("40 53 48 83 EC ? 48 8B D9 48 8B 89 ? ? ? ? 33 D2 41 B8");

	// Adjust the memory offsets of the allocated pools
	hook::put<uint32_t>((char*)initPoolsLocation + 88, pools[eSceneGraphPool::FW_ENTITY_CONTAINER].poolSize); // 800
	hook::put<uint32_t>((char*)initPoolsLocation + 98, pools[eSceneGraphPool::FW_ENTITY_CONTAINER].poolSize + pools[eSceneGraphPool::FW_FIXED_ENTITY_CONTAINER].poolSize); // 802
	hook::put<uint32_t>((char*)initPoolsLocation + 81, pools[eSceneGraphPool::FW_SO_A_ENTITY_CONTAINER].poolSize); // 1802
	hook::put<uint32_t>((char*)initPoolsLocation + 123, pools[eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE].poolSize + pools[eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE].poolSize); // 406

	const int poolStorageSize = CalculateTotalSize(true);
	const int poolFlagSize = CalculateTotalSize(false);

	// Change the size of poolStorage and poolFlags size
	hook::put<uint32_t>((char*)initPoolsLocation + 44, poolStorageSize);
	hook::put<uint32_t>((char*)initPoolsLocation + 34, poolStorageSize + poolFlagSize);

	// Change the pool size of each pool constructor
	hook::put<uint32_t>((char*)initPoolsLocation + 176, pools[eSceneGraphPool::FW_ENTITY_CONTAINER].poolSize);
	/*
	 * To apply the size in the constructor of FW_FIXED_ENTITY_CONTAINER we have to know that the value is being calculated based on an rdi calculation,
	 * above you can see the instruction "mov edi, 40h" and the argument is passed as "lea edx, [rdi-X]",
	 * we calculate this value X based on the pool size we want, by default the poolSize is 2, X is 3Eh because 64 - 2 = 62(3E)
	 */
	hook::put((char*)initPoolsLocation + 256, (uint8_t)(64 - pools[eSceneGraphPool::FW_FIXED_ENTITY_CONTAINER].poolSize));
	hook::put((char*)initPoolsLocation + 336, pools[eSceneGraphPool::FW_SO_A_ENTITY_CONTAINER].poolSize);
	// We skip the pool size of FW_EXTERIOR_SCENE_GRAPH_NODE because it can't be changed due to the limitations in the memory offsets when allocating the pools
	hook::put((char*)initPoolsLocation + 484, pools[eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE].poolSize);
	hook::put((char*)initPoolsLocation + 563, pools[eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE].poolSize);
	hook::put((char*)initPoolsLocation + 646, pools[eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE].poolSize);
	hook::put((char*)initPoolsLocation + 725, pools[eSceneGraphPool::FW_PORTAL_SCENE_GRAPH_NODE].poolSize);

	// Adjust offsets in AssignStorage function
	hook::put<uint32_t>((char*)assignStorageLocation + 13, pools[eSceneGraphPool::FW_ENTITY_CONTAINER].poolSize * pools[eSceneGraphPool::FW_ENTITY_CONTAINER].classSize); // 38400
	hook::put<uint8_t>((char*)assignStorageLocation + 27, static_cast<uint8_t>(pools[eSceneGraphPool::FW_FIXED_ENTITY_CONTAINER].poolSize * pools[eSceneGraphPool::FW_FIXED_ENTITY_CONTAINER].classSize)); // 96
	hook::put<uint32_t>((char*)assignStorageLocation + 45, pools[eSceneGraphPool::FW_SO_A_ENTITY_CONTAINER].poolSize * pools[eSceneGraphPool::FW_SO_A_ENTITY_CONTAINER].classSize); // 40000
	hook::put<uint8_t>((char*)assignStorageLocation + 59, static_cast<uint8_t>(pools[eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE].poolSize * pools[eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE].classSize)); // 80
	hook::put<uint32_t>((char*)assignStorageLocation + 70, pools[eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE].poolSize * pools[eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE].classSize); // 64000
	hook::put<uint32_t>((char*)assignStorageLocation + 84, pools[eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE].poolSize * pools[eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE].classSize); // 4800
	hook::put<uint32_t>((char*)assignStorageLocation + 98, pools[eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE].poolSize * pools[eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE].classSize); // 12288
	hook::put<uint32_t>((char*)assignStorageLocation + 113, pools[eSceneGraphPool::FW_PORTAL_SCENE_GRAPH_NODE].poolSize * pools[eSceneGraphPool::FW_PORTAL_SCENE_GRAPH_NODE].classSize); // 102400

	// Adjust offsets in ComputeSceneNode function
	hook::put<uint32_t>((char*)computeSceneNodeLocation + 50, pools[eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE].poolSize + pools[eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE].poolSize + pools[eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE].poolSize + pools[eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE].poolSize); // 1407
	hook::put<uint32_t>((char*)computeSceneNodeLocation + 90, pools[eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE].poolSize + pools[eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE].poolSize + pools[eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE].poolSize); // 1151
	hook::put<uint32_t>((char*)computeSceneNodeLocation + 113, pools[eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE].poolSize + pools[eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE].poolSize); // 1001

	// Adjust offsets in AssignScratchBuffer function
	int sceneNodeCount =
		pools[eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE].poolSize +
		pools[eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE].poolSize +
		pools[eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE].poolSize +
		pools[eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE].poolSize +
		pools[eSceneGraphPool::FW_PORTAL_SCENE_GRAPH_NODE].poolSize;
	hook::put<uint32_t>((char*)assignScratchBufferLocation + 28, sceneNodeCount * 4);
	hook::put<uint32_t>((char*)clearVisDataLocation + 20, sceneNodeCount * 4);

	// Change pool size of FIRST_ROOM_SCENE_NODE_INDEX
	
	std::initializer_list<PatternPair> roomSceneNodeIndexLocations = {
		{ "41 BC ? ? ? ? 0F BA E0 ? 73 ? 0F BA E0", 2 }, // AddVisibility
		{ "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 41 BB", 61 } // ProcessEntityContainer
	};
	for(auto& entry : roomSceneNodeIndexLocations)
	{
		hook::put<uint32_t>(hook::get_pattern(entry.pattern, entry.offset), 1151);
	}
});
