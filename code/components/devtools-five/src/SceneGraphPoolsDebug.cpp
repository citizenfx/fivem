#include <StdInc.h>
#include <CoreConsole.h>
#include <ConsoleHost.h>
#include <imgui.h>
#include <Hooking.h>
#include <jitasm.h>

#include "Hooking.Stubs.h"
#include "ScriptEngine.h"
#include "ScriptInvoker.h"

namespace rage
{
	struct alignas(16) Vec4V
	{
		float x, y, z, w;
	};
	struct Vector3
	{
		float x, y, z;
		float __pad;
	};
	struct atUserBitSet
	{
		unsigned int *bits;
		unsigned __int16 size;
		unsigned __int16 bitSize;
		char m_Pad[4];
	};
	struct PortalEntry
	{
		void* portal;
		void* destination;
	};
	struct PortalStack
	{
		PortalEntry elements[192];
		int count;
		char m_Pad[4];
	};
	enum eSceneGraphNodeType : uint8_t
	{
		EXTERIOR,
		STREAMED,
		INTERIOR,
		ROOM,
		PORTAL
	};
	struct fwSceneGraphNode
	{
		fwSceneGraphNode* next;
		fwSceneGraphNode* child;
		eSceneGraphNodeType type;
		uint8_t flags;
		int16_t index;
		char m_pad[4];
	};
	struct fwScanNodes
	{
		Vec4V exteriorScreenQuad;
		Vector3 cameraPosition;
		
		PortalStack postalStack;
		void* scanBase;
		fwSceneGraphNode* rootNode;
		void* scanResults;
		void* zBuffer[2];
		unsigned int *visFlags;
		Vec4V* screenQuadsStorage[2];
		atUserBitSet beingVisited[2];
		atUserBitSet visited[2];
		atUserBitSet lodsOnly;
		uint8_t* bufferGuard;
	};
	struct fwPortalCorners
	{
		rage::Vector3 corners[4];
	};
	struct fwPortalSceneGraphNode : fwSceneGraphNode
	{
		uint8_t unk0[4];
		char m_pad[4];
		fwPortalCorners corners;
		void* container;
		void* interiorNode;
		fwSceneGraphNode* negativePortal;
		fwSceneGraphNode* positivePortal;
	};
	struct Color32 {
		uint32_t m_Color;
		constexpr Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
			: m_Color((uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b)) {}
	};
	struct Vector2
	{
		float x;
		float y;
	};
	struct fwBasePool
	{
		uint8_t* storage;
		uint8_t* flags;
		int size;
		int storageSize;
		int firstFreeIndex;
		int lastFreeIndex;
		int slotsUsed : 30;
		int ownsArray : 2;
		char pad[0x2C];
	};
}

static hook::cdecl_stub<void(rage::Vec4V*, rage::Vec4V*, rage::Vec4V*, rage::Color32 color)> drawPoly([]()
{
	return hook::get_call(hook::get_pattern("4C 8D 44 24 ? 0F B6 C1", 0x10));
});

static rage::fwScanNodes* scanNodes = nullptr;
static int scanNodesCount = 0;

static rage::fwPortalSceneGraphNode* nearestPortal = nullptr;
static rage::Vector3 nearestPortalPos = { 0.0f, 0.0f, 0.0f };
static float nearestPortalDist = 100.0f;

static bool drawPortals = true;
static float portalColor[4] = { 0.0f, 0.0f, 1.0f, 0.4f };

static void VisibilityBegin(rage::fwScanNodes* scanNodes)
{
	scanNodes = scanNodes;
}

static inline float DistSq(const rage::Vector3& a, const rage::Vector3& b)
{
	const float dx = a.x - b.x;
	const float dy = a.y - b.y;
	const float dz = a.z - b.z;
	return dx*dx + dy*dy + dz*dz;
}

static inline rage::Vector3 PortalCenter(const rage::fwPortalSceneGraphNode* p)
{
	const auto& c = p->corners.corners;
	return { (c[0].x + c[1].x + c[2].x + c[3].x) * 0.25f,
			 (c[0].y + c[1].y + c[2].y + c[3].y) * 0.25f,
			 (c[0].z + c[1].z + c[2].z + c[3].z) * 0.25f,
			 0.0f };
}

static void ConsiderPortalNearest(rage::fwPortalSceneGraphNode* portal, const rage::Vector3& refPos)
{
	const rage::Vector3 center = PortalCenter(portal);
	const float d2 = DistSq(center, refPos);
	if (d2 < nearestPortalDist)
	{
		nearestPortalDist = d2;
		nearestPortal = portal;
		nearestPortalPos = center;
	}
}

static void DrawPortal(rage::fwPortalSceneGraphNode portal, rage::Color32 color)
{
	if(!drawPortals)
		return;
	
	const auto& pc = portal.corners.corners;
	
	rage::Vec4V v0 = { pc[0].x, pc[0].y, pc[0].z, 1.0f };
	rage::Vec4V v1 = { pc[1].x, pc[1].y, pc[1].z, 1.0f };
	rage::Vec4V v2 = { pc[2].x, pc[2].y, pc[2].z, 1.0f };
	rage::Vec4V v3 = { pc[3].x, pc[3].y, pc[3].z, 1.0f };

	// Front Face
	drawPoly(&v0, &v1, &v2, color);
	drawPoly(&v0, &v2, &v3, color);

	// Back Face
	drawPoly(&v0, &v2, &v1, color);
    drawPoly(&v0, &v3, &v2, color);
}

static void TraverseGraphNode(rage::fwSceneGraphNode* node)
{
	while (node)
	{
		scanNodesCount++;
		
		switch (node->type)
		{
			case rage::PORTAL:
			{
				ConsiderPortalNearest(reinterpret_cast<rage::fwPortalSceneGraphNode*>(node), scanNodes->cameraPosition);
				rage::Color32 color(
					static_cast<uint8_t>(portalColor[0] * 255),
					static_cast<uint8_t>(portalColor[1] * 255),
					static_cast<uint8_t>(portalColor[2] * 255),
					static_cast<uint8_t>(portalColor[3] * 255)
				);
				DrawPortal(*reinterpret_cast<rage::fwPortalSceneGraphNode*>(node), color);
				break;
			}
			case rage::EXTERIOR:
			case rage::STREAMED:
			case rage::INTERIOR:
			case rage::ROOM:
				break;
		}

		if (node->child)
			TraverseGraphNode(node->child);

		node = node->next;
	}
}

static void DebugDrawGraphNode()
{
	if(scanNodes == nullptr)
	{
		return;
	}
	rage::fwSceneGraphNode* root = scanNodes->rootNode;
	scanNodesCount = 0;
	nearestPortalDist = 100.0f;
	
	TraverseGraphNode(root);
	if(nearestPortal)
	{
		rage::Color32 color(255, 0, 0, static_cast<uint8_t>(portalColor[3] * 255));
		DrawPortal(*nearestPortal, color);
	}
}

static void VisibilityEnd()
{
	DebugDrawGraphNode();
}

// This is to validate that beingVisited, visited, lodsOnly, etc bitsets are not being overflowed
static void ValidateScratchBufferGuard(rage::fwScanNodes* self)
{
	for (int i = 0; i < 16; ++i)
	{
		assert((self->bufferGuard[i] == 0xFE) && "Scratch buffer guard corruption detected");
	}
}

struct PoolEntry
{
	const char* name;
	rage::fwBasePool* pool;
};

static std::vector<PoolEntry> poolList;

static void (*g_origFwBasePool)(rage::fwBasePool*, int, uint8_t*, uint8_t*, const char*, int, int);
static void FwBasePool(rage::fwBasePool* pool, int poolSize, uint8_t* storage, uint8_t* scratchBuffer, const char* name, int classSize, int flags)
{
	poolList.push_back({ name, pool });
	g_origFwBasePool(pool, poolSize, storage, scratchBuffer, name, classSize, flags);
}

static int16_t (*g_origComputeSceneNodeIndex)(const void* sceneNode);
static int16_t ComputeSceneNodeIndex(const void* sceneNode)
{
	auto result = g_origComputeSceneNodeIndex(sceneNode);
	trace("Scene Node Index: %d\n", result);
	return result;
}

static HookFunction hookFunction([]()
{
	g_origComputeSceneNodeIndex = hook::trampoline(hook::get_pattern("4C 8B C1 0F B6 49 ? 85 C9"), ComputeSceneNodeIndex);
	g_origFwBasePool = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 89 1D ? ? ? ? EB ? 48 89 35 ? ? ? ? 48 8B 0D")), FwBasePool);
	// Visibility Being
	{
		auto location = hook::get_pattern<char>("44 88 8B ? ? ? ? 66 45 3B 88");
		
		static struct : jitasm::Frontend
		{
			uintptr_t continueLocation;

			virtual void InternalMain() override
			{
				mov(byte_ptr[rbx+0xCE1], r9b);

				mov(rcx, rbx);
				mov(r15, (uintptr_t)VisibilityBegin); // VisibilityBeing(this)
				call(r15);
				
				mov(r15, continueLocation);
				jmp(r15);
			}
		} stub;
		stub.continueLocation = (uintptr_t)location + 7;
		hook::nop(location, 7);
		hook::jump(location, stub.GetCode());
	}

	// Visibility End
	{
		auto location = hook::get_pattern<char>("48 8B 93 ? ? ? ? 48 8D 4D ? 48 81 C2 ? ? ? ? E8 ? ? ? ? 4C 8D 4C 24");

		static struct : jitasm::Frontend
		{
			uintptr_t continueLocation;

			virtual void InternalMain() override
			{
				mov(rdx, qword_ptr[rbx+0xC28]);
				
				mov(rcx, (uintptr_t)VisibilityEnd);
				call(rcx);
				
				mov(rcx, continueLocation);
				jmp(rcx);
			}
		} stub;
		stub.continueLocation = (uintptr_t)location + 7;
		hook::nop(location, 7);
		hook::jump_rcx(location, stub.GetCode());
	}

	// Asset scratch buffer guard
	{
		auto location = hook::get_pattern<char>("48 8D 4C 24 ? E8 ? ? ? ? B0 ? 48 81 C4 ? ? ? ? 5D");

		static struct : jitasm::Frontend
		{
			uintptr_t continueLocation;

			virtual void InternalMain() override
			{
				lea(rcx, dword_ptr[rsp+0x20]);
				mov(rax, (uintptr_t)ValidateScratchBufferGuard);
				call(rax);
				
				lea(rcx, dword_ptr[rsp+0x20]);
				mov(rax, continueLocation);
				jmp(rax);
			}
		} stub;
		stub.continueLocation = (uintptr_t)location + 5;
		hook::nop(location, 5);
		hook::jump_rcx(location, stub.GetCode());
	}
});

static InitFunction initFunction([]()
{
	static bool sceneGraphPoolsEnabled = false;
	static ConVar<bool> archetypeListVar("sceneGraphPools", ConVar_Archive | ConVar_UserPref, false, &sceneGraphPoolsEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || sceneGraphPoolsEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!sceneGraphPoolsEnabled)
		{
			return;
		}
		
		if (ImGui::Begin("Scene Graph Pools", &sceneGraphPoolsEnabled))
		{
			int totalPools = poolList.size();
			int totalSlots = 0;
			int totalUsed = 0;
			float maxUsage = -1.f, minUsage = 101.f;

			for (auto& entry : poolList)
			{
				totalSlots += entry.pool->size;
				totalUsed += entry.pool->slotsUsed;
				float usage = (float)entry.pool->slotsUsed / entry.pool->size * 100.0f;
			}

			float totalUsage = (float)totalUsed / totalSlots * 100.0f;

			ImGui::Text("Pools count: %d", totalPools);
			ImGui::Text("Total slots: %d", totalSlots);
			ImGui::Text("Used slots: %d (%.2f%%)", totalUsed, totalUsage);
			
			if (ImGui::BeginTable("Pools Data", 6,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 2.0f);
				ImGui::TableSetupColumn("Pool Size", ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGui::TableSetupColumn("Used Slots", ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGui::TableSetupColumn("Free Slots", ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGui::TableSetupColumn("Entry Size", ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGui::TableSetupColumn("Used Percentage", ImGuiTableColumnFlags_WidthStretch, 1.0f);
				ImGui::TableHeadersRow();

				for(int i = 0; i < poolList.size(); i++)
				{
					const auto& entry = poolList[i];
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("%s", entry.name);
					ImGui::TableSetColumnIndex(1); ImGui::Text("%d", entry.pool->size);
					ImGui::TableSetColumnIndex(2); ImGui::Text("%d", entry.pool->slotsUsed);
					ImGui::TableSetColumnIndex(3); ImGui::Text("%d", entry.pool->size - entry.pool->slotsUsed);
					ImGui::TableSetColumnIndex(4); ImGui::Text("%d", entry.pool->storageSize);
					ImGui::TableSetColumnIndex(5); ImGui::Text("%.2f%%", (float)entry.pool->slotsUsed / (float)entry.pool->size * 100.0f);
				}

				ImGui::EndTable();
			}
		}
		

		ImGui::End();
	});
});
