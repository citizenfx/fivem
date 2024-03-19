#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

#include <CrossBuildRuntime.h>

#include <DirectXMath.h>

namespace rage
{
using Mat34V = DirectX::XMMATRIX;

class crSkeletonData;
class crSkeleton
{
public:
	crSkeletonData* m_skeletonData;
	Mat34V* m_parent;
	Mat34V* m_unknown;
	Mat34V* m_bones;
	uint32_t m_boneCount;
};
}

static void (*g_getGlobalMatrix)(rage::crSkeleton*, uint32_t, rage::Mat34V*);
static void GetGlobalMatrix(rage::crSkeleton* skeleton, uint32_t boneIndex, rage::Mat34V* outMatrix)
{
	if (skeleton && boneIndex != 0xFFFFFFFF)
	{
		g_getGlobalMatrix(skeleton, boneIndex, outMatrix);
	}
	else if (skeleton && skeleton->m_parent)
	{
		*outMatrix = *skeleton->m_parent;
	}
	else
	{
		*outMatrix = DirectX::XMMatrixIdentity();
	}
}

// Apparently this index parameter is signed: movsxd rax,edx
static void (*g_getGlobalTransform)(rage::crSkeletonData*, int, rage::Mat34V*, rage::Mat34V*);
static void GetGlobalTransform(rage::crSkeleton* skeleton, int boneIndex, rage::Mat34V* parent, rage::Mat34V* outMatrix)
{
	if (skeleton && boneIndex != -1)
	{
		g_getGlobalTransform(skeleton->m_skeletonData, boneIndex, parent, outMatrix);
	}
	else
	{
		*outMatrix = *parent;
	}
}

static bool (*g_getBoneIndexFromId)(rage::crSkeletonData*, uint16_t, int*);
static bool GetBoneIndexFromId(rage::crSkeleton* skeleton, uint16_t boneId, int* outIndex)
{
	if (skeleton)
	{
		return g_getBoneIndexFromId(skeleton->m_skeletonData, boneId, outIndex);
	}
	else
	{
		return false;
	}
}

static HookFunction hookFunction([]()
{
	// For unknown reasons some entities will have an invalid or unloaded
	// crSkeleton reference. Most places in game-code sanitize this, however
	// there are still few places in game-code that do not. Common assembly:
	//
	//		xor ecx,ecx
	//	LABEL:
	//		mov rcx,[rcx].
	//
	// Note, the replay editor paths have not been modified:
	//  48 8B 09 BA ? ? ? ? E8 ? ? ? ? 83 F8 FF
	//  48 8B 09 BA ? ? ? ? E8 ? ? ? ? ? 8B
	//  8B 5D D0 4C 8D 45 E0 8B D3 E8 ? ? ? ? 48 8B 07
	auto put_call = [](void* address, LPVOID funcStub)
	{
		hook::put<uint8_t>(address, 0xE8);
		hook::put<int>((uintptr_t)address + 1, (intptr_t)funcStub - (intptr_t)hook::get_adjusted(address) - 5);
	};

	// crSkeleton::_GetGlobalMatrix
	{
		std::initializer_list<std::tuple<std::string_view, ptrdiff_t>> list = {
			{ "4C 8B C7 33 D2 E8 ? ? ? ? 80 67 6D 1F", 0x5 },
			{ "4C 8D 45 20 41 8B D6 E8 ? ? ? ? 49 8B 4D 58", 0x7 },
			{ "BA ? ? ? ? 48 8B CB E8 ? ? ? ? 4C 8D 44 24 ? 48 8B CE", 0x17 },
		};

		LPVOID funcStub = hook::AllocateFunctionStub(GetGlobalMatrix);
		for (const auto& [pattern, offset] : list)
		{
			auto location = hook::get_pattern(pattern, offset);
			if (g_getGlobalMatrix == nullptr)
			{
				hook::set_call(&g_getGlobalMatrix, location);
			}
			put_call(location, funcStub);
		}
	}

	// crSkeletonData::_GetGlobalTransform
	{
		std::initializer_list<std::tuple<std::string_view, ptrdiff_t>> list = {
			{ "48 8B 09 4C 8D 4D D0 4C 8D 45 80 8B D6 E8", 0xD },
			{ "48 8B 09 4C 8D 4D D0 4C 8D 45 90 41 8B D6 E8", 0xE },
			{ "48 8B 09 4C 8D 4D C0 8B D6 E8", 0x9 },
		};

		LPVOID funcStub = hook::AllocateFunctionStub(GetGlobalTransform);
		for (const auto& [pattern, offset] : list)
		{
			auto location = hook::get_pattern<char>(pattern);
			if (g_getGlobalTransform == nullptr)
			{
				hook::set_call(&g_getGlobalTransform, location + offset);
			}
			hook::nop(location, 3); // mov rcx,[rcx]
			put_call(location + offset, funcStub);
		}
	}

	// GH-1563: GET_ENTITY_BONE_INDEX_BY_NAME is also affected by this. The
	// native is obfuscated so we need to jump to the call.
	{
		auto location = hook::get_pattern<char>("48 8B 09 4C 8D 44 24 ? 0F B7 D7 E9");
		auto jmp_loc = hook::get_address<char*>(location + 0xB + 1);
		hook::set_call(&g_getBoneIndexFromId, jmp_loc);

		hook::nop(location, 3); // mov rcx,[rcx]
		hook::call(jmp_loc, GetBoneIndexFromId);
	}

	// crSkeletonData::_GetBoneIndexFromId
	{
		std::initializer_list<std::tuple<std::string_view, ptrdiff_t, uint32_t>> list = {
			{ "48 8B 09 4C 8D 85 ? ? ? ? 33 D2 E8", 0xC, 4 },
			{ "48 8B 09 4C 8D 44 24 ? BA ? ? ? ? E8", 0xD, 2 },
			{ "48 8B 09 4C 8D 44 24 ? 0F B7 ? E8", 0xB, 2 },
			{ "48 8B 09 4C 8D 45 70 41 0F B7 D7 E8", 0xB, 1 },
			{ "48 8B 09 4C 8D 44 24 ? 33 D2 E8", 0xA, 1 },
		};

		LPVOID funcStub = hook::AllocateFunctionStub(GetBoneIndexFromId);
		for (const auto& [pattern, offset, expectedCount] : list)
		{
			auto matches = hook::pattern(pattern).count(expectedCount);
			for (size_t i = 0; i < matches.size(); ++i)
			{
				auto location = matches.get(i).get<char>();
				hook::nop(location, 3); // mov rcx,[rcx]
				put_call(location + offset, funcStub);
			}
		}
	}

#if 0
	// fwEntity::_GetGlobalMatrix: If required hook the dynamic-dispatch bits.
	{
		auto location = hook::get_pattern("B2 01 48 8B CF E8 ? ? ? ? 4C 8B C6", 0x26);
		hook::jump(location, GetGlobalMatrix);
	}
#endif
});
