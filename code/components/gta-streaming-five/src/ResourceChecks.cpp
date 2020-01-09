#include "StdInc.h"
#include "Hooking.h"

#include "Streaming.h"

#include <Error.h>

#include <atArray.h>

#define RAGE_FORMATS_IN_GAME

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#include <phBound.h>

#include <MinHook.h>

static int(*g_origInsertModule)(void*, void*);

static thread_local std::string g_currentStreamingName;

std::string GetCurrentStreamingName()
{
	return g_currentStreamingName;
}

class strStreamingModule
{
public:
	virtual ~strStreamingModule() {}

public:
	uint32_t baseIdx;
	uint32_t pad;
	uint32_t unk1;
	atArray<char> name;
};

static void CallBeforeStreamingLoad(strStreamingModule* strModule, uint32_t index, void* data)
{
	uint32_t moduleBase = strModule->baseIdx;

	g_currentStreamingName = streaming::GetStreamingNameForIndex(moduleBase + index);
}

static void CallAfterStreamingLoad(strStreamingModule* strModule, uint32_t index, void* data)
{
	g_currentStreamingName = "";
}

static int InsertStreamingModuleWrap(void* moduleMgr, void* strModule)
{
	void** vt = *(void***)strModule;
	
	struct StreamingOnLoadStub : public jitasm::Frontend
	{
		void* m_origFunc;

		StreamingOnLoadStub(void* origFunc)
			: m_origFunc(origFunc)
		{

		}

		virtual void InternalMain() override
		{
			push(rbx);
			push(rsi);
			push(rbp);
			push(rdi);

			sub(rsp, 0x28);

			mov(rbx, rcx); // streaming module
			mov(rsi, rdx); // index in module
			mov(rbp, r8);  // data pointer
			mov(rdi, r9);  // unknown

			mov(rax, (uintptr_t)CallBeforeStreamingLoad);
			call(rax);

			mov(rcx, rbx);
			mov(rdx, rsi);
			mov(r8, rbp);
			mov(r9, rdi);

			mov(rax, (uintptr_t)m_origFunc);
			call(rax);

			mov(rcx, rbx);
			mov(rdx, rsi);
			mov(r8, rbp);

			mov(rax, (uintptr_t)CallAfterStreamingLoad);
			call(rax);

			add(rsp, 0x28);

			pop(rdi);
			pop(rbp);
			pop(rsi);
			pop(rbx);

			ret();
		}
	};

	auto stub = new StreamingOnLoadStub(vt[6]);
	vt[6] = stub->GetCode();

	return g_origInsertModule(moduleMgr, strModule);
}

static void PolyErrorv(const std::string& str, fmt::printf_args args)
{
	trace("Physics validation failed for asset %s.\nThis asset is **INVALID**, but we've fixed it for this load. Please fix the exporter used to export it.\nDetails: %s\n",
		g_currentStreamingName, fmt::vsprintf(str, args));
}

template<typename... TArgs>
static inline void PolyError(const std::string& str, const TArgs&... args)
{
	PolyErrorv(str, fmt::make_printf_args(args...));
}

extern std::set<std::string> g_customStreamingFileRefs;

static void ValidateGeometry(void* geomPtr)
{
	// only validate #ft files for the time being.
	// #bn/#dr files tend to be exported with GIMS Evo, which inherently exports broken data
	// (edges point to vertices, not polygons; this is *plain wrong*)

	// also, slow PCs don't like validating collisions, that makes people fall through the ground

	// updated: check *any* file but only if it's a custom streaming asset
	// and we'll try fixing it
	if (g_currentStreamingName.find(".yft") == std::string::npos && g_customStreamingFileRefs.find(g_currentStreamingName) == g_customStreamingFileRefs.end())
	{
		return;
	}

	auto geom = (rage::five::phBoundGeometry*)geomPtr;
	auto polys = geom->GetPolygons();
	auto numPolys = geom->GetNumPolygons();
	auto numVerts = geom->GetNumVertices();

	bool error = false;

	for (size_t i = 0; i < numPolys; i++)
	{
		auto poly = &polys[i];

		if (poly->type == 0)
		{
#define CHECK_EDGE(e, a, b) \
	if (poly->poly.e != -1) \
	{ \
		if (poly->poly.e >= numPolys) \
		{ \
			if (!error) PolyError("Poly %d has edge %d which is > %d", i, poly->poly.e, numPolys); \
			error = true; \
			break; \
		} \
		\
		{ \
			auto& p = polys[poly->poly.e]; \
			if (p.type == 0) \
			{ \
				uint16_t v = (p.poly.v1 & 0x7FFF) + (p.poly.v2 & 0x7FFF) + (p.poly.v3 & 0x7FFF) - (poly->poly.a & 0x7FFF) - (poly->poly.b & 0x7FFF); \
				\
				if (v >= numVerts) \
				{ \
					if (!error) PolyError("Poly %d edge reference is invalid. It leads to vertex %d, when there are only %d vertices.", i, v, numVerts); \
					error = true; \
					break; \
				} \
			} \
		} \
	}

			CHECK_EDGE(e1, v1, v2);
			CHECK_EDGE(e2, v2, v3);
			CHECK_EDGE(e3, v1, v3);

#undef CHECK_EDGE
		}
	}

	if (error)
	{
		// recalculate poly neighbors
		struct PolyEdge
		{
			uint32_t edges[3];
		};

		std::vector<PolyEdge> outPolyEdges(numPolys);

		auto makeEdge = [](uint16_t a, uint16_t b)
		{
			if (a < b)
			{
				return (a << 16) | b;
			}
			else
			{
				return (b << 16) | a;
			}
		};

		struct PolyEdgeMap
		{
			uint32_t left;
			uint32_t right;
		};

		std::map<uint32_t, PolyEdgeMap> edgeMapping;
		auto outPolys = polys;

		for (int i = 0; i < numPolys; i++)
		{
			auto& outPoly = outPolys[i];

			// only type 0 has neighbors
			if (outPoly.type != 0)
			{
				continue;
			}

			PolyEdge edge;
			edge.edges[0] = makeEdge(outPoly.poly.v1, outPoly.poly.v2);
			edge.edges[1] = makeEdge(outPoly.poly.v2, outPoly.poly.v3);
			edge.edges[2] = makeEdge(outPoly.poly.v3, outPoly.poly.v1);
			outPolyEdges[i] = edge;

			for (int j = 0; j < 3; j++)
			{
				auto it = edgeMapping.find(edge.edges[j]);

				if (it == edgeMapping.end())
				{
					PolyEdgeMap map;
					map.left = i;
					map.right = -1;

					edgeMapping[edge.edges[j]] = map;
				}
				else
				{
					auto& edgeMap = it->second;

					if (edgeMap.right == -1)
					{
						edgeMap.right = i;
					}
				}
			}
		}

		auto findEdge = [&](int i, int edgeIdx) -> int16_t
		{
			auto& edge = outPolyEdges[i].edges[edgeIdx];
			auto& map = edgeMapping[edge];

			if (map.right == -1)
			{
				return -1;
			}
			else
			{
				if (map.left == i)
				{
					return map.right;
				}
				else
				{
					return map.left;
				}
			}
		};

		for (int i = 0; i < numPolys; i++)
		{
			auto& outPoly = outPolys[i];

			// only type 0 has neighbors
			if (outPoly.type != 0)
			{
				continue;
			}

			outPoly.poly.e1 = findEdge(i, 0);
			outPoly.poly.e2 = findEdge(i, 1);
			outPoly.poly.e3 = findEdge(i, 2);
		}
	}
}

static void(*g_origBVHThing)(void*, void*);

static void DoBVHThing(char* a1, void* a2)
{
	g_origBVHThing(a1, a2);

	ValidateGeometry(a1);
}

static void(*g_origGeometryThing)(void*, void*);

static void DoGeometryThing(char* a1, void* a2)
{
	g_origGeometryThing(a1, a2);

	ValidateGeometry(a1);
}

static HookFunction hookFunction([]()
{
	MH_Initialize();

	{
		auto insertModule = hook::get_pattern("76 16 48 8B 41 18 44 0F B7 41 20 4E", -15);
		MH_CreateHook(insertModule, InsertStreamingModuleWrap, (void**)&g_origInsertModule);
	}

	MH_EnableHook(MH_ALL_HOOKS);

	// phBoundBVH geometry check
	{
		auto location = hook::get_pattern("EB 4E 48 8B D1 48 8B CB E8", 8);
		hook::set_call(&g_origBVHThing, location);

		hook::call(location, DoBVHThing);
	}

	// phBoundGeometry check
	{
		auto location = hook::get_pattern("EB 4E 48 8B D1 48 8B CB E8", 21);
		hook::set_call(&g_origGeometryThing, location);

		hook::call(location, DoGeometryThing);
	}
});
