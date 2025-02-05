#include "StdInc.h"
#include <Hooking.h>

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
	eSceneGraphPool poolType;
	int classSize;
	int poolSize;
};

std::vector<SceneGraphPoolData> pools = {
	{eSceneGraphPool::FW_ENTITY_CONTAINER, 48, 800},
	{eSceneGraphPool::FW_FIXED_ENTITY_CONTAINER, 48, 2}, // Max pool size is 64
	{eSceneGraphPool::FW_SO_A_ENTITY_CONTAINER, 40, 1000}, // This pool size will apply to FW_STREAMED_SCENE_GRAPH_NODE
	{eSceneGraphPool::FW_EXTERIOR_SCENE_GRAPH_NODE, 80, 1}, // This pool size can't be changed, due to asm inc instruction
	{eSceneGraphPool::FW_STREAMED_SCENE_GRAPH_NODE, 64, 1000}, // This pool size is going to be overwritten by FW_SO_A_ENTITY_CONTAINER
	{eSceneGraphPool::FW_INTERIOR_SCENE_GRAPH_NODE, 32, 150},
	{eSceneGraphPool::FW_ROOM_SCENE_GRAPH_NODE, 48, 256},
	{eSceneGraphPool::FW_PORTAL_SCENE_GRAPH_NODE, 128, 800}
};

int CalculateTotalSize(bool storage) {
	int total = 0;
	for (const auto& pool : pools) {
		total += storage ? (pool.poolSize * pool.classSize) : pool.poolSize;
	}
	return total;
}

static HookFunction hookFunction([]()
{
	assert(pools[1].poolSize <= 64 && "The pool size of FW_FIXED_ENTITY_CONTAINER can't be greater than 64");
	auto initPoolsLocation = hook::get_pattern("48 8D B3 ? ? ? ? 48 8B FB");
	auto assignStorageLocation = hook::get_pattern("48 8B C1 48 89 0D ? ? ? ? 48 81 C1");

	// Adjust the memory offsets of the allocated pools
	hook::put<uint32_t>((char*)initPoolsLocation + 3, pools[0].poolSize); // 800
	hook::put<uint32_t>((char*)initPoolsLocation + 13, pools[0].poolSize + pools[1].poolSize); // 802
	hook::put<uint32_t>((char*)initPoolsLocation - 4, pools[2].poolSize); // 1802
	hook::put<uint32_t>((char*)initPoolsLocation + 38, pools[5].poolSize + pools[6].poolSize); // 406

	const int poolStorageSize = CalculateTotalSize(true);
	const int poolFlagSize = CalculateTotalSize(false);

	// Change the size of poolStorage and poolFlags size
	hook::put<uint32_t>((char*)initPoolsLocation - 41, poolStorageSize);
	hook::put<uint32_t>((char*)initPoolsLocation - 51, poolStorageSize + poolFlagSize);

	// Change the pool size of each pool constructor
	hook::put<uint32_t>((char*)initPoolsLocation + 91, pools[0].poolSize);
	/*
	 * To apply the size in the constructor of FW_FIXED_ENTITY_CONTAINER we have to know that the value is being calculated based on an rdi calculation,
	 * above you can see the instruction "mov edi, 40h" and the argument is passed as "lea edx, [rdi-X]",
	 * we calculate this value X based on the pool size we want, by default the poolSize is 2, X is 3Eh because 64 - 2 = 62(3E)
	 */
	hook::put((char*)initPoolsLocation + 168, (uint8_t)(64 - pools[1].poolSize));
	hook::put((char*)initPoolsLocation + 251, pools[2].poolSize);
	// We skip the pool size of FW_EXTERIOR_SCENE_GRAPH_NODE because it can't be changed due to the limitations in the memory offsets when allocating the pools
	hook::put((char*)initPoolsLocation + 399, pools[4].poolSize);
	hook::put((char*)initPoolsLocation + 478, pools[5].poolSize);
	hook::put((char*)initPoolsLocation + 561, pools[6].poolSize);
	hook::put((char*)initPoolsLocation + 640, pools[7].poolSize);

	// Adjust offsets in AssignStorage function
	hook::put<uint32_t>((char*)assignStorageLocation + 13, pools[0].poolSize * pools[0].classSize); // 38400
	hook::put<uint8_t>((char*)assignStorageLocation + 27, static_cast<uint8_t>(pools[1].poolSize * pools[1].classSize)); // 96
	hook::put<uint32_t>((char*)assignStorageLocation + 45, pools[2].poolSize * pools[2].classSize); // 40000
	hook::put<uint8_t>((char*)assignStorageLocation + 59, static_cast<uint8_t>(pools[3].poolSize * pools[3].classSize)); // 80
	hook::put<uint32_t>((char*)assignStorageLocation + 70, pools[4].poolSize * pools[4].classSize); // 64000
	hook::put<uint32_t>((char*)assignStorageLocation + 84, pools[5].poolSize * pools[5].classSize); // 4800
	hook::put<uint32_t>((char*)assignStorageLocation + 98, pools[6].poolSize * pools[6].classSize); // 12288
	hook::put<uint32_t>((char*)assignStorageLocation + 113, pools[7].poolSize * pools[7].classSize); // 102400
});
