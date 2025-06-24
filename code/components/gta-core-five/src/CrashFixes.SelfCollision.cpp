#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"
#include <DirectXMath.h>
#include <jitasm.h>

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
	struct phBound
	{
		uint8_t m_Type;
		uint8_t m_Flags;
		uint8_t m_PartIndex;
		float m_RadiusAroundCentroid;
	};
	using Mat34V = DirectX::XMMATRIX;

	struct phBoundComposite : phBound {
		phBound** m_Bounds;
		rage::Mat34V* m_CurrentMatrices;
		rage::Mat34V* m_LastMatrices;
		rage::Vec3V* m_LocalBoxMinMaxs;
		uint32_t* m_TypeAndIncludeFlags;
		uint32_t* m_OwnedTypeAndIncludeFlags;
		uint16_t m_MaxNumBounds;
		uint16_t m_NumBounds;
	};

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
		rage::ScalarV* result = new rage::ScalarV();
		result->v = DirectX::XMVectorSet(100000.0f, 10000.0f, 10000.0f, 1.0f);
		return result;
	}
	return g_origCalcClosestLeafDistance(out, point, bvh);
}

static HookFunction hookFunction([]()
{
	g_origProcessSelfCollision = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 4C 89 48 ? 4C 89 40 ? 48 89 50 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? B8"), ProcessSelfCollision);
	g_origCalcClosestLeafDistance = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 48 89 50 ? 55 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 49 8B 38"), CalcClosestLeafDistance);

	// phManifold::RefreshContactPoints
	// 
	// This function is responsible for updating the contact points (manifold) between two physics instances (A and B) 
	// based on their current and previous transformation matrices. These contact points are essential for maintaining 
	// stable and accurate collision resolution over time.
	//
	// In some edge cases, instanceB can return a valid pointer with a null archetype (0x00000000), which would cause 
	// a crash when accessing the bound of instanceB. To avoid this, we patch the function to insert 
	// a conditional check: if instanceB or instanceB->archetype is null, we jump early to the failure handler.
	//
	// The original `jz` instruction is replaced with an unconditional `jmp`, and the logic is moved to a stub where 
	// both conditions are properly evaluated before jumping to success or fail locations.
	{
		auto location = hook::get_pattern<char>("48 8B 8B ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 48 8B 41 ? F3 0F 10 41");

		static struct : jitasm::Frontend
		{
			uintptr_t successLocation;
			uintptr_t failLocation;

			void Init(uintptr_t success, uintptr_t fail)
			{
				successLocation = success;
				failLocation = fail;
			}
			
			virtual void InternalMain() override
			{
				mov(rcx, qword_ptr[rbx+0xA8]); // rcx = instanceB
				test(rcx, rcx);                // if (instanceB == nullptr)
				jz("fail");

				mov(rax, qword_ptr[rcx+0x10]); // rax = instanceB->archetype
				test(rax, rax);                // if (instanceB->archetype == nullptr)
				jz("fail");

				mov(rdi, successLocation);
				jmp(rdi);

				L("fail");
				mov(rax, failLocation);
				jmp(rax);
			}
		} stub;
		stub.Init((uintptr_t)location + 16, (uintptr_t)location + 11);

		// Replace the conditional jump with an unconditional jmp to allow the stub to handle logic
		hook::nop(location + 10, 2);
		hook::put<char>(location + 11, 0xE9);

		hook::nop(location, 10);
		hook::jump(location, stub.GetCode());
	}
});
