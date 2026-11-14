#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"
#include <DirectXMath.h>

namespace rage {
	struct alignas(16) Vec3V
	{
		float x;
		float y;
		float z;
		float pad;

		Vec3V()
			: x(0), y(0), z(0), pad(NAN)
		{
		}

		Vec3V(float x, float y, float z)
			: x(x), y(y), z(z), pad(NAN)
		{
		}
	};
	using Mat34V = DirectX::XMMATRIX;

	struct phBoundComposite
	{
		char m_Padding[0x70];
		void** m_Bounds;
	};
	static_assert(offsetof(phBoundComposite, m_Bounds) == 0x70);

	struct phOptimizedBvhNode
	{
		__int16 m_AABBMin[3];
		__int16 m_AABBMax[3];
		unsigned __int16 m_NodeData;
		unsigned __int8 m_PolygonCount;
	};

	struct phOptimizedBvh
	{
		phOptimizedBvhNode* bvhNode;
		char pad[120];
	};

	struct ScalarV
	{
		DirectX::XMVECTOR v;
	};
}

static void (*g_origProcessSelfCollision)(void* self, rage::phBoundComposite* boundComposite, rage::Mat34V* a3, rage::Mat34V a4, void* phManifold, unsigned __int8* a6, unsigned __int8* a7, int a8, bool a9);
static void ProcessSelfCollision(void* self, rage::phBoundComposite* boundComposite, rage::Mat34V* a3, rage::Mat34V a4, void* phManifold, unsigned __int8* a6, unsigned __int8* a7, int a8, bool a9)
{
	if (!boundComposite || !boundComposite->m_Bounds)
	{
		return;
	}
	g_origProcessSelfCollision(self, boundComposite, a3, a4, phManifold, a6, a7, a8, a9);
}

static rage::ScalarV* (*g_origCalcClosestLeafDistance)(rage::ScalarV*, rage::Vec3V*, rage::phOptimizedBvh* bvh);

static rage::ScalarV* CalcClosestLeafDistance(rage::ScalarV* out, rage::Vec3V* point, rage::phOptimizedBvh* bvh)
{
	if (!bvh || !bvh->bvhNode)
	{
		out->v = DirectX::XMVectorSet(100000.0f, 10000.0f, 10000.0f, 1.0f);
		return out;
	}
	return g_origCalcClosestLeafDistance(out, point, bvh);
}

static HookFunction hookFunction([]()
{
	g_origProcessSelfCollision = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 4C 89 48 ? 4C 89 40 ? 48 89 50 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? B8"), ProcessSelfCollision);
	g_origCalcClosestLeafDistance = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 48 89 50 ? 55 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 49 8B 38"), CalcClosestLeafDistance);
});
