#include <StdInc.h>

#include <CoreConsole.h>

#include <DrawCommands.h>
#include <ScriptEngine.h>

#include <Hooking.h>

struct VertexPosition
{
	float x;
	float y;
};

struct VertexUv
{
	float x;
	float y;
};

struct VertexRect
{
	float x;
	float y;
	float z;
	float w;

	inline VertexRect()
		: x(0), y(0), z(0), w(0)
	{

	}

	inline VertexRect(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w)

	{

	}

	inline bool IsEmpty() const
	{
		return (x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f);
	}
};

struct DrawEntry
{
	std::vector<VertexPosition> positions;
	std::vector<VertexUv> uvs;
	std::vector<uint32_t> colors;

	std::string txd;
	std::string txn;

	VertexRect rect;

	inline DrawEntry(uint32_t vertexCount, const VertexPosition* positions, const uint32_t* colors = nullptr, const char* txd = nullptr, const char* txn = nullptr, const VertexUv* uvs = nullptr, const VertexRect& rect = {})
	{
		this->positions.resize(vertexCount);
		memcpy(this->positions.data(), positions, vertexCount * sizeof(VertexPosition));

		if (colors)
		{
			this->colors.resize(vertexCount);
			memcpy(this->colors.data(), colors, vertexCount * sizeof(uint32_t));
		}

		if (uvs)
		{
			this->uvs.resize(vertexCount);
			memcpy(this->uvs.data(), uvs, vertexCount * sizeof(VertexUv));
		}

		if (txd && txn)
		{
			this->txd = txd;
			this->txn = txn;
		}

		this->rect = rect;
	}

	void Render() const;
};

struct DrawList
{
	std::list<DrawEntry> entries;

	void Render() const;
};

rage::grcTexture* LookupTexture(const std::string& txd, const std::string& txn);

void DrawEntry::Render() const
{
	if (!uvs.empty() && !txd.empty() && !txn.empty())
	{
		SetTextureGtaIm(LookupTexture(txd, txn));
	}
	else
	{
		// todo: some white texture
		SetTextureGtaIm(nullptr);
		//SetTextureGtaIm(LookupTexture(txd, txn));
	}

	PushDrawBlitImShader();

	rage::grcBegin(3, positions.size());

	for (int i = 0; i < positions.size(); i++)
	{
		rage::grcVertex(positions[i].x, positions[i].y, 0.0f, 0.0f, 0.0f, -1.0f, colors.empty() ? 0xFFFFFFFF : colors[i], uvs.empty() ? 0.0f : uvs[i].x, uvs.empty() ? 0.0f : uvs[i].y);
	}

	if (rect.IsEmpty())
	{
		D3D11_RECT scissorRect;
		scissorRect.left = 0.0f;
		scissorRect.top = 0.0f;
		scissorRect.right = 1.0f;
		scissorRect.bottom = 1.0f;

		GetD3D11DeviceContext()->RSSetScissorRects(0, NULL);
	}
	else
	{
		// set scissor rects here, as they might be overwritten by a matrix push
		D3D11_RECT scissorRect;
		scissorRect.left = rect.x;
		scissorRect.top = rect.y;
		scissorRect.right = rect.z;
		scissorRect.bottom = rect.w;

		GetD3D11DeviceContext()->RSSetScissorRects(1, &scissorRect);
	}

	rage::grcEnd();

	PopDrawBlitImShader();
}

void DrawList::Render() const
{
	auto oldRasterizerState = GetRasterizerState();
	SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

	auto oldBlendState = GetBlendState();
	SetBlendState(GetStockStateIdentifier(BlendStateDefault));

	auto oldDepthStencilState = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

	for (const auto& entry : entries)
	{
		entry.Render();
	}

	SetRasterizerState(oldRasterizerState);

	SetBlendState(oldBlendState);

	SetDepthStencilState(oldDepthStencilState);

	{
		D3D11_RECT scissorRect;
		scissorRect.left = 0.0f;
		scissorRect.top = 0.0f;
		scissorRect.right = 1.0f;
		scissorRect.bottom = 1.0f;

		GetD3D11DeviceContext()->RSSetScissorRects(1, &scissorRect);
	}
}

static DrawList* g_drawList;

static void DoDrawImCommand(uint32_t vertexCount, const VertexPosition* positions, const uint32_t* colors = nullptr, const char* txd = nullptr, const char* txn = nullptr, const VertexUv* uvs = nullptr, const VertexRect& rect = {})
{
	auto dl = (DrawList*)InterlockedCompareExchangePointer((void**)&g_drawList, nullptr, nullptr);

	if (!dl)
	{
		dl = new DrawList();
		InterlockedExchangePointer((void**)&g_drawList, dl);
	}

	dl->entries.emplace_back(vertexCount, positions, colors, txd, txn, uvs, rect);
}

static void DoDrawCommands()
{
	auto drawList = (DrawList*)InterlockedExchangePointer((void**)&g_drawList, nullptr);

	if (drawList)
	{
		if (IsOnRenderThread())
		{
			drawList->Render();

			delete drawList;
		}
		else
		{
			uintptr_t argRef = (uintptr_t)drawList;

			EnqueueGenericDrawCommand([](uintptr_t a, uintptr_t b)
			{
				auto dl = ((DrawList*)a);
				dl->Render();

				delete dl;
			}, &argRef, &argRef);
		}
	}
}

static void(*g_origThing)(void*);

static void WrapThing(void* a1)
{
	g_origThing(a1);

	DoDrawCommands();
}

static InitFunction initFunction([]
{
	fx::ScriptEngine::RegisterNativeHandler("DRAW_IM_VERTICES", [](fx::ScriptContext & context)
	{
		auto count = context.GetArgument<int>(0);
		auto positions = context.CheckArgument<VertexPosition*>(1);
		auto colors = context.GetArgument<uint32_t*>(2);

		DoDrawImCommand(count, positions, colors);
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_TEXTURED_IM_VERTICES", [](fx::ScriptContext & context)
	{
		auto count = context.GetArgument<int>(0);
		auto positions = context.CheckArgument<VertexPosition*>(1);
		auto uvs = context.CheckArgument<VertexUv*>(2);
		auto txd = context.CheckArgument<const char*>(3);
		auto txn = context.CheckArgument<const char*>(4);
		auto colors = context.GetArgument<uint32_t*>(5);

		DoDrawImCommand(count, positions, colors, txd, txn, uvs);
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_TEXTURED_IM_VERTICES_CLIPPED", [](fx::ScriptContext & context)
	{
		auto count = context.GetArgument<int>(0);
		auto positions = context.CheckArgument<VertexPosition*>(1);
		auto uvs = context.CheckArgument<VertexUv*>(2);
		auto txd = context.CheckArgument<const char*>(3);
		auto txn = context.CheckArgument<const char*>(4);
		auto colors = context.GetArgument<uint32_t*>(5);

		VertexRect clipRect = {
			context.GetArgument<float>(6),
			context.GetArgument<float>(7),
			context.GetArgument<float>(8),
			context.GetArgument<float>(9),
		};

		DoDrawImCommand(count, positions, colors, txd, txn, uvs, clipRect);
	});
});

static HookFunction hookFunction([]
{
	// 1604
	auto location = 0x1404F48C1;
	hook::set_call(&g_origThing, location);
	hook::call(location, WrapThing);
});

static rage::grmShaderFx* g_default;
static int g_sampler;

static const float g_vertex_buffer_data[] = {
	-1.0f,-1.0f,-1.0f, // triangle 1 : begin
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f, // triangle 1 : end
	1.0f, 1.0f,-1.0f, // triangle 2 : begin
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f, // triangle 2 : end
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f
};

static float g_normal_buffer_data[std::size(g_vertex_buffer_data)];

static const float g_uv_buffer_data[] = {
	0.000059f, 0.000004f,
	0.000103f, 0.336048f,
	0.335973f, 0.335903f,
	1.000023f, 0.000013f,
	0.667979f, 0.335851f,
	0.999958f, 0.336064f,
	0.667979f, 0.335851f,
	0.336024f, 0.671877f,
	0.667969f, 0.671889f,
	1.000023f, 0.000013f,
	0.668104f, 0.000013f,
	0.667979f, 0.335851f,
	0.000059f, 0.000004f,
	0.335973f, 0.335903f,
	0.336098f, 0.000071f,
	0.667979f, 0.335851f,
	0.335973f, 0.335903f,
	0.336024f, 0.671877f,
	1.000004f, 0.671847f,
	0.999958f, 0.336064f,
	0.667979f, 0.335851f,
	0.668104f, 0.000013f,
	0.335973f, 0.335903f,
	0.667979f, 0.335851f,
	0.335973f, 0.335903f,
	0.668104f, 0.000013f,
	0.336098f, 0.000071f,
	0.000103f, 0.336048f,
	0.000004f, 0.671870f,
	0.336024f, 0.671877f,
	0.000103f, 0.336048f,
	0.336024f, 0.671877f,
	0.335973f, 0.335903f,
	0.667969f, 0.671889f,
	1.000004f, 0.671847f,
	0.667979f, 0.335851f
};

#include <DirectXMath.h>

#define RAGE_FORMATS_IN_GAME

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#include <phBound.h>

static hook::cdecl_stub<void(void*)> _waitAsReader([]()
{
	return (void*)0x1403385E8;
});

static hook::cdecl_stub<void(void*)> _releaseAsReader([]()
{
	return (void*)0x140331664;
});

static hook::cdecl_stub<uint16_t(void*, void*)> _getFirstCulledInstance([]()
{
	return (void*)0x14145AC54;
});

static hook::cdecl_stub<uint16_t(void*, void*)> _getNextCulledInstance([]()
{
	return (void*)0x14145AF60;
});

namespace rage
{
	class phMultiReaderLockToken
	{
	public:
		void WaitAsReader();
		void WaitAsWriter();
		void ReleaseAsReader();
		void ReleaseAsWriter();
	};

	void phMultiReaderLockToken::WaitAsReader()
	{
		return _waitAsReader(this);
	}

	void phMultiReaderLockToken::ReleaseAsReader()
	{
		return _releaseAsReader(this);
	}

	// 1604
	phMultiReaderLockToken* g_GlobalPhysicsLock = (phMultiReaderLockToken*)0x142CDD1E0;

	class phLevel;

	// 1604
	phLevel** g_level = (phLevel**)0x14248F668;

	class phIterator
	{
	private:
		alignas(16) rage::five::Vector3 center;
		alignas(16) rage::five::Vector3 size;
		char pad_20h[40];
		int m_48h; // 0
		char pad_4Ch[4];
		int fuckifiknow; // -1
		int padpadpad; // pad
		int mask; // -1?
		int m_5Ch; // 0
		int m_60h; // -1
		int m_64h; // 7?
		int m_68h; // 0?
		char pad_6Ch[0x3C];
		int lockType; // 2: not locked, 1: reader, 0: writer
		char pad_ACh[0x6E]; // end pad?

	public:
		inline phIterator(rage::five::Vector3 center, rage::five::Vector3 size)
			: center(center), size(size)
		{
			memset(this, 0xFF, sizeof(*this));

			fuckifiknow = -1;
			m_48h = -1;
			mask = -1;
			m_5Ch = -1;//0;
			m_60h = -1;
			m_64h = 0; // some ignore flag
			m_68h = -1;
			lockType = 1;

			//g_GlobalPhysicsLock->WaitAsReader();

			static_assert(offsetof(phIterator, m_48h) == 0x48, "48");
			static_assert(offsetof(phIterator, m_60h) == 0x60, "60");
			static_assert(offsetof(phIterator, pad_ACh) == 0xAC, "B2");
		}

		inline ~phIterator()
		{
			if (lockType != 2)
			{
				if (lockType == 1)
				{
					//g_GlobalPhysicsLock->ReleaseAsReader();
				}
				else
				{
					assert(!"ReleaseAsWriter");
				}

				lockType = 2;
			}
		}
	};

	class phArchetype
	{
	public:
		virtual ~phArchetype() = 0;

		inline five::phBound* GetBound()
		{
			return m_bound;
		}

	private:
		char m_pad[24];
		five::phBound* m_bound;
	};

	class phInst
	{
	public:
		virtual ~phInst() = 0;

		inline phArchetype* GetArchetype()
		{
			return m_archetype;
		}

		inline DirectX::XMFLOAT4X4* GetMatrix()
		{
			return &m_matrix;
		}

	private:
		void* m_pad;
		phArchetype* m_archetype;
		void* m_pad2;
		DirectX::XMFLOAT4X4 m_matrix;
	};

	class phLevel
	{
	public:
		uint16_t GetFirstCulledInstance(phIterator& it);

		uint16_t GetNextCulledInstance(phIterator& it);

		phInst* GetInstanceByIndex(uint16_t index)
		{
			return (phInst*)(m_instances[index].instance & ~(uintptr_t)0xF);
		}

	private:
		char m_pad[80];

		struct InstWrapper
		{
			char pad[32];
			uintptr_t instance;
			uintptr_t pad2;
		};

		InstWrapper* m_instances;
	};

	uint16_t phLevel::GetFirstCulledInstance(phIterator& it)
	{
		return _getFirstCulledInstance(this, &it);
	}

	uint16_t phLevel::GetNextCulledInstance(phIterator& it)
	{
		return _getNextCulledInstance(this, &it);
	}
}

static __declspec(noinline) void Nope()
{

}

static rage::grmShaderFx* g_emissive;
static int g_esampler;

static void RenderBound(const DirectX::XMFLOAT4X4& matrix, rage::five::phBound* bound)
{
	using namespace DirectX;

	if (!bound)
	{
		return;
	}

	switch (bound->GetType())
	{
	case rage::five::phBoundType::BVH:
	case rage::five::phBoundType::Geometry:
	{
		auto geom = (rage::five::phBoundGeometry*)bound;

		rage::five::phBoundPoly* polys = geom->GetPolygons();
		auto numPolys = geom->GetNumPolygons();

		auto verts = geom->GetVertices();
		auto quantum = geom->GetQuantum();
		auto offset = geom->GetVertexOffset();

		auto matRefs = geom->GetPolysToMaterials();
		auto mats = geom->GetMaterials();
		auto colors = geom->GetMaterialColors();

		int numTriangles = 0;

		for (int i = 0; i < numPolys; i++)
		{
			if (polys[i].type == 0)
			{
				numTriangles++;
			}
			else if (polys[i].type == 3)
			{
				numTriangles += 6 * 2;
			}
		}

		if (numTriangles == 0)
		{
			return;
		}

		SetWorldMatrix((float*)&matrix);

		int passes = g_emissive->PushTechnique(0, true, 0);

		// 1604, none
		g_emissive->SetSampler(g_esampler, *(rage::grcTexture**)0x142B07FD8);

		auto q = XMVectorSet(quantum.x, quantum.y, quantum.z, 1.0f);
		auto o = XMVectorSet(offset.x, offset.y, offset.z, 0.0f);

		auto getVert = [&](int idx) -> XMVECTOR
		{
			const auto& v = verts[idx & 0x7FFF];
			
			//return { v.x * quantum.x + offset.x, v.y * quantum.y + offset.y, v.z * quantum.z + offset.z };
			return XMVectorSet(v.x, v.y, v.z, 1.0f) * q + o;
		};

		for (int p = 0; p < passes; p++)
		{
			g_emissive->PushPass(p);

			rage::grcBegin(3, numTriangles * 3);

			for (int i = 0; i < numPolys; i++)
			{
				if (polys[i].type == 0)
				{
					auto v1 = getVert(polys[i].poly.v1);
					auto v2 = getVert(polys[i].poly.v2);
					auto v3 = getVert(polys[i].poly.v3);

					auto n = XMVector3Normalize(XMVector3Cross(v2 - v1, v3 - v1));
					
					XMFLOAT3 n2;
					XMStoreFloat3(&n2, n);

					auto color = (colors) ? colors[mats[matRefs[i]].mat2.materialColorIdx] : 0xFFFFFFFF;

					rage::grcVertex(XMVectorGetX(v1), XMVectorGetY(v1), XMVectorGetZ(v1), n2.x, n2.y, n2.z, color, 0.0f, 0.0f);
					rage::grcVertex(XMVectorGetX(v2), XMVectorGetY(v2), XMVectorGetZ(v2), n2.x, n2.y, n2.z, color, 0.0f, 0.0f);
					rage::grcVertex(XMVectorGetX(v3), XMVectorGetY(v3), XMVectorGetZ(v3), n2.x, n2.y, n2.z, color, 0.0f, 0.0f);
				}
				else if (polys[i].type == 3)
				{
					auto a = getVert(polys[i].box.indices[0]);
					auto b = getVert(polys[i].box.indices[1]);
					auto c = getVert(polys[i].box.indices[2]);
					auto d = getVert(polys[i].box.indices[3]);

					auto mid = ((c + d) - (a + b)) * 0.5f;
					b = a + mid;
					c = c - mid;
					d = d - mid;

					b -= a;
					c -= a;
					d -= a;

					auto getBox = [&](XMVECTOR in)
					{
						return a + (XMVectorGetX(in) * b + XMVectorGetY(in) * c + XMVectorGetZ(in) * d);
					};

					XMFLOAT3 verts[12];
					XMFLOAT3 norms[12];

					// -z
					XMStoreFloat3(&verts[0], getBox(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)));
					XMStoreFloat3(&norms[0], XMVector3Normalize(getBox(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f))));

					XMStoreFloat3(&verts[1], getBox(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)));
					XMStoreFloat3(&norms[1], XMVector3Normalize(getBox(XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f))));

					// +z
					XMStoreFloat3(&verts[2], getBox(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f)));
					XMStoreFloat3(&norms[2], XMVector3Normalize(getBox(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f))));

					XMStoreFloat3(&verts[3], getBox(XMVectorSet(0.0f, 1.0f, 1.0f, 1.0f)));
					XMStoreFloat3(&norms[3], XMVector3Normalize(getBox(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f))));

					// -y
					XMStoreFloat3(&verts[4], getBox(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)));
					XMStoreFloat3(&norms[4], XMVector3Normalize(getBox(XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f))));

					XMStoreFloat3(&verts[5], getBox(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f)));
					XMStoreFloat3(&norms[5], XMVector3Normalize(getBox(XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f))));

					// +y
					XMStoreFloat3(&verts[6], getBox(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)));
					XMStoreFloat3(&norms[6], XMVector3Normalize(getBox(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f))));

					XMStoreFloat3(&verts[7], getBox(XMVectorSet(0.0f, 1.0f, 1.0f, 1.0f)));
					XMStoreFloat3(&norms[7], XMVector3Normalize(getBox(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f))));

					// -x
					XMStoreFloat3(&verts[8], getBox(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)));
					XMStoreFloat3(&norms[8], XMVector3Normalize(getBox(XMVectorSet(-1.0f, 0.0f, 0.0f, 1.0f))));

					XMStoreFloat3(&verts[9], getBox(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f)));
					XMStoreFloat3(&norms[9], XMVector3Normalize(getBox(XMVectorSet(-1.0f, 0.0f, 0.0f, 1.0f))));

					// +x
					XMStoreFloat3(&verts[10], getBox(XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f)));
					XMStoreFloat3(&norms[10], XMVector3Normalize(getBox(XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f))));

					XMStoreFloat3(&verts[11], getBox(XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f)));
					XMStoreFloat3(&norms[11], XMVector3Normalize(getBox(XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f))));

					auto color = (colors) ? colors[mats[matRefs[i]].mat2.materialColorIdx] : 0xFFFFFFFF;

					static uint16_t indx[] = {
						0,2,1,1,2,3,
						4,5,6,5,7,6,
						8,9,10,9,11,10,
						12,14,13,13,14,15,
						16,18,17,17,18,19,
						20,21,22,21,23,22
					};

					for (auto ind : indx)
					{
						rage::grcVertex(verts[ind].x, verts[ind].y, verts[ind].z, norms[ind].x, norms[ind].y, norms[ind].z, color, 0.0f, 0.0f);
					}
				}
			}

			rage::grcEnd();

			g_emissive->PopPass();
		}

		g_emissive->PopTechnique();

		break;
	}
	case rage::five::phBoundType::Composite:
	{
		auto comp = (rage::five::phBoundComposite*)bound;
		auto count = comp->GetNumChildBounds();

		for (uint16_t i = 0; i < count; i++)
		{
			auto fourByFour = *(XMFLOAT4X4*)&comp->GetChildMatrices()[i];
			fourByFour._14 = 0;
			fourByFour._24 = 0;
			fourByFour._34 = 0;
			fourByFour._44 = 1;

			auto m = XMLoadFloat4x4(&matrix);
			auto m2 = XMLoadFloat4x4(&fourByFour);

			XMFLOAT4X4 matrixOut;
			XMStoreFloat4x4(&matrixOut, m * m2);

			RenderBound(matrixOut, comp->GetChildBound(i));
		}

		break;
	}
	}
}

static void* origin;
static float* (*getCoordsFromOrigin)(void*, float*);

static HookFunction hookFunctionOrigin([]()
{
	auto loc = hook::get_call(hook::get_pattern<char>("C6 45 0B 80 89 5D 0F", 0x1B));
	origin = hook::get_address<void*>(loc + 0xC);
	hook::set_call(&getCoordsFromOrigin, loc + 0x10);
});

static void DrawCollisionLevel()
{
	if (!g_emissive)
	{
		g_emissive = rage::grmShaderFactory::GetInstance()->GetShader();
		//g_emissive->LoadTechnique("emissive.sps", nullptr, true);
		g_emissive->LoadTechnique("default.sps", nullptr, true);

		g_esampler = g_emissive->GetParameter("DiffuseSampler");
	}

	uintptr_t a = 0;
	uintptr_t b = 0;

	EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
	{
		alignas(16) float centerOfWorld[4];
		getCoordsFromOrigin(origin, centerOfWorld);

		auto lastDs = GetDepthStencilState();
		//SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

		D3D11_RASTERIZER_DESC rs = CD3D11_RASTERIZER_DESC(D3D11_FILL_WIREFRAME, D3D11_CULL_NONE, TRUE, 0, 0.0f, 0.0f, TRUE, FALSE, FALSE, FALSE);

		auto lastRs = GetRasterizerState();
		//SetRasterizerState(CreateRasterizerState(&rs));

		rage::phIterator it({ centerOfWorld[0], centerOfWorld[1], centerOfWorld[2] }, { 250.0f, 250.0f, 2000.0f });
		int num = 0;

		for (auto id = (*rage::g_level)->GetFirstCulledInstance(it); id != 0xFFFF; id = (*rage::g_level)->GetNextCulledInstance(it))
		{
			rage::phInst* inst = (*rage::g_level)->GetInstanceByIndex(id);

			rage::five::phBound* bound = inst->GetArchetype()->GetBound();
			auto m = *inst->GetMatrix();
			m._14 = 0;
			m._24 = 0;
			m._34 = 0;
			m._44 = 1;

			RenderBound(m, bound);
			num++;
		}

		SetRasterizerState(lastRs);
		SetDepthStencilState(lastDs);

		//trace("drew %d bounds\n", num);
	}, &a, &b);
}

static float tx, ty, tz;

static HookFunction shaderHookFunction([]()
{
	return;

	using namespace DirectX;

	for (int i = 0; i < std::size(g_vertex_buffer_data) / 3 / 3; i++)
	{
		int start = (i * 3 * 3);
		auto t1 = XMVectorSet(g_vertex_buffer_data[start + 0 + 0], g_vertex_buffer_data[start + 0 + 1], g_vertex_buffer_data[start + 0 + 2], 0.0f);
		auto t2 = XMVectorSet(g_vertex_buffer_data[start + 3 + 0], g_vertex_buffer_data[start + 3 + 1], g_vertex_buffer_data[start + 3 + 2], 0.0f);
		auto t3 = XMVectorSet(g_vertex_buffer_data[start + 6 + 0], g_vertex_buffer_data[start + 6 + 1], g_vertex_buffer_data[start + 6 + 2], 0.0f);

		auto n = XMVector3Normalize(XMVector3Cross(t2 - t1, t3 - t1));

		g_normal_buffer_data[start + 0 + 0] = XMVectorGetX(n);
		g_normal_buffer_data[start + 3 + 0] = XMVectorGetX(n);
		g_normal_buffer_data[start + 6 + 0] = XMVectorGetX(n);

		g_normal_buffer_data[start + 0 + 1] = XMVectorGetY(n);
		g_normal_buffer_data[start + 3 + 1] = XMVectorGetY(n);
		g_normal_buffer_data[start + 6 + 1] = XMVectorGetY(n);

		g_normal_buffer_data[start + 0 + 2] = XMVectorGetZ(n);
		g_normal_buffer_data[start + 3 + 2] = XMVectorGetZ(n);
		g_normal_buffer_data[start + 6 + 2] = XMVectorGetZ(n);
	}

	// 1604
	// fuck the world
	hook::return_function(0x14050EC94);

	OnTempDrawEntityList.Connect([]()
	{
		DrawCollisionLevel();
	});

	OnTempDrawEntityList.Connect([]()
	{
		static ConVar<bool> tempCubeRenderer("tempCubeRenderer", ConVar_None, false);

		if (!tempCubeRenderer.GetValue())
		{
			return;
		}

		auto pp = *fx::ScriptEngine::GetNativeHandler(0xD80958FC74E988A6);

		fx::ScriptContextBuffer cx;
		pp(cx);

		cx.SetArgument(0, cx.GetResult<int>());
		auto ec = *fx::ScriptEngine::GetNativeHandler(0x3FEF770D40960D5A);
		ec(cx);

		struct scrVector
		{
			float x;
			int pad;
			float y;
			int ped;
			float z;
			int pid;
		};

		auto v = cx.GetResult<scrVector>();

		if (GetAsyncKeyState(VK_F5))
		{
			tx = v.x;
			ty = v.y;
			tz = v.z;
		}

		if (!g_default)
		{
			g_default = rage::grmShaderFactory::GetInstance()->GetShader();
			g_default->LoadTechnique("default.sps", nullptr, true);

			g_sampler = g_default->GetParameter("DiffuseSampler");
		}

		uintptr_t a = 0;
		uintptr_t b = 0;

		EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
		{
			float matrix[] = {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				tx, ty, tz, 1.0f
			};

			SetWorldMatrix(matrix);

			int passes = g_default->PushTechnique(0, true, 0);
			g_default->SetSampler(g_sampler, LookupTexture("globalroads", "cs_rsn_sl_desgravelstonesdirt_01"));

			for (int p = 0; p < passes; p++)
			{
				g_default->PushPass(p);

				rage::grcBegin(3, std::size(g_vertex_buffer_data) / 3);

				for (int i = 0; i < std::size(g_vertex_buffer_data) / 3; i++)
				{
					rage::grcVertex(g_vertex_buffer_data[i * 3 + 0], g_vertex_buffer_data[i * 3 + 1], g_vertex_buffer_data[i * 3 + 2], g_normal_buffer_data[i * 3 + 0], g_normal_buffer_data[i * 3 + 1], g_normal_buffer_data[i * 3 + 2], 0xffffffff, g_uv_buffer_data[i * 2 + 0], g_uv_buffer_data[i * 2 + 1]);
				}

				rage::grcEnd();

				g_default->PopPass();
			}

			g_default->PopTechnique();
		}, &a, &b);
	});
});

#include <Streaming.h>

class rmcDrawable
{
public:
	virtual ~rmcDrawable() = 0;

	virtual void m_8() = 0;

	virtual void m_10() = 0;

	virtual void m_18() = 0;

	virtual void Draw(const void* matrix, int drawBucketId, int lodId) = 0;

public:
	char m_pad[192 - 8];
	const char* name;
};

struct tempVec3
{
	float x;
	float y;
	float z;
};

struct tempVec4
{
	float x;
	float y;
	float z;
	float w;
};

class tempDrawEntity
{
public:
	tempDrawEntity(tempVec3 position, tempVec4 orientation, uint32_t modelHash, std::initializer_list<uint32_t> txds, uint32_t dwdHash)
		: position(position), orientation(orientation), modelHash(modelHash), txds(txds), dwdHash(dwdHash)
	{

	}

	void Update();

private:
	tempVec3 position;
	tempVec4 orientation;
	uint32_t modelHash;
	std::vector<uint32_t> txds;
	uint32_t dwdHash;
};

static tempDrawEntity entities[] = {
	{ { 2971.742, -3421.632, 180.9759 }, { 0, 0, 0, 1 }, 0xE81D17CC, { 0xC2049B11 }, 0x00000000 },
{ { 5287.458, -3159.843, 155.0877 }, { 0, 0, 0, 1 }, 0x58587819, { 0xC2049B11 }, 0x00000000 },
{ { 3224.538, -4005.678, 118.4348 }, { 0, 0, 0, 1 }, 0x7B422691, { 0xC2049B11 }, 0x00000000 },
{ { 563.4141, 20.12158, 91.64003 }, { 0, 0, 0, 1 }, 0x8B263E1E, { 0x8B263E1E }, 0x00000000 },
{ { 563.4141, 20.12158, 91.64003 }, { 0, 0, 0, 1 }, 0x14483335, { 0x14483335 }, 0x00000000 },
{ { 1495.459, -1034.235, 56.35914 }, { 0, 0, 0, 1 }, 0x63B34774, { 0xEB035DBE, 0xD52F9C1A, 0xE3018E05, 0xF4B1A815 }, 0x00000000 },
{ { 1495, -1031.174, 55.54118 }, { 0, 0, 0.2239991, 0.9745893 }, 0x03190581, { 0x5F9C6DFB, 0xD52F9C1A, 0xE3018E05, 0xF4B1A815 }, 0x00000000 },
{ { 2501.466, -1573.631, 110.4485 }, { 0, 0, 0, 1 }, 0x29469507, { 0x8FBAB9BA, 0xA9380EBA, 0x8B436D58 }, 0x9CDC0FDA },
{ { 2530.239, -1023.612, 73.44315 }, { 0, 0, 0, 1 }, 0x7576AAEC, { 0x8FBAB9BA, 0xA9380EBA, 0x8B436D58 }, 0x9CDC0FDA },
{ { 2266.588, -2202.771, 75.0304 }, { 0, 0, 0, 1 }, 0xB9371C35, { 0x8FBAB9BA, 0xA9380EBA, 0x8B436D58 }, 0xA67835CA },
{ { 2508.054, -373.6025, 101.8865 }, { 0, 0, 0, 1 }, 0xDEFFE27F, { 0x8FBAB9BA, 0xA9380EBA, 0x8B436D58 }, 0x5B17EF8F },
{ { 2706.003, -479.038, 62.80208 }, { 0, 0, 0, 1 }, 0x1CFDAD0F, { 0x9636DFEE }, 0x9636DFEE },
{ { 2767.535, -686.2159, 25.9073 }, { 0, 0, 0.286859, 0.9579728 }, 0xCF9D6A2E, { 0x9636DFEE }, 0x9636DFEE },
{ { 1622.855, -793.9711, 104.1432 }, { 0, 0, 0, 1 }, 0x7C6F0946, { 0x6A24DEDC }, 0x2DCA81C0 },
{ { 1312.146, -894.6407, 77.05546 }, { 0, 0, 0, 1 }, 0x7D5ACFE0, { 0x8E766A6A, 0xBD58B261, 0x9F958B66, 0xD2017791, 0x3E037D38 }, 0x00000000 },
{ { 1647.367, -890.1594, 73.0259 }, { 0, 0, 0, 1 }, 0x2DCB913A, { 0x3F9245C0, 0xBD58B261, 0x9F958B66, 0xD2017791, 0x3E037D38 }, 0x00000000 },
{ { 1649.696, -911.9739, 64.47817 }, { 0, 0, 0, 1 }, 0x668A02B6, { 0x3F9245C0, 0xBD58B261, 0x9F958B66, 0xD2017791, 0x3E037D38 }, 0x00000000 },
{ { 1353.486, -993.538, 74.48832 }, { 0, 0, 0, 1 }, 0x9FE39B52, { 0x8E766A6A, 0xBD58B261, 0x9F958B66, 0xD2017791, 0x3E037D38 }, 0x00000000 },
{ { 1675.557, -1197.542, 111.6309 }, { 0, 0, 0, 1 }, 0x0FD4938F, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1584.736, -1160.954, 122.0684 }, { 0, 0, 0, 1 }, 0x967CB3BE, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1533.387, -1182.13, 100.9376 }, { 0, 0, 0, 1 }, 0x77BEAFD4, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1646.369, -1239.98, 96.95461 }, { 0, 0, 0, 1 }, 0x6DC38219, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1583.743, -1270.047, 101.4746 }, { 0, 0, 0, 1 }, 0x9E528D47, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1587.463, -1169.9, 121.2832 }, { 0, 0, 0, 1 }, 0xB62D1726, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1654.408, -1209.109, 103.1779 }, { 0, 0, 0, 1 }, 0xE30E6CF2, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1644.722, -1250.755, 93.6414 }, { 0, 0, 0, 1 }, 0xAD570A3D, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1583.346, -1266.767, 103.4198 }, { 0, 0, 0, 1 }, 0x22755F69, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1654.426, -1208.731, 103.0669 }, { 0, 0, 0, 1 }, 0x939178F3, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1571.342, -1183.041, 114.2858 }, { 0, 0, 0, 1 }, 0x5467A31A, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1580.54, -1277.696, 99.03581 }, { 0, 0, 0, 1 }, 0x3243C8A2, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1662.048, -1203.159, 107.045 }, { 0, 0, 0, 1 }, 0xFEF3D92C, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1607.378, -1163.115, 130.0507 }, { 0, 0, 0, 1 }, 0x21C41930, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1506.295, -1219.89, 95.80144 }, { 0, 0, 0, 1 }, 0xAD946991, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1647.276, -1246.769, 94.22139 }, { 0, 0, 0, 1 }, 0x19BCF640, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1640.899, -1234.452, 96.78803 }, { 0, 0, 0, 1 }, 0x79346255, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x0FDEFBFA },
{ { 1499.155, -1174.161, 86.17865 }, { 0, 0, 0, 1 }, 0xF1176A26, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x276B2A75 },
{ { 1967.18, -944.6223, 78.00827 }, { 0, 0, 0, 1 }, 0xF62EFF98, { 0x59034FB3 }, 0x59034FB3 },
{ { 1958.945, -839.3553, 95.73784 }, { 0, 0, 0, 1 }, 0xA67DD426, { 0x55368F82 }, 0x55368F82 },
{ { 1736.933, -1225.486, 102.6833 }, { 0, 0, 0, 1 }, 0xCBD5BA52, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1685.494, -1285.379, 91.73894 }, { 0, 0, 0, 1 }, 0x5175B57D, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1645.563, -1280.826, 86.89836 }, { 0, 0, 0, 1 }, 0x440A0FAA, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1707.549, -1263.909, 93.39445 }, { 0, 0, 0, 1 }, 0xCA55E1DC, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1654.581, -1295.865, 85.8454 }, { 0, 0, 0, 1 }, 0xB2E2056F, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1740.712, -1205.868, 106.17 }, { 0, 0, 0, 1 }, 0x1245683B, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1707.911, -1259.357, 98.3264 }, { 0, 0, 0, 1 }, 0xF1A2F68D, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1651.734, -1291.399, 85.0908 }, { 0, 0, 0, 1 }, 0x009A95D8, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1763.423, -1213.882, 92.80026 }, { 0, 0, 0, 1 }, 0xB1D9E031, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1660.73, -1296.147, 86.86729 }, { 0, 0, 0, 1 }, 0xF617E597, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1715.293, -1253.687, 98.55184 }, { 0, 0, 0, 1 }, 0x362CE425, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1728.797, -1204.902, 110.7918 }, { 0, 0, 0, 1 }, 0x4341AB46, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1756.136, -1210.246, 96.64172 }, { 0, 0, 0, 1 }, 0x921B7D5D, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1725.531, -1201.586, 111.3991 }, { 0, 0, 0, 1 }, 0x54CBA39F, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5F1F30C2 },
{ { 1533.69, -1038.807, 54.80505 }, { 0, 0, 0, 1 }, 0x9D774C2B, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1663.068, -1047.633, 103.9969 }, { 0, 0, 0, 1 }, 0xDC89269B, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1599.711, -1083.936, 84.8225 }, { 0, 0, 0, 1 }, 0xCB0CF5AE, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1598.484, -1027.064, 70.93382 }, { 0, 0, 0, 1 }, 0x7F7CCA3A, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1660.491, -989.3005, 76.08327 }, { 0, 0, 0, 1 }, 0x9CBA4298, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1668.72, -1010.263, 89.81519 }, { 0, 0, 0, 1 }, 0x371C1E54, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1546.807, -1021.142, 55.74283 }, { 0, 0, 0, 1 }, 0xE3324C9E, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1661.884, -1057.174, 106.255 }, { 0, 0, 0, 1 }, 0x1164064C, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1593.321, -1086.975, 88.35764 }, { 0, 0, 0, 1 }, 0x201351CD, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1608.283, -1026.287, 75.50018 }, { 0, 0, 0, 1 }, 0x248EF884, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1658.573, -994.2795, 77.80389 }, { 0, 0, 0, 1 }, 0x3120040C, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 1649.597, -967.5728, 65.78365 }, { 0, 0, 0, 1 }, 0xD6ACF397, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x5C5BA968 },
{ { 2097.606, -751.4193, 92.6685 }, { 0, 0, 0, 1 }, 0x9184AB32, { 0x55368F82 }, 0x55368F82 },
{ { 1728.229, -1004.117, 116.8207 }, { 0, 0, 0, 1 }, 0xD89BE61E, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1685.644, -974.4666, 86.66085 }, { 0, 0, 0, 1 }, 0xB043A043, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1745.001, -935.6745, 91.21021 }, { 0, 0, 0, 1 }, 0x4BB9F855, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1744.177, -1003.387, 119.7666 }, { 0, 0, 0, 1 }, 0x8E88D15E, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1797.039, -972.0854, 119.2516 }, { 0, 0, 0, 1 }, 0x04DA0DE5, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1682.602, -969.1943, 80.3953 }, { 0, 0, 0, 1 }, 0x539B1229, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1738.771, -950.9182, 100.1955 }, { 0, 0, 0, 1 }, 0xB9A08035, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1738.466, -999.8344, 118.0255 }, { 0, 0, 0, 1 }, 0xD61A9190, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1808.354, -976.11, 115.8295 }, { 0, 0, 0, 1 }, 0x7FE6C00E, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1682.706, -990.8405, 88.98187 }, { 0, 0, 0, 1 }, 0x5DA53981, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1729.77, -953.6293, 106.4918 }, { 0, 0, 0, 1 }, 0x61D13FFC, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1791.11, -959.5366, 117.1136 }, { 0, 0, 0, 1 }, 0xE56ADB1F, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1728.555, -999.7198, 115.3571 }, { 0, 0, 0, 1 }, 0x861AE100, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1751.616, -959.4675, 104.9598 }, { 0, 0, 0, 1 }, 0x60E551AB, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xC246B50E },
{ { 1682.449, -1132.226, 130.1169 }, { 0, 0, 0, 1 }, 0x7FF360F3, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1692.953, -1054.801, 122.9721 }, { 0, 0, 0, 1 }, 0x45A7DD4A, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1791.598, -1130.412, 84.02214 }, { 0, 0, 0, 1 }, 0x97811F44, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1748.36, -1176.889, 101.3129 }, { 0, 0, 0, 1 }, 0x68E7B194, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1737.363, -1130.046, 108.2132 }, { 0, 0, 0, 1 }, 0xE88A058D, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1763.423, -1080.254, 101.9181 }, { 0, 0, 0, 1 }, 0x2EA8D551, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1807.342, -1081.33, 84.34256 }, { 0, 0, 0, 1 }, 0x41348ADE, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1693.884, -1026.438, 110.8808 }, { 0, 0, 0, 1 }, 0xB55BB3BE, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1797.327, -1137.129, 82.41267 }, { 0, 0, 0, 1 }, 0x55AE35E1, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1798.47, -1135.817, 81.94061 }, { 0, 0, 0, 1 }, 0x1262ACC8, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1693.699, -1126.837, 126.1121 }, { 0, 0, 0, 1 }, 0xC8995A44, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1798.059, -1134.934, 82.64594 }, { 0, 0, 0, 1 }, 0xD23DF461, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1798.945, -1076.968, 88.03568 }, { 0, 0, 0, 1 }, 0x9B23AC80, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1754.147, -1188.723, 95.8372 }, { 0, 0, 0, 1 }, 0xF982078D, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1689.127, -1126.645, 128.0309 }, { 0, 0, 0, 1 }, 0x7056A662, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1752.717, -1110.217, 103.0698 }, { 0, 0, 0, 1 }, 0x0F4DB9EB, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1705.504, -1028.232, 117.5322 }, { 0, 0, 0, 1 }, 0xF955ACE6, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1668.78, -1065.885, 113.2421 }, { 0, 0, 0, 1 }, 0x77EF2A0E, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1791.195, -1132.649, 84.84516 }, { 0, 0, 0, 1 }, 0xDF492811, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1797.624, -1081.764, 87.67862 }, { 0, 0, 0, 1 }, 0x80079D96, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1756.145, -1170.011, 96.3111 }, { 0, 0, 0, 1 }, 0x8ACD7830, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1738.783, -1101.108, 110.7352 }, { 0, 0, 0, 1 }, 0x67FDC35F, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1700.702, -1034.68, 118.6327 }, { 0, 0, 0, 1 }, 0x64150BE0, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1698.693, -1029.533, 114.6032 }, { 0, 0, 0, 1 }, 0x347D0C0B, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 1797.927, -1128.029, 84.28793 }, { 0, 0, 0, 1 }, 0x102D6FEC, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0x4E4F7B23 },
{ { 2059.872, -864.5206, 112.0185 }, { 0, 0, 0, 1 }, 0x7B2C1905, { 0x55368F82 }, 0x55368F82 },
{ { 1927.878, -937.4507, 79.7516 }, { 0, 0, 0, 1 }, 0xAB109A85, { 0x55368F82 }, 0x55368F82 },
{ { 1628.456, -1313.192, 83.13454 }, { 0, 0, 0, 1 }, 0x797BFFBC, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1589.238, -1300.415, 89.63047 }, { 0, 0, 0, 1 }, 0xC0669A8D, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1581.213, -1352.3, 88.68578 }, { 0, 0, 0, 1 }, 0xEA93A00A, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1634.362, -1324.938, 82.84157 }, { 0, 0, 0, 1 }, 0xA264E975, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1588.555, -1302.168, 88.42407 }, { 0, 0, 0, 1 }, 0x06B00205, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1585.451, -1357.665, 81.72176 }, { 0, 0, 0, 1 }, 0xFC0AA8A7, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1647.38, -1307.408, 83.65143 }, { 0, 0, 0, 1 }, 0xE3807158, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1586.954, -1329.509, 87.65215 }, { 0, 0, 0, 1 }, 0x0A026136, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1648.108, -1311.647, 83.25954 }, { 0, 0, 0, 1 }, 0x0B79C61A, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1587.104, -1330.506, 85.29591 }, { 0, 0, 0, 1 }, 0xA45FD711, { 0xD57EB968, 0xE00396AD, 0x8B436D58 }, 0xDFF11398 },
{ { 1861.38, -1010.142, 94.90672 }, { 0, 0, 0, 1 }, 0x5BBFDE3D, { 0x59034FB3 }, 0x59034FB3 },
{ { 2279.934, -581.3378, 82.29405 }, { 0, 0, 0, 1 }, 0xB4DCF5CE, { 0x55368F82 }, 0x55368F82 },
{ { 1599.371, -1353.842, 88.10283 }, { 0, 0, 0, 1 }, 0x7F1F8808, { 0x5D86377B }, 0xA2E3FC29 },
{ { 1619.2, -1348.392, 95.70247 }, { 0, 0, 0, 1 }, 0x8F49ED33, { 0x5D86377B }, 0xA2E3FC29 },
{ { 1502.365, -1017.595, 52.21829 }, { 0, 0, 0, 1 }, 0x646CB4B2, { 0x5D86377B }, 0x7948C0AC },
{ { 2034.797, -829.7502, 87.81308 }, { 0, 0, 0, 1 }, 0x94C72BD9, { 0x5D86377B }, 0x9A9A7328 },
{ { 1923.451, -942.5442, 78.16809 }, { 0, 0, -0.8971943, -0.4416361 }, 0x11A1F145, { 0x5D86377B }, 0x9A9A7328 },
{ { 2033.257, -818.6394, 93.58543 }, { 0, 0, 0, 1 }, 0xB2938629, { 0x5D86377B }, 0x9A9A7328 },
{ { 1779.861, -935.3251, 87.51277 }, { 0, 0, 0, 1 }, 0x252D1037, { 0x5D86377B }, 0xB93B16F8 },
{ { 1759.358, -936.048, 93.00938 }, { 0, 0, 0, 1 }, 0x285F2CC1, { 0x5D86377B }, 0xB93B16F8 },
{ { 1530.858, -1048.496, 58.22413 }, { -0.06538299, -0.2855269, -0.9320138, -0.213424 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1521.298, -1057.515, 63.59405 }, { -0.023781, -0.212983, -0.9707341, -0.108389 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1530.792, -1063.722, 64.96968 }, { -0.038914, -0.201723, -0.960952, -0.185377 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1529.846, -1072.945, 68.93819 }, { -0.03977799, -0.20881, -0.9598848, -0.182858 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1532.182, -1057.124, 62.11879 }, { -0.036739, -0.21395, -0.9620719, -0.165207 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1530.764, -1077.907, 70.90623 }, { -0.036319, -0.212097, -0.9625628, -0.164829 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1542.345, -1035.421, 53.05397 }, { -0.1425401, -0.2727181, 0.3044961, 0.9014373 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1542.43, -1030.309, 53.01607 }, { 0.188784, -0.037692, -0.910579, 0.365767 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1543.031, -1032.205, 53.1333 }, { 0.1795201, -0.1527881, -0.8594954, 0.4535372 }, 0xEDA770F8, { 0x3B2A8C05 }, 0x00000000 },
{ { 1658.146, -1315.429, 83.49644 }, { 0, 0, 0.4694718, 0.8829474 }, 0x2AFB6323, { 0xD919320A }, 0x00000000 },
{ { 1646.424, -1309.827, 83.10681 }, { 0, 0, -0.6156611, 0.788011 }, 0x5D37C683, { 0xD919320A }, 0x00000000 },
{ { 1650.501, -1316.263, 82.74122 }, { 0, 0, 0, 1 }, 0x2AFB6323, { 0xD919320A }, 0x00000000 },
{ { 1644.651, -1299.917, 84.21449 }, { 0, 0, -0.9996573, 0.02617701 }, 0x8A18A044, { 0xC00C0581 }, 0x00000000 },
{ { 1642.657, -1320.112, 81.90799 }, { 0, 0, -0.9612617, -0.2756369 }, 0x2AFB6323, { 0xD919320A }, 0x00000000 },
{ { 1618.056, -1325.885, 82.01298 }, { 0, 0, -0.97237, 0.233445 }, 0x2AFB6323, { 0xD919320A }, 0x00000000 },
{ { 1659.158, -1303.327, 84.62959 }, { 0, 0, -0.438371, 0.8987941 }, 0x5D37C683, { 0xD919320A }, 0x00000000 },
{ { 1599.031, -1047.08, 72.91144 }, { 0.1295051, -0.1105581, -0.6397993, 0.7494411 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1598.532, -1037.863, 73.49577 }, { 0.114498, -0.1024301, -0.6588233, 0.7364441 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1599.935, -1041.715, 73.65511 }, { 0.127812, -0.116084, -0.6622308, 0.7291358 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1576.572, -1069.002, 73.41742 }, { 0.024911, -0.2623149, -0.9603399, 0.09119999 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1584.484, -1060.668, 69.94736 }, { 0.03619099, -0.2092969, -0.9628927, 0.1665019 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1589.386, -1063.347, 72.17889 }, { 0.05724999, -0.243025, -0.9425299, 0.222033 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1576.617, -1059.805, 68.71223 }, { 0.025178, -0.202188, -0.9715191, 0.120982 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1585.55, -1070.551, 75.38472 }, { 0.028832, -0.271708, -0.9565771, 0.101507 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1583.609, -1063.146, 70.96535 }, { 0.03753099, -0.2340479, -0.9592457, 0.15382 }, 0xE43D331B, { 0xE76EAE8B }, 0x00000000 },
{ { 1572.837, -1026.372, 64.18144 }, { 0.109743, -0.05908997, -0.9920844, -0.01526799 }, 0xEDA770F8, { 0x3B2A8C05 }, 0x00000000 },
{ { 1569.9, -1045.32, 62.87985 }, { -0.005709004, -0.2340481, 0.1593851, 0.9590544 }, 0xEDA770F8, { 0x3B2A8C05 }, 0x00000000 },
{ { 1582.083, -1009.197, 64.16878 }, { -0.07426002, -0.129892, -0.102212, 0.9834461 }, 0x0C9B0A8A, { 0x395D067D }, 0x00000000 },
{ { 1625.708, -1006.921, 69.50903 }, { 0.1527579, -0.1230129, -0.9397786, -0.2799089 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1572.278, -1047.217, 64.02602 }, { 0.218314, -0.070099, -0.9283931, 0.292423 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1545.174, -1030.151, 54.13008 }, { 0.182784, -0.060419, -0.8592159, 0.474012 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1555.375, -1021.412, 58.06495 }, { -0.109342, -0.138703, 0.302741, 0.9365649 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1605.129, -993.3406, 63.83914 }, { -0.144598, 0.003024997, -0.6752439, 0.7232757 }, 0x0C9B0A8A, { 0x395D067D }, 0x00000000 },
{ { 1647.634, -994.2905, 72.45298 }, { 0.112901, -0.2452781, -0.8746462, 0.4025991 }, 0x0C9B0A8A, { 0x395D067D }, 0x00000000 },
{ { 1392.393, -1166.322, 78.16263 }, { 0, 0, 0, 1 }, 0x1D9912D7, { 0xD9F3CA53, 0xE437CC15, 0x0949389E, 0x3E037D38 }, 0x00000000 },
{ { 1381.782, -1170.933, 76.44763 }, { 0, 0, 0, 1 }, 0x363C3B69, { 0xA9C1E597, 0xE437CC15, 0x0949389E, 0x3E037D38 }, 0x00000000 },
{ { 1584.84, -1074.549, 77.87804 }, { 0.027761, -0.28503, -0.9536041, 0.09287801 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1579.842, -1062.38, 70.1924 }, { 0.02559, -0.226385, -0.9675401, 0.109366 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1538.747, -1036.629, 51.94981 }, { -0.103536, -0.111364, 0.169697, 0.9736947 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1546.933, -1039.689, 55.96197 }, { 0.08480101, -0.143509, -0.9844381, 0.055637 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1572.068, -1179.699, 114.4094 }, { 0.136166, -0.152547, -0.7302638, 0.6518458 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1553.35, -1173.507, 106.0133 }, { 0.154455, -0.136234, -0.6473041, 0.733881 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1553.924, -1162.201, 108.4042 }, { 0.2027339, -0.1199109, -0.4947627, 0.8364987 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1529.895, -1142.605, 101.9311 }, { 0.104955, -0.146729, -0.7999991, 0.5722381 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1534.799, -1137.943, 102.3032 }, { 0.08298301, -0.225182, -0.910894, 0.335677 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1536.698, -1133.717, 101.0576 }, { 0.081847, -0.252894, -0.9171869, 0.29684 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1531.121, -1133.936, 99.27914 }, { 0.07602798, -0.250294, -0.923515, 0.280522 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1541.613, -1182.629, 102.1889 }, { 0.108675, -0.19022, -0.847195, 0.4840109 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1531.637, -1201.76, 104.2854 }, { 0.10117, -0.210047, -0.8761138, 0.4219829 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1529.117, -1183.816, 97.44949 }, { 0.106718, -0.198022, -0.8577421, 0.4622521 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1536.175, -1190.315, 102.1486 }, { 0.09923898, -0.200427, -0.8734639, 0.432483 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1539.281, -1181.684, 101.0125 }, { 0.109635, -0.1901129, -0.8451575, 0.4873868 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1536.359, -1188.023, 101.5531 }, { 0.101805, -0.200241, -0.8686259, 0.44162 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1515.891, -1171.166, 90.17714 }, { 0.150061, -0.125449, -0.628998, 0.7523999 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1511.288, -1172.576, 88.16449 }, { 0.138267, -0.133885, -0.6826252, 0.7049679 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1532.365, -1194.322, 101.9541 }, { 0.09991599, -0.211735, -0.8792278, 0.4149019 }, 0xE43D331B, { 0xE76EAE8B }, 0x00000000 },
{ { 1513.569, -1168.462, 89.49593 }, { 0.1711529, -0.115329, -0.5467769, 0.8114437 }, 0xE43D331B, { 0xE76EAE8B }, 0x00000000 },
{ { 1519.828, -1166.445, 92.5376 }, { 0.184348, -0.112299, -0.507977, 0.8338849 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1534.135, -1196.108, 103.2606 }, { 0.09905903, -0.2080561, -0.8785873, 0.4183111 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1525.441, -1195.433, 99.52272 }, { 0.107576, -0.215084, -0.8681229, 0.434199 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1512.047, -1163.848, 89.97296 }, { 0.1913411, -0.107413, -0.4775811, 0.8507451 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1510.688, -1168.766, 88.29535 }, { 0.1653361, -0.117156, -0.5661642, 0.7989972 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1508.963, -1161.59, 89.29558 }, { 0.198604, -0.103653, -0.4509239, 0.8639908 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1527.702, -1134.101, 98.32687 }, { 0.07032997, -0.2436309, -0.9293655, 0.2682859 }, 0xE43D331B, { 0xE76EAE8B }, 0x00000000 },
{ { 1505.435, -1163.713, 87.32372 }, { 0.192343, -0.102806, -0.4600369, 0.860698 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1505.896, -1160.526, 88.41168 }, { 0.199175, -0.09992398, -0.4371469, 0.8713478 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1531.42, -1138.371, 101.3396 }, { 0.08078498, -0.216341, -0.9114947, 0.3403639 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1528.018, -1203.767, 103.4795 }, { 0.106861, -0.215757, -0.869749, 0.4307739 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1527.77, -1199.194, 101.8595 }, { 0.105056, -0.2145601, -0.8721144, 0.4270172 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1538.268, -1183.504, 101.0709 }, { 0.107196, -0.194686, -0.854082, 0.470266 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1515.548, -1167.245, 90.57779 }, { 0.179363, -0.112422, -0.5190511, 0.8281161 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1525.865, -1184.399, 96.28127 }, { 0.106796, -0.2005341, -0.8595553, 0.4577611 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1525.03, -1190.111, 97.60568 }, { 0.109292, -0.209683, -0.861625, 0.4491 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1529.947, -1195.766, 101.5463 }, { 0.101846, -0.215266, -0.8779299, 0.415364 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1533.112, -1187.616, 100.1334 }, { 0.102438, -0.2016569, -0.8684567, 0.4411618 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1494.221, -1091.334, 76.91412 }, { 0.038905, -0.176459, -0.960472, 0.211759 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1529.333, -1120.727, 91.45654 }, { 0.06604699, -0.275665, -0.9325879, 0.223442 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1518.633, -1080.363, 73.5497 }, { -0.02118599, -0.1896589, -0.9755536, -0.108976 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1523.521, -1112.828, 85.98201 }, { 0.009034004, -0.2079281, -0.9771804, 0.04245802 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1526.689, -1112.189, 85.98158 }, { 0.031586, -0.238079, -0.9622999, 0.127669 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1533.625, -1119.885, 92.3735 }, { 0.076971, -0.283885, -0.922458, 0.250112 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1521.965, -1114.184, 86.55232 }, { 0.005755001, -0.205007, -0.9783582, 0.02746301 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1522.355, -1077.099, 71.80414 }, { -0.029493, -0.198496, -0.9690199, -0.143978 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1513.583, -1100.706, 81.44241 }, { -0.006103999, -0.180964, -0.9829118, -0.033154 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1520.782, -1087.876, 76.29318 }, { -0.01725299, -0.1847039, -0.9783835, -0.09139195 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1516.697, -1080.677, 73.82481 }, { -0.017608, -0.186226, -0.9779871, -0.09247202 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1521.754, -1085.288, 75.20937 }, { -0.022004, -0.190038, -0.9750161, -0.112893 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1524.006, -1081.404, 73.36194 }, { -0.02897499, -0.198668, -0.9693828, -0.141381 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1525.483, -1096.356, 79.24025 }, { -0.014816, -0.191388, -0.9784749, -0.07574799 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1523.868, -1099.029, 80.38737 }, { -0.01177499, -0.1882299, -0.9801385, -0.06131197 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1531.54, -1110.352, 85.95967 }, { 0.05930302, -0.2742751, -0.9381423, 0.2028431 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1522.126, -1090.913, 77.3304 }, { -0.016101, -0.1849369, -0.9789156, -0.08522497 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1527.509, -1094.492, 78.35179 }, { -0.017542, -0.196203, -0.9765112, -0.08730602 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1533.181, -1114.076, 88.66763 }, { 0.07075196, -0.2853698, -0.9277145, 0.2300089 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1523.236, -1093.692, 78.33931 }, { -0.015154, -0.1868059, -0.9790636, -0.07942497 }, 0xE43D331B, { 0xE76EAE8B }, 0x00000000 },
{ { 1527.099, -1076.825, 71.007 }, { -0.03480298, -0.2062609, -0.9642475, -0.1627019 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1528.511, -1113.228, 86.83775 }, { 0.05187301, -0.265531, -0.9448451, 0.184581 }, 0xE43D331B, { 0xE76EAE8B }, 0x00000000 },
{ { 1531.018, -1118.647, 90.69093 }, { 0.06964798, -0.2815069, -0.9290168, 0.229849 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1527.316, -1048.65, 59.04129 }, { -0.03096699, -0.2497009, -0.9604697, -0.119115 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1526.943, -1070.068, 68.21935 }, { -0.03763401, -0.205964, -0.9619092, -0.175764 }, 0xE43D331B, { 0xE76EAE8B }, 0x00000000 },
{ { 1529.879, -1060.842, 63.99545 }, { -0.036128, -0.200565, -0.963507, -0.173559 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1525.277, -1073.736, 70.02274 }, { -0.03435901, -0.203808, -0.9647933, -0.162652 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1581.779, -1157.481, 121.6697 }, { 0.1627709, -0.117574, -0.5736229, 0.7941277 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1534.546, -1135.226, 101.0783 }, { 0.079921, -0.245937, -0.918694, 0.298545 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1531.718, -1058.096, 69.44052 }, { 0, 0, -0.5585691, 0.8294579 }, 0x58A9746B, { 0x0641E5DA }, 0x00000000 },
{ { 1672.265, -1125.631, 93.81793 }, { 0, 0, 0, 1 }, 0x792B49FE, { 0xD9F3CA53, 0xE437CC15, 0x0949389E, 0x3E037D38 }, 0x00000000 },
{ { 1702.07, -1194.765, 99.63185 }, { 0, 0, 0, 1 }, 0xC62407E9, { 0xA9C1E597, 0xE437CC15, 0x0949389E, 0x3E037D38 }, 0x00000000 },
{ { 1619.869, -1036.138, 80.89357 }, { 0.102057, -0.172905, -0.843639, 0.4979571 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1621.54, -1032.382, 80.08354 }, { 0.09371399, -0.201833, -0.8842568, 0.4105739 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1595.956, -1035.785, 72.57611 }, { 0.09484802, -0.108396, -0.7447242, 0.6516442 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1612.498, -1045.923, 78.47275 }, { 0.140927, -0.141424, -0.6940869, 0.6916519 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1601.637, -1047.476, 73.92012 }, { 0.1391061, -0.1247451, -0.6558744, 0.7313801 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1578.874, -1057.503, 68.0808 }, { 0.029046, -0.178562, -0.9707409, 0.157905 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1591.185, -1014.197, 66.7057 }, { -0.163855, -0.08627798, 0.03610499, 0.9820408 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1582.676, -1077.808, 79.67575 }, { 0.029714, -0.290257, -0.9515142, 0.09740902 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1587.665, -1078.892, 81.05565 }, { 0.03149801, -0.2937781, -0.9499102, 0.101846 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1590.846, -1076.689, 80.07176 }, { 0.04019502, -0.2908861, -0.9469154, 0.1308471 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1580.311, -1068.912, 73.79141 }, { 0.02520201, -0.2639771, -0.9598353, 0.09163503 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1583.982, -1064.395, 71.60916 }, { 0.03662502, -0.2439341, -0.9583583, 0.143889 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1703.751, -1119.895, 122.5544 }, { 0.1899121, 0.1322511, 0.5559523, 0.7983484 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1762.442, -1077.038, 102.8248 }, { 0.156816, 0.102796, 0.538505, 0.8214951 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1685.698, -1133.927, 129.1674 }, { 0.1900309, 0.144728, 0.5883509, 0.7725188 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1643.38, -1054.1, 94.10493 }, { 0.1481829, -0.2209349, -0.8005706, 0.5369508 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1641.398, -1056.386, 93.48059 }, { 0.150485, -0.221137, -0.7966081, 0.5420961 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1649.734, -1058.05, 98.76993 }, { 0.1523911, -0.2416291, -0.8105833, 0.5112212 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1645.25, -1044.974, 93.11713 }, { 0.129037, -0.21464, -0.8297329, 0.4988209 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1646.367, -1039.899, 92.28737 }, { 0.114384, -0.223337, -0.8615801, 0.441267 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1642.953, -1042.428, 91.43446 }, { 0.1211101, -0.2070871, -0.8380094, 0.4900892 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1662.26, -1067.542, 109.81 }, { 0.154924, -0.277902, -0.828055, 0.461621 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1663.792, -1063.38, 109.2525 }, { 0.1608861, -0.2680061, -0.8144125, 0.4888973 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1613.882, -1054.232, 79.12085 }, { 0.136861, -0.164379, -0.7507148, 0.6250408 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1618.014, -1058.727, 81.78467 }, { 0.120941, -0.218581, -0.8472521, 0.468785 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1682.507, -1020.856, 100.9971 }, { 0.124077, -0.2923269, -0.8728637, 0.3704849 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1618.536, -1052.374, 81.17604 }, { 0.1444791, -0.1579011, -0.7206714, 0.6594133 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1613.186, -1049.632, 78.77357 }, { 0.143629, -0.145879, -0.6974898, 0.6867298 }, 0xD71EC202, { 0xE76EAE8B }, 0x00000000 },
{ { 1715.65, -904.6171, 68.22482 }, { 0, 0, 0, 1 }, 0x80A77AC0, { 0xA9C1E597, 0xE437CC15, 0x0949389E, 0x3E037D38 }, 0x00000000 },
{ { 1545.377, -1041.318, 55.87861 }, { 0.08948298, -0.2944239, -0.4146439, 0.8563747 }, 0x0E1D31E7, { 0xFD21DCEF }, 0x00000000 },
{ { 1519.24, -1116.43, 87.50877 }, { 0.001518, -0.201691, -0.9794202, 0.007373001 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1513.786, -1086.162, 76.03676 }, { -0.009087997, -0.1763659, -0.9829786, -0.05065198 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1524.197, -1102.256, 81.64426 }, { -0.009500999, -0.189773, -0.9805539, -0.04909199 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1516.971, -1106.273, 83.4678 }, { -0.006175002, -0.184559, -0.9822523, -0.03286601 }, 0x98B1C712, { 0x98B1C712 }, 0x00000000 },
{ { 1706.36, -1598.28, 128.1207 }, { 0, 0, 0.1375311, 0.9904975 }, 0x1DD1A447, { 0x34A51163 }, 0x34A51163 },
{ { 1708.307, -1634.727, 115.2673 }, { 0, 0, 0.1375311, 0.9904975 }, 0x8375C6FB, { 0x34A51163 }, 0x34A51163 },
{ { 1863.107, -1398.041, 133.1898 }, { 0, 0, 0, 1 }, 0xD8C320EF, { 0xA7873153 }, 0xA7873153 },
{ { 1722.434, -1591.76, 115.6997 }, { 0, 0, 0, 1 }, 0x4F4BF8A6, { 0x34A51163 }, 0x34A51163 },
{ { 1887.873, -1290.086, 122.8206 }, { 0, 0, 0, 1 }, 0xE72012F3, { 0xA7873153 }, 0xA7873153 },
{ { 1849.624, -1057.073, 78.1862 }, { 0, 0, 0, 1 }, 0x3E236E78, { 0xA7873153 }, 0xA7873153 },
{ { 1665.203, -1339.896, 82.49194 }, { 0, 0, 0, 1 }, 0x5294B06B, { 0xBE84AE29 }, 0x468283C9 },
{ { 1824.045, -1198.807, 114.9715 }, { 0, 0, 0, 1 }, 0xBC034559, { 0xA7873153 }, 0xA7873153 },
{ { 1776.807, -1377.969, 121.8078 }, { 0, 0, 0, 1 }, 0xFA35AB20, { 0xA7873153 }, 0xA7873153 },
{ { 1888.59, -1115.491, 113.2622 }, { 0, 0, 0, 1 }, 0x7AD9C108, { 0xA7873153 }, 0xA7873153 },
{ { 1659.733, -1435.086, 99.30989 }, { 0, 0, 0, 1 }, 0x8AC1D461, { 0xA7873153 }, 0xA7873153 },
{ { 1937.71, -1003.843, 109.3148 }, { 0, 0, 0, 1 }, 0xD4D7DB02, { 0xA7873153 }, 0xA7873153 },
{ { 1752.463, -1528.102, 120.7639 }, { 0, 0, 0, 1 }, 0x4F1B627F, { 0xA7873153 }, 0xA7873153 },
{ { 1616.531, -1734.391, 84.1243 }, { 0, 0, 0.1375311, 0.9904975 }, 0xCFB39052, { 0x34A51163 }, 0x34A51163 },
{ { 1906.385, -1060.762, 95.72266 }, { 0, 0, 0, 1 }, 0x585B8E2B, { 0x6C48C128 }, 0x592728C5 },
{ { 1816.286, -1291.433, 108.4415 }, { 0, 0, 0, 1 }, 0xB88B4370, { 0x6C48C128 }, 0x592728C5 },
{ { 1726.463, -1446.017, 110.0187 }, { 0, 0, 0, 1 }, 0xFCA3B152, { 0x6C48C128 }, 0x592728C5 },
{ { 1793.991, -1323.103, 98.70202 }, { 0, 0, 0.002191999, 0.9999976 }, 0x15168F41, { 0x6C48C128 }, 0x592728C5 },
{ { 1796.412, -1265.448, 92.67049 }, { 0, 0, 0, 1 }, 0x825DB21B, { 0x6C48C128 }, 0x592728C5 },
{ { 1735.467, -1454.337, 108.0533 }, { 0, 0, 0, 1 }, 0x1233D1C1, { 0x6C48C128 }, 0x592728C5 },
{ { 1895.578, -1047.201, 84.2215 }, { 0, 0, 0, 1 }, 0xE7F720E3, { 0x6C48C128 }, 0x592728C5 },
{ { 1649.522, -1408.329, 91.22415 }, { 0, 0, 0, 1 }, 0x0285FA58, { 0x6C48C128 }, 0x592728C5 },
{ { 1661.632, -1401.068, 93.44214 }, { 0, 0, 0, 1 }, 0x749B1696, { 0x6C48C128 }, 0x592728C5 },
{ { 1681.03, -1421.904, 94.93137 }, { 0, 0, 0, 1 }, 0xC9D29AB8, { 0x9B51CCA8, 0x823A8C70, 0x27608C62 }, 0x00000000 },
{ { 1792.684, -1348.117, 98.14455 }, { 0, 0, -0.8555983, 0.5176402 }, 0x134F68E5, { 0x84314B5B }, 0x00000000 },
{ { 1775.06, -1317.994, 93.62894 }, { 0, 0, -0.3428581, 0.9393872 }, 0x708D300F, { 0x94B7EC68 }, 0x00000000 },
{ { 1691.048, -1433.19, 111.3123 }, { 0, 0, 0.007253999, 0.9999737 }, 0x134F68E5, { 0x84314B5B }, 0x00000000 },
{ { 1867.461, -1129.224, 84.83058 }, { 0, 0, -0.9940834, -0.1086189 }, 0x708D300F, { 0x94B7EC68 }, 0x00000000 },
{ { 1834.557, -1194.536, 91.04294 }, { 0, 0, -0.9897142, 0.143058 }, 0x134F68E5, { 0x84314B5B }, 0x00000000 },
{ { 1834.47, -1167.126, 90.4431 }, { -0.0002890001, -0.0002170001, -0.08020504, 0.9967783 }, 0x708D300F, { 0x94B7EC68 }, 0x00000000 },
{ { 1881.953, -1022.006, 77.56586 }, { 0, 0, 0.06428702, 0.9979314 }, 0x134F68E5, { 0x84314B5B }, 0x00000000 },
{ { 1869.314, -1037.353, 78.04143 }, { 0, 0, -0.7640312, -0.6451792 }, 0x708D300F, { 0x94B7EC68 }, 0x00000000 },
{ { 2399.319, -1145.637, 89.76403 }, { 0, 0, 0.6293202, 0.777146 }, 0x40EE47B6, { 0x4F94AED6 }, 0x4F94AED6 },
{ { 2235.623, -843.5902, 84.78347 }, { 0, 0, 0, 1 }, 0xEDAC5EAB, { 0x4F94AED6 }, 0x4F94AED6 },
{ { 2208.423, -1181.429, 134.6817 }, { 0, 0, 0, 1 }, 0xA80AF371, { 0x4F94AED6 }, 0x4F94AED6 },
{ { 2331.988, -750.4926, 80.17238 }, { 0, 0, 0, 1 }, 0xC288B5E3, { 0x4F94AED6 }, 0x4F94AED6 },
{ { 2197.119, -1002.965, 133.7139 }, { 0, 0, 0, 1 }, 0xA0960AD0, { 0x4F94AED6 }, 0x4F94AED6 },
{ { 2335.133, -1244.499, 93.1234 }, { 0, 0, 0, 1 }, 0xB83830A5, { 0x4F94AED6 }, 0x4F94AED6 },
{ { 2326.663, -1088.687, 82.61159 }, { 0, 0, 0, 1 }, 0x1D25015A, { 0x4F94AED6 }, 0x4F94AED6 },
{ { 1953.167, -1150.296, 109.0459 }, { 0, 0, 0, 1 }, 0x98D09442, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 1986.102, -1022.711, 89.11707 }, { 0, 0, 0, 1 }, 0x1F73AF82, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 2080.186, -912.077, 97.78432 }, { 0, 0, 0, 1 }, 0x31C66DAD, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 2029.315, -1235.257, 150.0652 }, { 0, 0, 0, 1 }, 0x6513FE5D, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 2167.918, -1099.555, 170.4367 }, { 0, 0, 0, 1 }, 0x0AD96588, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 2184.588, -978.0229, 130.0246 }, { 0, 0, 0, 1 }, 0x99458B9A, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 2161.806, -849.8798, 79.13533 }, { 0, 0, 0, 1 }, 0x24C663AB, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 2069.991, -1074.805, 126.08 }, { 0, 0, 0, 1 }, 0x3D0101F8, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 2327.505, -925.6754, 114.8553 }, { 0, 0, 0, 1 }, 0xCF0FB990, { 0x4F94AED6 }, 0x4F94AED6 },
{ { 1984.942, -1301.764, 153.6524 }, { 0, 0, 0, 1 }, 0x31C64422, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 2033.895, -1227.682, 145.8042 }, { 0, 0, 0, 1 }, 0x8C812CBE, { 0x74F6C4D4 }, 0x9FCFFD36 },
{ { 2095.245, -974.0582, 122.5239 }, { 0, 0, 0, 1 }, 0xBDB170A0, { 0x74F6C4D4 }, 0x9FCFFD36 },
{ { 1992.055, -1158.041, 119.3622 }, { 0, 0, 0, 1 }, 0xB534C745, { 0x74F6C4D4 }, 0x9FCFFD36 },
{ { 2005.693, -992.4855, 89.44189 }, { 0, 0, 0, 1 }, 0xCB1160D4, { 0x74F6C4D4 }, 0x9FCFFD36 },
{ { 2120.139, -1045.432, 141.2913 }, { 0, 0, 0, 1 }, 0x68184F76, { 0x74F6C4D4 }, 0x9FCFFD36 },
{ { 2140.553, -924.0831, 115.3552 }, { 0, 0, 0, 1 }, 0x8831803A, { 0x74F6C4D4 }, 0x9FCFFD36 },
{ { 2228.522, -1905.681, 99.28503 }, { 0, 0, 0, 1 }, 0x817DF418, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 1848.645, -1861.684, 155.3343 }, { 0, 0, 0, 1 }, 0x1B036A55, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 2267.429, -1330.669, 111.6245 }, { 0, 0, 0, 1 }, 0xE7120CDA, { 0xFA0F3BA4 }, 0xFA0F3BA4 },
{ { 2046.133, -1895.396, 135.2884 }, { 0, 0, 0, 1 }, 0xF8336E35, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 2105.88, -1631.434, 204.4264 }, { 0, 0, 0, 1 }, 0x834E8E50, { 0xFA0F3BA4 }, 0xFA0F3BA4 },
{ { 2086.369, -2000.536, 91.38773 }, { 0, 0, 0, 1 }, 0x9DC69984, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 2245.331, -2015.193, 61.89204 }, { 0, 0, 0, 1 }, 0x34C39D10, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 1976.431, -1352.01, 164.6461 }, { 0, 0, 0, 1 }, 0x0D7A8D0B, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 2140.117, -1437.96, 175.617 }, { 0, 0, 0, 1 }, 0x08A93E96, { 0xFA0F3BA4 }, 0xFA0F3BA4 },
{ { 1931.62, -1314.35, 139.9613 }, { 0, 0, 0, 1 }, 0xDAD0E35E, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 1870.537, -1486.666, 146.4938 }, { 0, 0, 0, 1 }, 0xF12B7E32, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 2346.208, -2001.679, 46.75536 }, { 0, 0, 0, 1 }, 0xBBECFDA1, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 2038.261, -1715.655, 178.1059 }, { 0, 0, 0, 1 }, 0x0BDCDF41, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 1986.171, -1468.583, 187.0519 }, { 0, 0, 0, 1 }, 0xE630F0AB, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 2036.48, -1842.154, 128.1706 }, { 0, 0, 0.669066, 0.743203 }, 0xCEA64C5A, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 1942.681, -1502.441, 181.455 }, { 0, 0, 0.5482201, 0.8363341 }, 0xE388E4B1, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 1833.305, -1690.367, 149.6861 }, { 0, 0, 0, 1 }, 0xE64FE8EE, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 2155.797, -1297.085, 155.3921 }, { 0, 0, 0, 1 }, 0xD427ABF0, { 0xFA0F3BA4 }, 0xFA0F3BA4 },
{ { 2269.331, -1452.04, 125.0042 }, { 0, 0, 0.059219, 0.998245 }, 0x043E773F, { 0xFA0F3BA4 }, 0xFA0F3BA4 },
{ { 1592.172, -922.7185, 71.95197 }, { 0, 0, 0, 1 }, 0x64947283, { 0x46D4B359 }, 0x78BE8DB8 },
{ { 2073.979, -853.0886, 73.61191 }, { 0, 0, 0, 1 }, 0x5C652E69, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 1951.279, -1108.647, 104.2246 }, { 0, 0, 0, 1 }, 0x2ED58DFA, { 0xB0D70FEE }, 0xB0D70FEE },
{ { 1626.795, -951.5367, 62.66398 }, { 0, 0, 0, 1 }, 0xEBE71577, { 0xC181C6E4 }, 0x6FED3FB0 },
{ { 1811.964, -1137.675, 80.46782 }, { 0, 0, 0, 1 }, 0x097A5643, { 0xC181C6E4 }, 0xE9EBFF9B },
{ { 1897.173, -989.6202, 75.94279 }, { 0, 0, 0, 1 }, 0x09C63136, { 0xC181C6E4 }, 0xE9EBFF9B },
{ { 1844.635, -1502.795, 123.4053 }, { 0, 0, 0, 1 }, 0xDDAADC01, { 0x6A88CC5F }, 0x6A88CC5F },
{ { 1597.62, -1391.009, 77.76465 }, { 0, 0, 0, 1 }, 0x66D8D667, { 0xC181C6E4 }, 0x69838A09 },
{ { 1726.752, -1273.841, 83.60555 }, { 0, 0, 0, 1 }, 0x85DC18D6, { 0xC181C6E4 }, 0x69838A09 },
{ { 1635.304, -951.3137, 62.57839 }, { 0, 0, 0, 1 }, 0x54E294B8, { 0x04C23C9C, 0x4B95FFAD }, 0x00000000 },
{ { 1441.233, -1061.895, 105.1264 }, { 0, 0, 0, 1 }, 0x313AED9B, { 0x8755FC9B }, 0x00000000 },
{ { 1625.169, -1004.385, 131.4074 }, { 0, 0, 0, 1 }, 0xF726F970, { 0x8755FC9B }, 0x00000000 },
{ { 2348.19, -569.9466, 77.03905 }, { 0, 0, 0, 1 }, 0x413AF86D, { 0x55368F82 }, 0x55368F82 },
{ { 5230.601, -3658.283, 37.42877 }, { 0, 0, 0, 1 }, 0x09DCC702, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5427.531, -3685.496, 39.77926 }, { 0, 0, 0, 1 }, 0x96626279, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5547.625, -3714.948, 17.29472 }, { 0, 0, 0, 1 }, 0x70684C89, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5139.647, -3826.281, 88.66957 }, { 0, 0, 0, 1 }, 0xC704A6C2, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5362.846, -3858.354, 123.92 }, { 0, 0, 0, 1 }, 0x301CD36C, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5204.524, -3978.239, 106.2544 }, { 0, 0, 0, 1 }, 0xADD278A6, { 0x75DD554F }, 0xBF7B46D4 },
{ { 4941.529, -3934.013, 85.08528 }, { 0, 0, 0, 1 }, 0x7BB730B1, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5109.207, -4177.33, 4.091782 }, { 0, 0, 0, 1 }, 0xA9E6E84C, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5295.198, -4114.431, 10.64554 }, { 0, 0, 0, 1 }, 0xDB6625C7, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5496.224, -3957.268, 4.391566 }, { 0, 0, 0, 1 }, 0x93C71573, { 0x75DD554F }, 0xBF7B46D4 },
{ { 5194.502, -4144.602, 36.66399 }, { 0, 0, 0.5372996, 0.8433915 }, 0x2CD60B4C, { 0x5A6FA02B }, 0x8662B3D0 },
{ { 5278.891, -4158.204, 36.66399 }, { 0, 0, 0.258819, 0.9659259 }, 0x7FE95CE7, { 0x5A6FA02B }, 0x8662B3D0 },
{ { 5397.076, -4080.521, 36.66399 }, { 0, 0, 0.06104855, 0.9981348 }, 0xC9C0ACD9, { 0x5A6FA02B }, 0x8662B3D0 },
{ { 4874.165, -3889.083, 12.58546 }, { 0, 0, 0, 1 }, 0x911F4477, { 0x2F7A499E }, 0x920B4D29 },
{ { 4959.276, -4062.663, 12.84823 }, { 0, 0, 0, 1 }, 0x63CCEBFC, { 0x2F7A499E }, 0x920B4D29 },
{ { 5104.025, -4095.955, 13.22607 }, { 0, 0, 0, 1 }, 0x9FF937A2, { 0x2F7A499E }, 0x920B4D29 },
{ { 5057.172, -4166.026, 13.97361 }, { 0, 0, 0, 1 }, 0x86D4E004, { 0x2F7A499E }, 0x920B4D29 },
{ { 5094.655, -3978.971, 13.60893 }, { 0, 0, 0, 1 }, 0xACB4C513, { 0x2F7A499E }, 0x920B4D29 },
{ { 5085.887, -3872.02, 20.87367 }, { 0, 0, 0, 1 }, 0xF0589E56, { 0x2F7A499E }, 0x920B4D29 },
{ { 5087.856, -3774.62, 20.90761 }, { 0, 0, 0, 1 }, 0x47085B9C, { 0x2F7A499E }, 0x920B4D29 },
{ { 5180.737, -3650.181, 20.94887 }, { 0, 0, 0, 1 }, 0x1B81EFAA, { 0x2F7A499E }, 0x920B4D29 },
{ { 5195.865, -3870.872, 18.97625 }, { 0, 0, 0, 1 }, 0x9B0DFD5C, { 0x2F7A499E }, 0x920B4D29 },
{ { 5193.688, -3968.993, 13.30506 }, { 0, 0, 0, 1 }, 0xBF50CC6F, { 0x2F7A499E }, 0x920B4D29 },
{ { 5297.598, -3905.064, 15.29698 }, { 0, 0, 0, 1 }, 0xEF97C437, { 0x2F7A499E }, 0x920B4D29 },
{ { 5293.426, -3787.007, 20.88466 }, { 0, 0, 0, 1 }, 0x8F74A0CF, { 0x2F7A499E }, 0x920B4D29 },
{ { 5361.473, -3674.519, 16.91885 }, { 0, 0, 0, 1 }, 0x3C4114C7, { 0x2F7A499E }, 0x920B4D29 },
{ { 5448.569, -3612.152, 13.72686 }, { 0, 0, 0, 1 }, 0xF221F325, { 0x2F7A499E }, 0x920B4D29 },
{ { 5435.083, -3783.967, 13.06918 }, { 0, 0, 0, 1 }, 0x0D514983, { 0x2F7A499E }, 0x920B4D29 },
{ { 5403.083, -3876.417, 13.79765 }, { 0, 0, 0, 1 }, 0xFC44FF89, { 0x2F7A499E }, 0x920B4D29 },
{ { 5332.787, -4016.263, 13.1763 }, { 0, 0, 0, 1 }, 0x0592C7C8, { 0x2F7A499E }, 0x920B4D29 },
{ { 5444.389, -3980.072, 10.42879 }, { 0, 0, 0, 1 }, 0x47D97B4C, { 0x2F7A499E }, 0x920B4D29 },
{ { 5508.402, -3892.223, 9.960314 }, { 0, 0, 0, 1 }, 0xF5106A7A, { 0x2F7A499E }, 0x920B4D29 },
{ { 5611.843, -3665.026, 9.488731 }, { 0, 0, 0, 1 }, 0xA6FCAF1D, { 0x2F7A499E }, 0x920B4D29 },
{ { 5196.224, -3774.39, 20.86458 }, { 0, 0, 0, 1 }, 0x25D7FADB, { 0x2F7A499E }, 0x920B4D29 },
{ { 5554.864, -3704.672, 9.616348 }, { 0, 0, 0, 1 }, 0x4999B5DF, { 0x2F7A499E }, 0x920B4D29 },
{ { 4991.068, -3688.684, 28.96204 }, { 0, 0, 0, 1 }, 0xF4D1856D, { 0x652FB3F4 }, 0xAD39A251 },
{ { 5047.832, -3487.543, 41.81176 }, { 0, 0, 0, 1 }, 0xE639841C, { 0x652FB3F4 }, 0xAD39A251 },
{ { 4828.649, -3723.993, 77.94583 }, { 0, 0, 0, 1 }, 0x1C684D1F, { 0x652FB3F4 }, 0xAD39A251 },
{ { 4810.218, -3508.385, 23.20844 }, { 0, 0, 0, 1 }, 0xB7D3F676, { 0x652FB3F4 }, 0xAD39A251 },
{ { 4534.543, -3527.351, 33.13202 }, { 0, 0, 0, 1 }, 0xC32F2A47, { 0x652FB3F4 }, 0xAD39A251 },
{ { 5121.434, -3508.253, 16.49435 }, { 0, 0, -0.7071066, 0.7071066 }, 0xDEB39BA0, { 0xEAC1D5CA }, 0xE573E70E },
{ { 5044.848, -3508.379, 13.92477 }, { 0, 0, -0.7071066, 0.7071066 }, 0x4FEAA087, { 0xEAC1D5CA }, 0xE573E70E },
{ { 5114.892, -3440.936, 16.00085 }, { 9E-08, 0, 0, 1 }, 0xBA167CC1, { 0xEAC1D5CA }, 0xE573E70E },
{ { 5041.696, -3578.546, 16.00701 }, { 0, 0, -0.7071066, 0.7071066 }, 0xF62150DB, { 0x16531C97 }, 0x1DC5930C },
{ { 4954.092, -3747.522, 8.17159 }, { 9E-08, 0, 0, 1 }, 0xEEFE1B90, { 0x16531C97 }, 0x1DC5930C },
{ { 4943.822, -3753.523, 3.929602 }, { 0, 0, -0.5, 0.8660254 }, 0xC2F32157, { 0x16531C97 }, 0x1DC5930C },
{ { 4866.995, -3638.826, 6.274557 }, { 0, 0, 0.258819, 0.9659259 }, 0xFF463A5D, { 0x16531C97 }, 0x1DC5930C },
{ { 4899.617, -3676.126, 6.151159 }, { 0, 0, 0.258819, 0.9659259 }, 0xD5D0E3BD, { 0x16531C97 }, 0x1DC5930C },
{ { 4939.919, -3547.711, 16.12172 }, { 0, 0, 0.7071066, 0.7071066 }, 0x6E2198FC, { 0xEAC1D5CA }, 0xE573E70E },
{ { 4940.329, -3455.146, 16.24631 }, { 0, 0, 1, 8E-08 }, 0x535BB5C3, { 0x7DA16D6B }, 0xF2F7C276 },
{ { 4938.504, -3827.925, 8.652489 }, { 0, 0, 0.258819, 0.9659259 }, 0x8777A0BD, { 0x25F01606 }, 0x494F8DEC },
{ { 4892.975, -3748.808, 8.753193 }, { 0, 0, 0.258819, 0.9659259 }, 0xF8073349, { 0x25F01606 }, 0x494F8DEC },
{ { 4692.313, -3540.673, 11.70124 }, { 0, 0, 0.3826833, 0.9238795 }, 0x93C29187, { 0x7DA16D6B }, 0xF2F7C276 },
{ { 4847.524, -3638.521, 8.896328 }, { 0, 0, 0.9659258, -0.2588191 }, 0x23B3671E, { 0x25F01606 }, 0x494F8DEC },
{ { 4814.986, -3591.985, 8.922886 }, { 0, 0, 0.9659258, -0.2588192 }, 0x0FA6D55E, { 0x7DA16D6B }, 0xF2F7C276 },
{ { 4677.835, -3423.133, 11.87955 }, { 0, 0, 0, 1 }, 0x20EBB7F1, { 0x7DA16D6B }, 0xF2F7C276 },
{ { 4961.036, -3440.593, 19.5739 }, { 0, 0, 0, 1 }, 0x5FC1CCDA, { 0xEAC1D5CA }, 0xE573E70E },
{ { 4972.05, -3432.203, 19.57024 }, { 0, 0, 0, 1 }, 0xCA455C0C, { 0xEAC1D5CA }, 0xE573E70E },
{ { 4991.172, -3432.203, 19.57024 }, { 0, 0, 0, 1 }, 0x925E158E, { 0xEAC1D5CA }, 0xE573E70E },
{ { 5020.12, -3600.345, 30.1941 }, { 0, 0, 0, 1 }, 0x2B7B4175, { 0x16531C97 }, 0x1DC5930C },
{ { 4998.078, -3600.345, 30.1941 }, { 0, 0, 0, 1 }, 0xFF323638, { 0x16531C97 }, 0x1DC5930C },
{ { 5009.01, -3593.16, 25.90114 }, { 0, 0, 0, 1 }, 0x8794D944, { 0x16531C97 }, 0x1DC5930C },
{ { 5093.573, -3616.45, 24.09729 }, { 0, 0, 0, 1 }, 0x83636B4D, { 0x16531C97 }, 0x1DC5930C },
{ { 5078.775, -3664.518, 24.09017 }, { 0, 0, 0, 1 }, 0x9F6FAEC3, { 0x16531C97 }, 0x1DC5930C },
{ { 5118.26, -3664.652, 24.09017 }, { 0, 0, 0, 1 }, 0x746CF2D9, { 0x16531C97 }, 0x1DC5930C },
{ { 5093.573, -3712.906, 24.09729 }, { 0, 0, 0, 1 }, 0x2BD8CF54, { 0x16531C97 }, 0x1DC5930C },
{ { 5068.333, -3664.575, 25.29918 }, { 0, 0, 0, 1 }, 0xE5882013, { 0x16531C97 }, 0x1DC5930C },
{ { 4581.208, -3543.05, 9.719808 }, { 0, 0, 0, 1 }, 0x1E8EA38A, { 0x91BC3529 }, 0x831AC67C },
{ { 4681.393, -3623.039, 7.005788 }, { 0, 0, 0, 1 }, 0xBC01D119, { 0x91BC3529 }, 0x831AC67C },
{ { 4878.398, -3636.084, 16.44633 }, { 0.6123731, -0.3535537, -0.3535527, 0.612372 }, 0x720A82E3, { 0x16531C97 }, 0x1DC5930C },
{ { 5023.563, -3524.967, 20.96713 }, { 0, 0, 0, 1 }, 0x57387830, { 0xEAC1D5CA }, 0xE573E70E },
{ { 4973.519, -3514.983, 22.3148 }, { 0, 0, 0, 1 }, 0xA8F22057, { 0xEAC1D5CA }, 0xE573E70E },
{ { 5057.256, -3516.69, 18.28863 }, { 0, 0, 0, 1 }, 0x7E8DA016, { 0xEAC1D5CA }, 0xE573E70E },
{ { 4991.977, -3528.111, 20.537 }, { 0, 0, 0, 1 }, 0xC47DE79B, { 0xEAC1D5CA }, 0xE573E70E },
{ { 4529.18, -3503.302, 12.65939 }, { 0, 0, -9E-08, 1 }, 0x30A4A388, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4521.33, -3497.243, 6.010113 }, { 0, 0, -0.3826833, 0.9238795 }, 0xC586977F, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4472.021, -3416.472, 12.37265 }, { 0, 0, -9E-08, 1 }, 0x57F543CF, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4476.348, -3415.615, 5.729432 }, { 0, 0, -0.7071068, 0.7071066 }, 0x9C48A5E2, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4558.738, -3430.788, 12.66166 }, { 0, 0, -9E-08, 1 }, 0xA14A747B, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4561.21, -3432.024, 6.495557 }, { 0, 0, -0.7071068, 0.7071066 }, 0x56CD7201, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4644.522, -3524.674, 14.39379 }, { 0, 0, -0.3826829, 0.9238797 }, 0x7F339CA1, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4648.812, -3527.184, 8.050386 }, { 0, 0, -0.3826833, 0.9238795 }, 0x6843E376, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4587.288, -3589.81, 12.73932 }, { 0, 0, -9E-08, 1 }, 0x6CEB8E43, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4562.157, -3587.693, 5.688617 }, { 0, 0, 0, 1 }, 0x09954494, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4666.099, -3628.293, 12.60665 }, { 0, 0, 0, 1 }, 0x9F79DE70, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4655.555, -3651.04, 5.576724 }, { 0, 0, -0.3826833, 0.9238795 }, 0x310D10AA, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4766.475, -3697.618, 12.91185 }, { 0, 0, -0.7071068, 0.7071066 }, 0x418CF0A7, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4850.319, -3799.569, 6.612502 }, { 0, 0, -3.5E-07, 1 }, 0x17E96346, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4901.509, -3731.292, 5.852348 }, { 0, 0, 1, 8E-08 }, 0x5593A1C6, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4955.949, -3746.492, 14.24874 }, { 0, 0, 0, 1 }, 0x54617C43, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5029.106, -3697.01, 24.91603 }, { 0, 0, 0, 1 }, 0xB4248F38, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5037.714, -3634.063, 25.67468 }, { 0, 0, -0.7071068, 0.7071066 }, 0x3A90C3B5, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5035.445, -3639.889, 15.465 }, { 0, 0, 0, 1 }, 0x59A80EFB, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4995.923, -3640.262, 24.95383 }, { 0, 0, 1, 8E-08 }, 0xCA461A54, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4997.522, -3639.704, 15.01305 }, { 0, 0, 0, 1 }, 0x04007605, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4965.896, -3589.356, 25.31253 }, { 0, 0, -0.7071068, 0.7071066 }, 0x86B548F9, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4964.892, -3589.078, 14.97056 }, { 0, 0, 0, 1 }, 0xCFDF2B1D, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5013.322, -3572.911, 19.11747 }, { 0, 0, 0, 1 }, 0xCE4FCC26, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5013.043, -3584.916, 15.31343 }, { 0, 0, 0, 1 }, 0x723F044C, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4849.928, -3561.188, 14.03581 }, { 0, 0, -0.7071068, 0.7071066 }, 0x7B03D289, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4754.043, -3544.528, 14.58851 }, { 0, 0, -0.7071068, 0.7071066 }, 0xA3BCCF93, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4734.178, -3566.608, 7.418481 }, { 0, 0, -0.3826833, 0.9238795 }, 0xCC888C23, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4660.368, -3470.763, 12.80745 }, { 0, 0, -0.3826833, 0.9238795 }, 0xD1BA6E90, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4668.714, -3465.652, 8.725426 }, { 0, 0, -0.7071068, 0.7071066 }, 0xABA1BA46, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4611.872, -3491.801, 14.18815 }, { 0, 0, -0.7071068, 0.7071066 }, 0x228CF2CA, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4631.752, -3488.209, 7.754668 }, { 0, 0, -9E-08, 1 }, 0xBFBB966F, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4622.255, -3369.465, 14.02813 }, { 0, 0, -9E-08, 1 }, 0x1C52D3D0, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4629.227, -3365.865, 7.160995 }, { 0, 0, -0.3826833, 0.9238795 }, 0xE7F7D538, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4784.648, -3413.271, 20.82771 }, { 0, 0, -0.3826833, 0.9238795 }, 0x2029EA8B, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4886.678, -3419.177, 22.13092 }, { 0, 0, 0, 1 }, 0x5AB196FD, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4945.83, -3454.432, 21.81544 }, { 0, 0, -9E-08, 1 }, 0xB3952B20, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5020.353, -3412.607, 22.26728 }, { 0, 0, 0.9238796, -0.3826834 }, 0x22D78DA9, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5094.68, -3441.453, 22.14004 }, { 0, 0, -0.3826833, 0.9238795 }, 0x0AFE7FF5, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5017.264, -3520.437, 22.51163 }, { 0, 0, 0, 1 }, 0x29F6840E, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5094.206, -3578.262, 19.08375 }, { 0, 0, 0, 1 }, 0xB2C7FEEC, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5076.854, -3636.689, 18.91444 }, { 0, 0, 0, 1 }, 0x1244260B, { 0x02A7D75B }, 0xF7BC8239 },
{ { 5076.854, -3691.028, 18.81274 }, { 0, 0, 0, 1 }, 0xD25A7B74, { 0x02A7D75B }, 0xF7BC8239 },
{ { 4712.417, -2781.363, 25.06914 }, { 0, 0, 0, 1 }, 0xD40D64D9, { 0x5A719E78 }, 0x18AA7931 },
{ { 4712.666, -2945.777, 20.4967 }, { 0, 0, 0, 1 }, 0xC1EDC09A, { 0x5A719E78 }, 0x18AA7931 },
{ { 4732.606, -3271.739, 41.60819 }, { 0, 0, 0, 1 }, 0xE8C98E51, { 0x5A719E78 }, 0x18AA7931 },
{ { 4722.443, -3114.7, 52.56467 }, { 0, 0, 0, 1 }, 0xF8902DDE, { 0x5A719E78 }, 0x18AA7931 },
{ { 4667.024, -2994.599, 13.01223 }, { 0, 0, 9E-08, 1 }, 0xD23AA7D5, { 0x772F14B0 }, 0xC4310B76 },
{ { 4642.354, -2787.502, 10.73352 }, { 0, 0, 9E-08, 1 }, 0xD30778C8, { 0x772F14B0 }, 0xC4310B76 },
{ { 4689.196, -3133.128, 14.01472 }, { 0, 0, 0, 1 }, 0xEF8F1E85, { 0x772F14B0 }, 0xC4310B76 },
{ { 4681.901, -3341.023, 11.94318 }, { 0, 0, 0, 1 }, 0xE5775640, { 0x772F14B0 }, 0xC4310B76 },
{ { 4779.131, -2894.102, 50.67519 }, { 2.4E-07, 0, -9E-08, 1 }, 0xF10BD361, { 0xA2321D9A }, 0x1EFC163D },
{ { 4806.711, -2977.166, 16.3936 }, { 0, 0, 0, 1 }, 0x085EF556, { 0xA2321D9A }, 0x1EFC163D },
{ { 4799.187, -2970.453, 54.98525 }, { 0, 0, 0, 1 }, 0x43B2290E, { 0xA2321D9A }, 0x1EFC163D },
{ { 4688.291, -2968.893, 53.32533 }, { 0, 0, 0, 1 }, 0xBD14BE46, { 0xA2321D9A }, 0x1EFC163D },
{ { 4699.688, -2910.177, 14.60714 }, { 0, 0, 0, 1 }, 0x5A607753, { 0xA2321D9A }, 0x1EFC163D },
{ { 4661.387, -2958.001, 14.49324 }, { 0, 0, 0, 1 }, 0x41171515, { 0xA2321D9A }, 0x1EFC163D },
{ { 4758.123, -2752.093, 39.13006 }, { 0, 0, 0, 1 }, 0xFDA90CC8, { 0xA2321D9A }, 0x1EFC163D },
{ { 4796.242, -2750.959, 30.76378 }, { 0, 0, 0, 1 }, 0x3659C541, { 0xA2321D9A }, 0x1EFC163D },
{ { 4790.251, -2726.678, 15.96586 }, { 0, 0, 0, 1 }, 0x9129576C, { 0xA2321D9A }, 0x1EFC163D },
{ { 4699.493, -2751.5, 17.20886 }, { 0, 0, 0, 1 }, 0xC149157A, { 0xA2321D9A }, 0x1EFC163D },
{ { 4801.358, -2845.687, 23.48966 }, { 0, 0, 0, 1 }, 0x592F6FFB, { 0xA2321D9A }, 0x1EFC163D },
{ { 4766.17, -2823.205, 29.2198 }, { 0, 0, 0, 1 }, 0x8B99CCE3, { 0xA2321D9A }, 0x1EFC163D },
{ { 4702.676, -2813.494, 14.69778 }, { 0, 0, 0, 1 }, 0x22B6B155, { 0xA2321D9A }, 0x1EFC163D },
{ { 4770.625, -3037.272, 102.6325 }, { 0, 0, 0, 1 }, 0x3F34BBD0, { 0xA2321D9A }, 0x1EFC163D },
{ { 4800.2, -3045.47, 38.57456 }, { 0, 0, 0, 1 }, 0xA2199E60, { 0xA2321D9A }, 0x1EFC163D },
{ { 4805.039, -3054.802, 15.79767 }, { 0, 0, 0, 1 }, 0x61B04412, { 0xA2321D9A }, 0x1EFC163D },
{ { 4751.27, -3134.927, 32.70834 }, { 0, 0, 0, 1 }, 0x861DDC3F, { 0xA2321D9A }, 0x1EFC163D },
{ { 4789.244, -3125.605, 65.25184 }, { 0, 0, 0, 1 }, 0x28DF0F69, { 0xA2321D9A }, 0x1EFC163D },
{ { 4766.627, -3184.064, 77.29416 }, { 0, 0, 0, 1 }, 0xD2DE2A6A, { 0xA2321D9A }, 0x1EFC163D },
{ { 4784.851, -3198.568, 53.84061 }, { 0, 0, 0, 1 }, 0x56DA2295, { 0xA2321D9A }, 0x1EFC163D },
{ { 4795.724, -3268.833, 59.27673 }, { 0, 0, 0, 1 }, 0x17DD0712, { 0xA2321D9A }, 0x1EFC163D },
{ { 4802.014, -3352.251, 83.89646 }, { 0, 0, 0, 1 }, 0xB000690F, { 0xA2321D9A }, 0x1EFC163D },
{ { 4754.377, -3391.632, 49.02041 }, { 0, 0, 0, 1 }, 0xB91F27C1, { 0xA2321D9A }, 0x1EFC163D },
{ { 4752.177, -3338.707, 26.77628 }, { 0, 0, 0, 1 }, 0xB2132EF8, { 0xA2321D9A }, 0x1EFC163D },
{ { 4757.78, -3295.685, 17.6983 }, { 0, 0, 0, 1 }, 0x701EB780, { 0xA2321D9A }, 0x1EFC163D },
{ { 4709.397, -3179.25, 25.7429 }, { 0, 0, 0, 1 }, 0xFE618543, { 0xA2321D9A }, 0x1EFC163D },
{ { 4713.859, -3252.045, 26.03499 }, { 0, 0, 0, 1 }, 0xC74ACBED, { 0xA2321D9A }, 0x1EFC163D },
{ { 4706.148, -3283.962, 28.74548 }, { 0, 0, 0, 1 }, 0xBB923B16, { 0xA2321D9A }, 0x1EFC163D },
{ { 4710.816, -3334.742, 28.70299 }, { 0, 0, 0, 1 }, 0x5D46C901, { 0xA2321D9A }, 0x1EFC163D },
{ { 4708.27, -3371.929, 28.09167 }, { 0, 0, 0, 1 }, 0x8D653487, { 0xA2321D9A }, 0x1EFC163D },
{ { 4774.441, -3317.446, 13.94975 }, { 0, 0, 0, 1 }, 0xDC58E46C, { 0xA2321D9A }, 0x1EFC163D },
{ { 4706.649, -3205.781, 39.22861 }, { 0, 0, 0, 1 }, 0x7329E9C2, { 0xA2321D9A }, 0x1EFC163D },
{ { 4804.978, -3239.541, 15.88659 }, { 0, 0, 0, 1 }, 0xEA11B25C, { 0xA2321D9A }, 0x1EFC163D },
{ { 4804.898, -3211.834, 16.25199 }, { 0, 0, 0, 1 }, 0xA7E3D85C, { 0xA2321D9A }, 0x1EFC163D },
{ { 4774.961, -3128.601, 9.771055 }, { 0, 0, 0, 1 }, 0x3780FDD4, { 0xA2321D9A }, 0x1EFC163D },
{ { 4758.684, -3095.18, 31.59566 }, { 0, 0, 0, 1 }, 0x6A70C42A, { 0xA2321D9A }, 0x1EFC163D },
{ { 4765.313, -2953.923, 35.0072 }, { 0, 0, 0, 1 }, 0xEBCCFD8F, { 0xA2321D9A }, 0x1EFC163D },
{ { 4704.184, -3066.896, 12.57941 }, { 0, 0, -1.5E-07, 1 }, 0x82A20A08, { 0xA2321D9A }, 0x1EFC163D },
{ { 4741.386, -3116.247, 11.35921 }, { 0, 0, 0, 1 }, 0x3DF290F7, { 0xC39A5493 }, 0x821B3E32 },
{ { 4740.437, -3318.253, 18.15306 }, { 0, 0, 0, 1 }, 0xBC0419C8, { 0xC2466C3F }, 0x09617C7A },
{ { 4742.832, -3170.041, 15.8704 }, { 0, 0, 0, 1 }, 0x1EE86C2F, { 0xC2466C3F }, 0x09617C7A },
{ { 4779.593, -3012.199, 17.98801 }, { 0, 0, 0, 1 }, 0x18777F0F, { 0xC2466C3F }, 0x09617C7A },
{ { 4810.086, -2830.67, 19.34821 }, { 0, 0, 0, 1 }, 0xF59396E7, { 0xC2466C3F }, 0x09617C7A },
{ { 4842.843, -2622.574, 102.5265 }, { 0, 0, 0, 1 }, 0x7F979C31, { 0x84E8F36E }, 0x989CF918 },
{ { 4733.874, -2424.701, 52.94262 }, { 0, 0, 0, 1 }, 0x8E50085C, { 0x84E8F36E }, 0x989CF918 },
{ { 4559.715, -2230.69, 6.548146 }, { 0, 0, 0, 1 }, 0x2FAA150A, { 0x84E8F36E }, 0x989CF918 },
{ { 4575.861, -2445.719, -67.25586 }, { 0, 0, 0, 1 }, 0x7C1A90EF, { 0x84E8F36E }, 0x989CF918 },
{ { 4725.129, -2194.327, 50.80219 }, { 0, 0, 0, 1 }, 0xAA760836, { 0x84E8F36E }, 0x989CF918 },
{ { 4632.891, -2617.145, 13.73739 }, { 0, 0, 0, 1 }, 0x5C66BF9D, { 0x84E8F36E }, 0x989CF918 },
{ { 4613.27, -2120.028, 8.879774 }, { 0, 0, -0.7171254, 0.6969441 }, 0x00A275C8, { 0xD467EB00 }, 0x4090D78D },
{ { 4640.057, -2117.003, 12.62117 }, { 0, 0, -9E-08, 1 }, 0xB893CDD4, { 0xD467EB00 }, 0x4090D78D },
{ { 4696.167, -2464.037, 12.70044 }, { 0, 0, -2E-08, 1 }, 0x103CE4DB, { 0x405A2227 }, 0x806CD279 },
{ { 4755.982, -2511.173, 8.969593 }, { 0, 0, 0.7071071, 0.7071064 }, 0x3AD5A7CA, { 0x405A2227 }, 0x806CD279 },
{ { 4749.288, -2509.801, 8.963184 }, { 0, 0, -0.7171254, 0.6969441 }, 0xBDE82A4C, { 0x405A2227 }, 0x806CD279 },
{ { 4567.279, -2156.502, 10.33582 }, { -1E-08, 1E-08, 0.7071069, 0.7071066 }, 0x9A77B5A3, { 0xCB978568 }, 0xAC502B40 },
{ { 4692.819, -2444.589, 8.992138 }, { 0, 0, 0, 1 }, 0xB9C461D7, { 0x405A2227 }, 0x806CD279 },
{ { 4555.31, -2109.934, 8.88957 }, { 0, 0, 0.9986475, 0.05199193 }, 0x350A31EF, { 0xCB978568 }, 0xAC502B40 },
{ { 4554.236, -2153.682, 8.941545 }, { 0, 0, -0.6619566, 0.7495422 }, 0x818F6553, { 0xCB978568 }, 0xAC502B40 },
{ { 4575.515, -2206.974, 10.49444 }, { 0, 0, 0.08582594, 0.9963101 }, 0xA4B6C8C7, { 0xCB978568 }, 0xAC502B40 },
{ { 4553.801, -2128.164, 11.7066 }, { 0, 0, -9E-08, 1 }, 0x8CE37E87, { 0xCB978568 }, 0xAC502B40 },
{ { 4562.414, -2430.058, 13.54295 }, { 0, 0, 0, 1 }, 0x21468F3B, { 0x51E770D4 }, 0x98C3878A },
{ { 4538.143, -2301.209, 14.7882 }, { 0, 0, 9E-08, 1 }, 0x942B54E4, { 0xCB978568 }, 0xAC502B40 },
{ { 4568.133, -2211.805, 17.62 }, { 0, 0, 0, 1 }, 0x7795E52D, { 0xCB978568 }, 0xAC502B40 },
{ { 4662.646, -2130.098, 30.77932 }, { 0, 0, 0, 1 }, 0xB6167332, { 0x405A2227 }, 0x806CD279 },
{ { 4916.904, -2672.632, 87.54901 }, { 0, 0, 0, 1 }, 0xD428CE2D, { 0x405A2227 }, 0x806CD279 },
{ { 4678.634, -2360.396, 56.36315 }, { 0, 0, 0, 1 }, 0x7C80915E, { 0x405A2227 }, 0x806CD279 },
{ { 4539.97, -2249.065, 13.54456 }, { 0, 0, 0, 1 }, 0xB7456924, { 0xCB978568 }, 0xAC502B40 },
{ { 4536.499, -2104.632, 22.70459 }, { 0, -9E-08, 1, -4E-08 }, 0xC30717AF, { 0xCB978568 }, 0xAC502B40 },
{ { 4654.039, -2625.696, 10.62425 }, { 0, 0, 0, 1 }, 0x2D3060E8, { 0x85438CBE }, 0x267ED54D },
{ { 4656.072, -2708.335, 11.12999 }, { 0, 0, 0, 1 }, 0x24CDD32D, { 0x85438CBE }, 0x267ED54D },
{ { 4811.096, -2371.088, 13.66213 }, { 0, 0, -9E-08, 1 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4566.365, -2182.547, 9.764145 }, { 0, 0, 0, 1 }, 0x66D88663, { 0xCB978568 }, 0xAC502B40 },
{ { 4559.265, -2140.351, 9.852196 }, { 0, 0, 0, 1 }, 0x9B821779, { 0xCB978568 }, 0xAC502B40 },
{ { 4575.049, -2126.523, 9.852196 }, { 0, 0, 0, 1 }, 0x8B2429DC, { 0xCB978568 }, 0xAC502B40 },
{ { 4537.327, -2122.272, 9.627476 }, { 0, 0, 0, 1 }, 0x67FF247D, { 0xCB978568 }, 0xAC502B40 },
{ { 4570.379, -2129.332, 10.5856 }, { 0, 0, 0, 1 }, 0x126F257A, { 0xCB978568 }, 0xAC502B40 },
{ { 4567.247, -2202.691, 10.20053 }, { 0, 0, 0, 1 }, 0xDB1C3D6C, { 0xCB978568 }, 0xAC502B40 },
{ { 4614.495, -2110.42, 8.783386 }, { 0, 0, -0.04568202, 0.998956 }, 0xBBD3BD21, { 0xD467EB00 }, 0x4090D78D },
{ { 4747.178, -2500.385, 8.956686 }, { 0, 0, -0.7089429, 0.7052658 }, 0x23F764E1, { 0x405A2227 }, 0x806CD279 },
{ { 4483.401, -2304.683, -0.6737752 }, { 0, 0, 0, 1 }, 0xD854BA55, { 0x0E625D8F }, 0x1E2AF8D7 },
{ { 4809.335, -2290.654, 16.03786 }, { 0, 0, 0, 1 }, 0xC3A835F0, { 0xD40B6742 }, 0xF5680D3A },
{ { 4620.689, -2595.01, 6.361023 }, { 0, 0, 0, 1 }, 0xC550BA50, { 0x85438CBE }, 0x267ED54D },
{ { 4520.978, -2324.474, 6.829353 }, { 0, 0, 0, 1 }, 0xE5F02C09, { 0xCB978568 }, 0xAC502B40 },
{ { 4534.58, -2378.939, 6.318039 }, { 0, 0, 0, 1 }, 0x599130F8, { 0x51E770D4 }, 0x98C3878A },
{ { 4558.37, -2432.997, 7.239059 }, { 0, 0, 0, 1 }, 0xDC91F9B5, { 0x51E770D4 }, 0x98C3878A },
{ { 4592.917, -2520.401, 6.213882 }, { 0, 0, 0, 1 }, 0x82CE322C, { 0x51E770D4 }, 0x98C3878A },
{ { 4632.125, -2688.514, 6.392218 }, { 0, 0, 0, 1 }, 0xC3DC6169, { 0x85438CBE }, 0x267ED54D },
{ { 4811.096, -2143.687, 13.69195 }, { 0, 0, 0, 1 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4811.096, -2221.987, 13.66213 }, { 0, 0, 0, 1 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4837.821, -2141.364, 13.76533 }, { 0, 0, 1, -8E-08 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4837.82, -2280.275, 13.71424 }, { 0, 0, 1, 8E-08 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4837.807, -2403.406, 13.70181 }, { 0, 0, 1, 8E-08 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4811.096, -2476.993, 13.66213 }, { 0, 0, 0, 1 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4837.848, -2524.713, 13.70796 }, { 0, 0, 1, -8E-08 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4811.096, -2668.63, 13.78424 }, { 0, 0, 0, 1 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4811.096, -2694.312, 13.78424 }, { 0, 0, 0, 1 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4837.748, -2659.657, 13.78375 }, { 0, 0, 1, -8E-08 }, 0x7D817DF8, { 0x405A2227 }, 0x806CD279 },
{ { 4814.962, -2514.077, 14.28599 }, { 0, 0, 0, 1 }, 0x5BAC76F4, { 0x5BAC76F4 }, 0x27A102A5 },
{ { 4834.083, -2601.799, 14.10143 }, { 0, 0, 0, 1 }, 0xD41F9B9C, { 0x7A044675 }, 0xC4F4EA75 },
{ { 4814.213, -2522.046, 14.10143 }, { 0, 0, 0, 1 }, 0x390CDC4D, { 0x405A2227 }, 0x806CD279 },
{ { 4688.044, -2457.277, 40.29506 }, { 0, 0, 0, 1 }, 0x20B521F9, { 0x405A2227 }, 0x806CD279 },
{ { 4774.261, -2446.45, 47.39162 }, { 0, 0, 0, 1 }, 0x2671776F, { 0x405A2227 }, 0x806CD279 },
{ { 4777.386, -2283.813, 68.41401 }, { 0, 0, 0, 1 }, 0x0A6BBE92, { 0x405A2227 }, 0x806CD279 },
{ { 4774.673, -2169.825, 58.22137 }, { 0, 0, 0, 1 }, 0x0C8F58D9, { 0x405A2227 }, 0x806CD279 },
{ { 4673.307, -2164.255, 22.30664 }, { 0, 0, 0, 1 }, 0x092F3F38, { 0x2F7A499E }, 0x1077074C },
{ { 4830.307, -2218.466, 19.16458 }, { 0, 0, -0.3826833, 0.9238795 }, 0x78E83D67, { 0x2F7A499E }, 0x1077074C },
{ { 4522.417, -2290.072, 19.90259 }, { 0, 0, -0.3826833, 0.9238795 }, 0xE76CD004, { 0x2F7A499E }, 0x1077074C },
{ { 4684.582, -2309.04, 20.00293 }, { 0, 0, -9E-08, 1 }, 0x6AD4EAE9, { 0x2F7A499E }, 0x1077074C },
{ { 4575.876, -2418.635, 19.88004 }, { 0, 0, 1, 8E-08 }, 0xEDF780BB, { 0x2F7A499E }, 0x1077074C },
{ { 4791.113, -2439.76, 16.82813 }, { 0, 0, -0.7071068, 0.7071066 }, 0x0B74BF29, { 0x2F7A499E }, 0x1077074C },
{ { 4619.607, -2583.866, 16.69086 }, { 0, 0, -0.3826833, 0.9238795 }, 0xA87D73F2, { 0x2F7A499E }, 0x1077074C },
{ { 4846.749, -2628.493, 24.56456 }, { 0, 0, -0.7071068, 0.7071066 }, 0x73AB84F6, { 0x2F7A499E }, 0x1077074C },
{ { 4609.26, -2880.508, -23.1006 }, { 0, 0, 0, 1 }, 0xB3D013F1, { 0x32BEF73A }, 0xEBDB54D5 },
{ { 4482.309, -2530.447, -20.44253 }, { 0, 0, 0, 1 }, 0xFDBA7B4E, { 0x32BEF73A }, 0xEBDB54D5 },
{ { 5120.414, -1947.81, 36.70941 }, { 0, 0, 0, 1 }, 0x1CFC86F4, { 0xF2CA46FF }, 0xBA51BC37 },
{ { 5073.788, -1762.377, 21.62747 }, { 0, 0, 0, 1 }, 0xAA13F2DD, { 0xF2CA46FF }, 0xBA51BC37 },
{ { 4703.208, -2004.788, 20.91621 }, { 0, 0, 0, 1 }, 0x04BB9028, { 0xF2CA46FF }, 0xBA51BC37 },
{ { 4913.755, -1914.863, 42.65476 }, { 0, 0, 0, 1 }, 0x2CBFEDCE, { 0xF2CA46FF }, 0xBA51BC37 },
{ { 4701.254, -1820.481, 27.66876 }, { 0, 0, 0, 1 }, 0x2D469239, { 0xF2CA46FF }, 0xBA51BC37 },
{ { 4916.553, -1752.911, 27.42437 }, { 0, 0, 0, 1 }, 0x6AC6F156, { 0xF2CA46FF }, 0xBA51BC37 },
{ { 4963.679, -2037.707, 17.93289 }, { 0, 0, 0, 1 }, 0x941B52E8, { 0x941B52E8 }, 0x62E6AFA4 },
{ { 4766.242, -1772.329, 18.06403 }, { 0, 0, 0, 1 }, 0xC7846E94, { 0x5F733F32 }, 0x34ADB876 },
{ { 4824.472, -1817.594, 22.49409 }, { 0, 0, 0, 1 }, 0x587A4E06, { 0x03A5E387 }, 0x60C9977A },
{ { 4824.467, -1854.348, 27.75582 }, { 0, 0, 0, 1 }, 0x8710CB2B, { 0xD4443834 }, 0x054C9459 },
{ { 4824.596, -1782.881, 27.75582 }, { 0, 0, 0, 1 }, 0x0964EC2F, { 0xD4443834 }, 0x054C9459 },
{ { 4824.465, -1817.591, 27.75582 }, { 0, 0, 0, 1 }, 0xB480C7CC, { 0xD4443834 }, 0x054C9459 },
{ { 4782.586, -2048.935, 63.93446 }, { 0, 0, 0, 1 }, 0xCEE4E3E6, { 0x258A2916 }, 0x50259B69 },
{ { 4776.559, -1943.487, 67.1916 }, { 0, 0, 0, 1 }, 0x0239E59F, { 0x258A2916 }, 0x50259B69 },
{ { 4751.106, -1957.929, 55.71543 }, { 0, 0, 0, 1 }, 0xB301738C, { 0xEDFD05ED }, 0x796CAC78 },
{ { 4649.399, -1998.338, 108.8947 }, { 0, 0, 0, 1 }, 0x33DD2E65, { 0xEDFD05ED }, 0x796CAC78 },
{ { 4683.034, -1948.79, 108.8947 }, { 0, 0, 0, 1 }, 0x4E78F524, { 0xEDFD05ED }, 0x796CAC78 },
{ { 4679.464, -2049.251, 108.8947 }, { 0, 0, -9E-08, 1 }, 0xCF2340AF, { 0xEDFD05ED }, 0x796CAC78 },
{ { 4824.484, -1902.068, 23.01969 }, { 0, 0, 0, 1 }, 0xE5FE2217, { 0xAAF84021 }, 0x86440782 },
{ { 4824.359, -2003.216, 15.83303 }, { 0, 0, 0, 1 }, 0x848F58C9, { 0xAAF84021 }, 0x86440782 },
{ { 4904.051, -1700.722, 19.6798 }, { 0, 0, 0, 1 }, 0xE9AA079C, { 0xD4443834 }, 0x054C9459 },
{ { 4888.854, -1701.624, 19.6798 }, { 0, 0, 0, 1 }, 0xE8039BE0, { 0xD4443834 }, 0x054C9459 },
{ { 4874.151, -1704.071, 19.6798 }, { 0, 0, 0, 1 }, 0x1F4DD994, { 0xD4443834 }, 0x054C9459 },
{ { 4860.168, -1709.171, 19.6798 }, { 0, 0, 0, 1 }, 0xDD5E5910, { 0xD4443834 }, 0x054C9459 },
{ { 4847.935, -1717.196, 19.6798 }, { 0, 0, 0, 1 }, 0x88C703C2, { 0xD4443834 }, 0x054C9459 },
{ { 4919.024, -1700.722, 19.6798 }, { 0, 0, 0, 1 }, 0x18F763F0, { 0xD4443834 }, 0x054C9459 },
{ { 4934.651, -1700.825, 19.6798 }, { 0, 0, 0, 1 }, 0xA7D3E44B, { 0xD4443834 }, 0x054C9459 },
{ { 4949.646, -1700.825, 19.6798 }, { 0, 0, 0, 1 }, 0x57B023F2, { 0xD4443834 }, 0x054C9459 },
{ { 4964.613, -1700.825, 19.6798 }, { 0, 0, 0, 1 }, 0x23D675F8, { 0xD4443834 }, 0x054C9459 },
{ { 4979.596, -1700.825, 19.6798 }, { 0, 0, 0, 1 }, 0xFD0F52CA, { 0xD4443834 }, 0x054C9459 },
{ { 4994.572, -1700.825, 19.6798 }, { 0, 0, 0, 1 }, 0x396D20EA, { 0xD4443834 }, 0x054C9459 },
{ { 4825.766, -1766.349, 19.6798 }, { 0, 0, 0, 1 }, 0xDE5E9BFC, { 0xD4443834 }, 0x054C9459 },
{ { 4824.426, -1909.455, 16.79284 }, { 0, 0, 0, 1 }, 0x0DB674C7, { 0xAAF84021 }, 0x86440782 },
{ { 4835.173, -1742.052, 19.6798 }, { 0, 0, 0, 1 }, 0x3F243F81, { 0xD4443834 }, 0x054C9459 },
{ { 4824.468, -1876.197, 18.60045 }, { 0, 0, 0, 1 }, 0x68D079FE, { 0xAAF84021 }, 0x86440782 },
{ { 4882.953, -1960.402, 45.17541 }, { 0, 0, 0, 1 }, 0x0C474C9B, { 0x258A2916 }, 0x50259B69 },
{ { 4882.321, -1797.901, 33.54697 }, { 0, 0, 0, 1 }, 0x0811F2E1, { 0x258A2916 }, 0x50259B69 },
{ { 4939.721, -1823.251, 19.43722 }, { 0, 0, 0, 1 }, 0xD312CDB3, { 0x258A2916 }, 0x50259B69 },
{ { 4964.133, -1722.394, 33.37504 }, { 0, 0, 0, 1 }, 0x24CECEBC, { 0x258A2916 }, 0x50259B69 },
{ { 4872.135, -1878.244, 57.35948 }, { 0, 0, 0, 1 }, 0xA849CE64, { 0x258A2916 }, 0x50259B69 },
{ { 4783.977, -1795.474, 45.49724 }, { 0, 0, 0, 1 }, 0x91A4CDC7, { 0x258A2916 }, 0x50259B69 },
{ { 4775.301, -1873.493, 33.98679 }, { 0, 0, 0, 1 }, 0xF01DA411, { 0x258A2916 }, 0x50259B69 },
{ { 4682.127, -1888.79, 29.63412 }, { 0, 0, 0, 1 }, 0x2DC40769, { 0x258A2916 }, 0x50259B69 },
{ { 4865.355, -1725.312, 28.22554 }, { 0, 0, 0, 1 }, 0xE3983B70, { 0x258A2916 }, 0x50259B69 },
{ { 5042.178, -1824.022, 15.20417 }, { 0, 0, 0, 1 }, 0x230720A3, { 0x258A2916 }, 0x50259B69 },
{ { 4957.83, -1786.292, 20.583 }, { 0, 0, 0, 1 }, 0x635ACD4D, { 0xD4443834 }, 0x054C9459 },
{ { 4958.708, -1812.158, 20.583 }, { 0, 0, 0, 1 }, 0xCBC04B27, { 0xD4443834 }, 0x054C9459 },
{ { 4958.501, -1799.391, 21.12157 }, { 0, 0, 0, 1 }, 0x045E4012, { 0xD4443834 }, 0x054C9459 },
{ { 5014.316, -1916.615, 36.26769 }, { 0, 0, 0, 1 }, 0x081CEFB9, { 0x258A2916 }, 0x50259B69 },
{ { 4973.979, -1874.917, 42.6398 }, { 0, 0, 0, 1 }, 0x8BC7C0F1, { 0x258A2916 }, 0x50259B69 },
{ { 4854.273, -1771.185, 20.52565 }, { 0, 0, 0, 1 }, 0x44A741F7, { 0xD4443834 }, 0x054C9459 },
{ { 4797.531, -1744.284, 20.64333 }, { 0, 0, 0, 1 }, 0x80B3DCC9, { 0xD4443834 }, 0x054C9459 },
{ { 5018.94, -1851.766, 22.08009 }, { 0, 0, 0, 1 }, 0xE0A39947, { 0xAAF84021 }, 0x86440782 },
{ { 4881.506, -1856.92, 68.34274 }, { 0, 0, 0, 1 }, 0xE1DB4C5B, { 0x28E4D75E }, 0x7F432BAA },
{ { 5100.033, -1946.707, 26.67563 }, { 0, 0, 0, 1 }, 0xB81BF459, { 0xA9322CC6 }, 0xB33AA924 },
{ { 5064.044, -1943.639, 36.55471 }, { 0, 0, 0, 1 }, 0xA91BFBE7, { 0x258A2916 }, 0x50259B69 },
{ { 5099.87, -1959.168, 141.923 }, { 0, 0, 0, 1 }, 0xF56DA27E, { 0x258A2916 }, 0x50259B69 },
{ { 5024.129, -1695.545, 19.2729 }, { 0, 0, 0, 1 }, 0x3578A135, { 0xD4443834 }, 0x054C9459 },
{ { 4874.425, -1695.543, 24.43291 }, { 0, 0, 0, 1 }, 0x358C89B5, { 0xD4443834 }, 0x054C9459 },
{ { 4846.753, -1734.175, 30.08807 }, { 0, 0, 0, 1 }, 0x15FB9D21, { 0xD4443834 }, 0x054C9459 },
{ { 4924.108, -1821.607, 35.81312 }, { 0, 0, 0, 1 }, 0x9381FD34, { 0xD4443834 }, 0x054C9459 },
{ { 4792.309, -1856.789, 29.28053 }, { 0, 0, 0, 1 }, 0x3306CA86, { 0x5F733F32 }, 0x34ADB876 },
{ { 4681.771, -1882.92, 34.74875 }, { 0, 0, 0, 1 }, 0xF9B47EA8, { 0x5F733F32 }, 0x34ADB876 },
{ { 4948.124, -1710.001, 30.62388 }, { 0, 0, 0, 1 }, 0x674D60CA, { 0xD4443834 }, 0x054C9459 },
{ { 5038.22, -1824.368, 30.73925 }, { 0, 0, 0, 1 }, 0x43EBB5C9, { 0xA210F56C }, 0x173E7F06 },
{ { 5013.323, -1916.631, 36.62052 }, { 0, 0, 0, 1 }, 0x13A7FA0C, { 0xAAF84021 }, 0x86440782 },
{ { 4909.027, -1980.753, 40.37388 }, { 0, 0, 0, 1 }, 0x1F3B61FE, { 0xAAF84021 }, 0x86440782 },
{ { 4638.039, -1914.136, 20.83106 }, { 0, 0, 0, 1 }, 0xEA65F0CE, { 0xEDFD05ED }, 0x796CAC78 },
{ { 4918.575, -1694.85, 23.82852 }, { 0, 0, -9E-08, 1 }, 0x7281B32A, { 0xD4443834 }, 0x054C9459 },
{ { 5170.938, -1971.237, 23.35401 }, { 0, 0, -9E-08, 1 }, 0xE5B8385D, { 0xA9322CC6 }, 0xB33AA924 },
{ { 5139.488, -1937.934, 38.81043 }, { 0, 0, 0, 1 }, 0x079F7454, { 0x258A2916 }, 0x50259B69 },
{ { 4985.843, -1728.806, 19.58475 }, { 0, 0, 0, 1 }, 0x88646415, { 0xD4443834 }, 0x054C9459 },
{ { 4681.205, -1847.193, 24.86503 }, { 0, 0, 0, 1 }, 0xE5B06E64, { 0x5F733F32 }, 0x34ADB876 },
{ { 4658.051, -1785.867, 18.4124 }, { 0, 0, 0, 1 }, 0x4958C498, { 0x5F733F32 }, 0x34ADB876 },
{ { 5099.935, -1939.948, 74.22976 }, { 0, 0, -0.7071066, 0.7071066 }, 0x04AA77F6, { 0xA9322CC6 }, 0x4C225CE7 },
{ { 5137.436, -1967.623, 74.22934 }, { 0, 0, 1, 4E-08 }, 0x04AA77F6, { 0xA9322CC6 }, 0x4C225CE7 },
{ { 5189.973, -2033.743, 110.6484 }, { 0, 0, 9E-08, 1 }, 0xB62DFDF2, { 0x258A2916 }, 0x50259B69 },
{ { 5080.202, -1859.968, 44.49377 }, { 0, 0, 0, 1 }, 0xE30C1182, { 0x258A2916 }, 0x50259B69 },
{ { 5160.025, -2007.868, 19.89432 }, { 0, 0, 0, 1 }, 0x4A368578, { 0xA9322CC6 }, 0xB33AA924 },
{ { 5098.001, -2033.826, 19.88158 }, { 0, 0, 0, 1 }, 0xF82ACCD7, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 5103.437, -2029.673, 28.34263 }, { 0, 0, 0, 1 }, 0x54BEC469, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 4983.498, -2039.164, 51.46058 }, { 0, 0, 0, 1 }, 0x44000179, { 0x258A2916 }, 0x50259B69 },
{ { 4912.204, -2009.579, 23.47858 }, { 0, 0, 0, 1 }, 0x9608BD73, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 4928.139, -2012.169, 37.02722 }, { 0, 0, 0, 1 }, 0x1D13E967, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 5112.911, -2030.267, 33.79222 }, { 0, 0, 0, 1 }, 0xE231ECE5, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 4897.814, -2058.714, 16.29748 }, { 0, 0, 0.7071066, 0.7071069 }, 0x6D3F1EEE, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 4894.785, -2072.161, 17.18306 }, { 0, 0, -0.7071066, 0.7071066 }, 0x128ACFBE, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 4873.991, -2059.854, 17.9007 }, { 0, 0, 0, 1 }, 0x14262815, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 5105.31, -2026.965, 20.5143 }, { 0, 0, 0, 1 }, 0xEFB73D42, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 4880.458, -2029.007, 46.21145 }, { 0, 0, 0, 1 }, 0xA82AE1AF, { 0x258A2916 }, 0x50259B69 },
{ { 5089.827, -2058.537, 46.71652 }, { 0, 0, 0, 1 }, 0xFAAF7EB3, { 0x258A2916 }, 0x50259B69 },
{ { 5169.976, -2035.474, 19.91772 }, { 0, 0, 0, 1 }, 0xE66A2E33, { 0xA9322CC6 }, 0xB33AA924 },
{ { 5170.733, -2065.278, 16.95582 }, { 0, 0, 0, 1 }, 0x4606C2FA, { 0xA9322CC6 }, 0xB33AA924 },
{ { 5205.411, -2065.311, 16.95582 }, { 0, 0, 0, 1 }, 0x8A2FB737, { 0xA9322CC6 }, 0xB33AA924 },
{ { 5210.81, -2035.491, 19.91772 }, { 0, 0, 0, 1 }, 0x29EA5866, { 0xA9322CC6 }, 0xB33AA924 },
{ { 4849.987, -2063.594, 14.95792 }, { 0, 0, 0, 1 }, 0x2CB055CD, { 0x7EE7E611 }, 0x6191F0E5 },
{ { 5099.397, -1965.19, 27.77427 }, { 0, 0, 0, 1 }, 0x78B0C614, { 0xA9322CC6 }, 0xB33AA924 },
{ { 5075.721, -1936.34, 36.7619 }, { 0, 0, 0, 1 }, 0xC148B162, { 0xA9322CC6 }, 0xB33AA924 },
{ { 4687.571, -1750.974, 25.34378 }, { 0, 0, 0, 1 }, 0xDAAEEEC1, { 0x2F7A499E }, 0x348BFEEA },
{ { 4672.604, -1999.565, 19.84685 }, { 0, 0, 0, 1 }, 0x8BD18BD6, { 0x2F7A499E }, 0x348BFEEA },
{ { 4860.341, -1695.925, 27.49707 }, { 0, 0, 0, 1 }, 0x25472C81, { 0x2F7A499E }, 0x348BFEEA },
{ { 4968.194, -1780.947, 29.56552 }, { 0, 0, 0, 1 }, 0x8E67261E, { 0x2F7A499E }, 0x348BFEEA },
{ { 5082.083, -1768.507, 28.95892 }, { 0, 0, 0, 1 }, 0x3A2CD001, { 0x2F7A499E }, 0x348BFEEA },
{ { 5186.642, -2052.076, 25.24407 }, { 0, 0, 0, 1 }, 0xC64C50A4, { 0x2F7A499E }, 0x348BFEEA },
{ { 4915.727, -1874.696, 23.26878 }, { 0, 0, 0, 1 }, 0x50809B04, { 0xAAF84021 }, 0x86440782 },
{ { 5082.631, -2052.79, 21.49936 }, { 0, 0, 0, 1 }, 0x7E0BB845, { 0x2F7A499E }, 0x348BFEEA },
{ { 4981.674, -2064.583, 23.30782 }, { 0, 0, 0, 1 }, 0x00D78033, { 0x2F7A499E }, 0x348BFEEA },
{ { 4870.868, -2053.566, 23.85868 }, { 0, 0, 0, 1 }, 0xC73B1B18, { 0x2F7A499E }, 0x348BFEEA },
{ { 5128.882, -1936.344, 36.77818 }, { 0, 0, 0, 1 }, 0x81EE0DD3, { 0xA9322CC6 }, 0x00000000 },
{ { 4981.652, -2542.54, 14.41506 }, { 0, 0, 0, 1 }, 0x1058968F, { 0x249DFD6B }, 0xA73B69FD },
{ { 4981.677, -2398.377, 18.03093 }, { 0, 0, 0, 1 }, 0x0FCD6968, { 0x807D9268 }, 0xD6D03E21 },
{ { 4981.756, -2221.835, 6.743426 }, { 0, 0, 0, 1 }, 0x833BAB44, { 0xBE94A1A0 }, 0x65B4B4DA },
{ { 4952.794, -2486.15, 5.594 }, { 0, 0, 0, 1 }, 0x4E5A1DF4, { 0x0C6F7EAF }, 0x19F81A8E },
{ { 5080.17, -2435.202, 14.30875 }, { 0, 0, 0, 1 }, 0xA96DBFC1, { 0x9F06A3DF }, 0x20492730 },
{ { 5109.203, -2434.88, 21.454 }, { 0, 0, 0, 1 }, 0x261BE320, { 0xA3DEB46A }, 0x3BD5962B },
{ { 4851.991, -2613.307, 31.04435 }, { 0, 0, 0, 1 }, 0x2CB0E179, { 0x0C6F7EAF }, 0x19F81A8E },
{ { 5109.946, -2611.99, 18.95173 }, { 0, 0, 0, 1 }, 0x585DCE9E, { 0x0C6F7EAF }, 0x19F81A8E },
{ { 5091.622, -2595.339, 19.88874 }, { 0, 0, 0, 1 }, 0x4BB062B6, { 0x0C6F7EAF }, 0x19F81A8E },
{ { 4983.096, -2590.116, 14.26328 }, { 0, 0, 0, 1 }, 0x61C93910, { 0x0C6F7EAF }, 0x19F81A8E },
{ { 4883.674, -2550.48, 14.61242 }, { 0, 0, 0, 1 }, 0x85F01EF0, { 0x0C6F7EAF }, 0x19F81A8E },
{ { 4986.851, -2377.548, 7.081706 }, { 0, 0, 0, 1 }, 0xBE80E444, { 0x9F06A3DF }, 0x20492730 },
{ { 5030.658, -2167.779, 2.907256 }, { 0, 0, 0, 1 }, 0x4B8420B1, { 0x8DD80182 }, 0x78445725 },
{ { 4975.218, -2308.174, 7.457335 }, { 0, 0, 0, 1 }, 0xFBFD0CEF, { 0x9F06A3DF }, 0x20492730 },
{ { 4977.816, -2524.549, 7.511263 }, { 0, 0, 0, 1 }, 0xA4673A52, { 0x0C6F7EAF }, 0x19F81A8E },
{ { 4932.845, -2514.841, 7.687603 }, { 0, 0, 0, 1 }, 0x9379E580, { 0x0C6F7EAF }, 0x19F81A8E },
{ { 4855.552, -2322.693, 21.8436 }, { 0, 0, 0, 1 }, 0xA131089D, { 0x9F06A3DF }, 0x20492730 },
{ { 5059.034, -2469.524, 20.7279 }, { 0, 0, 0, 1 }, 0x27338877, { 0x9F06A3DF }, 0x20492730 },
{ { 4896.455, -2159.437, 22.4018 }, { 0, 0, 0, 1 }, 0x5164983B, { 0x8F18C559 }, 0xB8B25805 },
{ { 4983.859, -2143.215, 22.04889 }, { 0, 0, 0, 1 }, 0x531BBFC8, { 0x8F18C559 }, 0xB8B25805 },
{ { 5085.981, -2146.148, 22.33893 }, { 0, 0, 0, 1 }, 0xCC49B320, { 0x8F18C559 }, 0xB8B25805 },
{ { 5089.593, -2270.988, 23.15286 }, { 0, 0, 0, 1 }, 0x8B36F7D8, { 0x8F18C559 }, 0xB8B25805 },
{ { 4982.549, -2260.028, 18.62192 }, { 0, 0, 0, 1 }, 0x8A4B1D54, { 0x8F18C559 }, 0xB8B25805 },
{ { 4887.518, -2263.628, 22.11474 }, { 0, 0, 0, 1 }, 0x20AB0706, { 0x8F18C559 }, 0xB8B25805 },
{ { 5086.093, -2383.553, 25.38806 }, { 0, 0, 0, 1 }, 0x0E1F6CEF, { 0x8F18C559 }, 0xB8B25805 },
{ { 4989.067, -2377.22, 22.24867 }, { 0, 0, 0, 1 }, 0xF979125F, { 0x8F18C559 }, 0xB8B25805 },
{ { 4888.438, -2373.647, 22.13895 }, { 0, 0, 0, 1 }, 0x10E77C0D, { 0x8F18C559 }, 0xB8B25805 },
{ { 5004.68, -2479.738, 22.46517 }, { 0, 0, 0, 1 }, 0xAFD8AE46, { 0x8F18C559 }, 0xB8B25805 },
{ { 5077.617, -2509.779, 26.43212 }, { 0, 0, 0, 1 }, 0xF5542067, { 0x8F18C559 }, 0xB8B25805 },
{ { 5077.924, -2579.397, 25.50356 }, { 0, 0, 0, 1 }, 0x73298E0F, { 0x8F18C559 }, 0xB8B25805 },
{ { 4994.845, -2579.696, 23.12712 }, { 0, 0, 0, 1 }, 0xEE04EB00, { 0x8F18C559 }, 0xB8B25805 },
{ { 4898.213, -2578.785, 25.08625 }, { 0, 0, 0, 1 }, 0x7F3CD0E7, { 0x8F18C559 }, 0xB8B25805 },
{ { 4878.246, -2476.672, 22.3798 }, { 0, 0, 0, 1 }, 0xEA8AD0DC, { 0x8F18C559 }, 0xB8B25805 },
{ { 5354.629, -3042.06, 41.35968 }, { 0, 0, 0, 1 }, 0x2F37D8D9, { 0xBB41E023 }, 0x640E0FFB },
{ { 5400.247, -3266.11, 21.54333 }, { 0, 0, 0, 1 }, 0xE066BB38, { 0xBB41E023 }, 0x640E0FFB },
{ { 5242.324, -2792.172, 117.9032 }, { 0, 0, 0, 1 }, 0x51D39E14, { 0xBB41E023 }, 0x640E0FFB },
{ { 5401.866, -2879.315, 14.22254 }, { 0, 0, 0, 1 }, 0x1D81B56D, { 0xBB41E023 }, 0x640E0FFB },
{ { 5432.102, -2929.404, 14.25757 }, { 0, 0, 9E-08, 1 }, 0xF5F6E658, { 0xBB41E023 }, 0x640E0FFB },
{ { 5260.583, -3352.128, 17.96082 }, { 0, 0, 0, 1 }, 0x0BAC11C2, { 0xBB41E023 }, 0x640E0FFB },
{ { 5400.247, -3266.11, 21.54333 }, { 0, 0, 0, 1 }, 0x40E47C32, { 0xBB41E023 }, 0x640E0FFB },
{ { 5237.482, -3007.414, 13.7619 }, { 0, 0, 0, 1 }, 0x03B001CE, { 0xBB41E023 }, 0x640E0FFB },
{ { 5239.754, -3224.929, 118.1078 }, { 0, 0, 0, 1 }, 0x1637A6DD, { 0xBB41E023 }, 0x640E0FFB },
{ { 5508.846, -2363.702, 1.680944 }, { 0, 0, 0, 1 }, 0xF4852415, { 0x1C165E27 }, 0x8FF8F9D8 },
{ { 5620.156, -3196.767, 1.200892 }, { 0, 0, 0, 1 }, 0x710EC21F, { 0x1C165E27 }, 0x8FF8F9D8 },
{ { 5184.937, -3393.533, 15.77847 }, { 0, 0, -0.7071066, 0.7071066 }, 0xCB174B65, { 0x300047CE }, 0x4F4B557D },
{ { 5173.508, -3133.602, 15.96085 }, { 0, 0, 0.7071066, 0.7071066 }, 0x78DE68FE, { 0x300047CE }, 0x4F4B557D },
{ { 5307.977, -3399.557, 16.28281 }, { 0, 0, 0.7071066, 0.7071066 }, 0xDD93A8D0, { 0x300047CE }, 0x4F4B557D },
{ { 5290.698, -3297.76, 16.61523 }, { 0, 0, 0.7071066, 0.7071066 }, 0xEE18052A, { 0x300047CE }, 0x4F4B557D },
{ { 5217.069, -3026.433, 16.61523 }, { 0, 0, 0.7071066, 0.7071066 }, 0x7E8713B8, { 0x300047CE }, 0x4F4B557D },
{ { 5288.904, -2789.757, 16.62756 }, { 0, 0, 0.7071066, 0.7071066 }, 0x139DE74C, { 0x300047CE }, 0x4F4B557D },
{ { 5271.667, -2880.436, 16.42587 }, { 0, 0, -0.7071066, 0.7071066 }, 0xF9D847AC, { 0x300047CE }, 0x4F4B557D },
{ { 5346.234, -2794.27, 22.17004 }, { 0, 0, 0, 1 }, 0xF7F47937, { 0x2F7A499E }, 0x3B6A5D32 },
{ { 5204.716, -2800.41, 22.56616 }, { 0, 0, 0, 1 }, 0xB73D27DE, { 0x2F7A499E }, 0x3B6A5D32 },
{ { 5233.636, -2954.735, 21.72056 }, { 0, 0, 0, 1 }, 0x53591AD6, { 0x2F7A499E }, 0x3B6A5D32 },
{ { 5336.798, -3071.752, 19.87017 }, { 0, 0, 0, 1 }, 0x3D61D65F, { 0x2F7A499E }, 0x3B6A5D32 },
{ { 5370.154, -2924.742, 25.7942 }, { 0, 0, 0, 1 }, 0x63CE0BA9, { 0x2F7A499E }, 0x3B6A5D32 },
{ { 5401.123, -3220.551, 17.79538 }, { 0, 0, 0, 1 }, 0x41E3E5BA, { 0x2F7A499E }, 0x3B6A5D32 },
{ { 5398.626, -3362.269, 22.55073 }, { 0, 0, 0, 1 }, 0xF0BD9B26, { 0x2F7A499E }, 0x3B6A5D32 },
{ { 5218.085, -3354.446, 22.63718 }, { 0, 0, 0, 1 }, 0x1CD9F0E9, { 0x2F7A499E }, 0x3B6A5D32 },
{ { 5210.422, -2181.694, 45.46176 }, { 0, 0, 0, 1 }, 0x9552DDF0, { 0xB20ECDBD }, 0x43E74FAA },
{ { 5210.288, -2339.744, 53.08125 }, { 0, 0, 0, 1 }, 0xF6ACA310, { 0xB20ECDBD }, 0x43E74FAA },
{ { 5222.259, -2534.81, 84.44593 }, { 0, 0, 0, 1 }, 0xD91B9DF2, { 0xB20ECDBD }, 0x43E74FAA },
{ { 5091.144, -2661.962, 112.7093 }, { 0, 0, 0, 1 }, 0xABA8F00F, { 0xB20ECDBD }, 0x43E74FAA },
{ { 5388.875, -2241.701, 11.21497 }, { 0, 0, 0, 1 }, 0xDCD32CE2, { 0xB20ECDBD }, 0x43E74FAA },
{ { 5348.327, -2529.666, 11.53719 }, { 0, 0, 0, 1 }, 0x38920E27, { 0xB20ECDBD }, 0x43E74FAA },
{ { 5259.028, -2663.229, 42.94707 }, { 0, 0, 0, 1 }, 0x4767C81F, { 0xB20ECDBD }, 0x43E74FAA },
{ { 5274.964, -2116.723, 4.853393 }, { 0, 0, 1, -4E-08 }, 0xAEE846F7, { 0xF3E06FFD }, 0x711C0E9D },
{ { 5352.575, -2559.389, 12.11037 }, { 0, 0, 0, 1 }, 0x10796C45, { 0x190F2F28 }, 0xE6413D45 },
{ { 5350.911, -2245.02, 18.7538 }, { 0, 0, 0, 1 }, 0xB38940AE, { 0xE9D96379 }, 0xBC304845 },
{ { 5244.222, -2244.36, 18.82485 }, { 0, 0, 0, 1 }, 0xADFF2082, { 0xE9D96379 }, 0xBC304845 },
{ { 5340.284, -2168.183, 18.82485 }, { 0, 0, 0, 1 }, 0x784DA6AE, { 0xE9D96379 }, 0xBC304845 },
{ { 5333.813, -2646.853, 19.33599 }, { 0, 0, 0.7071066, 0.7071066 }, 0x3759C4E2, { 0x190F2F28 }, 0xE6413D45 },
{ { 5337.155, -2607.899, 18.03606 }, { 0, 0, 0, 1 }, 0x8310E190, { 0x190F2F28 }, 0xE6413D45 },
{ { 5329.854, -2446.457, 22.06584 }, { 0, 0, 0.9716099, 0.236589 }, 0xF1A3E692, { 0x190F2F28 }, 0xE6413D45 },
{ { 5283.247, -2243.474, 28.58185 }, { 0, 0, 0.7151031, 0.699019 }, 0x66EF6BF9, { 0xE9D96379 }, 0xBC304845 },
{ { 5276.908, -2281.46, 46.1861 }, { 0, 0, 0.9063078, 0.4226183 }, 0xA1155EC6, { 0xE9D96379 }, 0xBC304845 },
{ { 5293.27, -2217.744, 32.76078 }, { 0, 0, 0, 1 }, 0x69CB704F, { 0xF3E06FFD }, 0x711C0E9D },
{ { 5356.386, -2162.93, 16.14105 }, { 0, 0, 0.88652, -0.4626903 }, 0x4A523244, { 0x04D56468 }, 0xC80425DC },
{ { 5099.787, -2673.764, 78.77474 }, { 1, -4E-08, 4E-08, 0 }, 0x44900D36, { 0x8780C63D }, 0x2BF70A1F },
{ { 5295.323, -2411.435, 13.79732 }, { 0, 0, 0, 1 }, 0x935014D7, { 0x65907D8B }, 0x1D602903 },
{ { 5332.982, -2325.879, 14.42982 }, { 0, 0, -0.7071068, 0.7071066 }, 0xB12B337D, { 0x3869B553 }, 0x73F85C76 },
{ { 5323.876, -2518.953, 14.42982 }, { 0, 0, -0.7071068, 0.7071066 }, 0xEFE0F8F2, { 0x3869B553 }, 0x73F85C76 },
{ { 5323.876, -2484.741, 14.42982 }, { 0, 0, -0.7071068, 0.7071066 }, 0xD8883294, { 0x3869B553 }, 0x73F85C76 },
{ { 5323.876, -2451.125, 14.42982 }, { 0, 0, -0.7071068, 0.7071066 }, 0x0962102F, { 0x3869B553 }, 0x73F85C76 },
{ { 5332.982, -2367.899, 14.42982 }, { 0, 0, -0.7071068, 0.7071066 }, 0xD5C53542, { 0x3869B553 }, 0x73F85C76 },
{ { 5328.429, -2413.829, 14.42982 }, { 0, 0, -0.7071068, 0.7071066 }, 0xA7D505C4, { 0x3869B553 }, 0x73F85C76 },
{ { 5076.578, -2603.426, 19.27166 }, { 0, 0, 0, 1 }, 0xCAFA429B, { 0x04D56468 }, 0xC80425DC },
{ { 5194.037, -2623.451, 47.14655 }, { 0, 0, 0, 1 }, 0xFC0DC3A4, { 0x04D56468 }, 0xC80425DC },
{ { 5320.696, -2601.808, 16.56303 }, { 0, 0, 0, 1 }, 0x21344392, { 0x04D56468 }, 0xC80425DC },
{ { 5180.403, -2478.664, 19.19751 }, { 0, 0, -0.7071068, 0.7071066 }, 0xCD5BAFCD, { 0x04D56468 }, 0xC80425DC },
{ { 5314.444, -2479.679, 15.52358 }, { 0, 0, -9E-08, 1 }, 0x36C51255, { 0x04D56468 }, 0xC80425DC },
{ { 5197.718, -2338.013, 18.97025 }, { 0, 0, 1, 8E-08 }, 0x6FFC882B, { 0x04D56468 }, 0xC80425DC },
{ { 5346.138, -2337.775, 16.33944 }, { 0, 0, 1, 8E-08 }, 0x62FE68ED, { 0x04D56468 }, 0xC80425DC },
{ { 5180.721, -2170.004, 19.88368 }, { 0, 0, 0, 1 }, 0x7776882C, { 0x04D56468 }, 0xC80425DC },
{ { 4904.521, -2838.825, 109.449 }, { 0, 0, 0, 1 }, 0x55315E29, { 0x9F5BC806 }, 0x7E4C872C },
{ { 5078.056, -2839.303, 128.9517 }, { 0, 0, 0, 1 }, 0x510DD55C, { 0x9F5BC806 }, 0x7E4C872C },
{ { 5079.62, -3108.92, 129.5309 }, { 0, 0, 0, 1 }, 0xC9AFF80D, { 0x9F5BC806 }, 0x7E4C872C },
{ { 4904.529, -3108.92, 120.2997 }, { 0, 0, 0, 1 }, 0x6BA24ED7, { 0x9F5BC806 }, 0x7E4C872C },
{ { 5079.62, -3319.661, 112.1126 }, { 0, 0, 0, 1 }, 0xEFB28B07, { 0x9F5BC806 }, 0x7E4C872C },
{ { 4908.451, -3315.972, 213.7329 }, { 0, 0, 0, 1 }, 0xA7460857, { 0x9F5BC806 }, 0x7E4C872C },
{ { 5144.579, -3165.054, 13.78815 }, { 0, 0, 0, 1 }, 0x956278A8, { 0xB8CB9D34 }, 0xF9B64DEE },
{ { 4827.573, -3258.483, 13.70232 }, { 0, 0, 0, 1 }, 0x629FED71, { 0x4D4934FD }, 0x37E7C6C1 },
{ { 4869.219, -2809.025, 22.14193 }, { 0, 0, 0, 1 }, 0xA0AF5FC2, { 0x2F7A499E }, 0x4C1C67C7 },
{ { 5077.208, -2799.315, 19.56702 }, { 0, 0, 0, 1 }, 0x2D972F73, { 0x2F7A499E }, 0x4C1C67C7 },
{ { 4846.892, -2940.778, 22.13956 }, { 0, 0, 0, 1 }, 0x1EA907BD, { 0x2F7A499E }, 0x4C1C67C7 },
{ { 4898.714, -3073.298, 22.14048 }, { 0, 0, 0, 1 }, 0xA37035C2, { 0x2F7A499E }, 0x4C1C67C7 },
{ { 5072.364, -2961.73, 22.13927 }, { 0, 0, 0, 1 }, 0xD1DAFB50, { 0x2F7A499E }, 0x4C1C67C7 },
{ { 4885.215, -3219.669, 22.0639 }, { 0, 0, 0, 1 }, 0xF6FBEF68, { 0x2F7A499E }, 0x4C1C67C7 },
{ { 5068.54, -3072.448, 22.0909 }, { 0, 0, 0, 1 }, 0x8B14897D, { 0x2F7A499E }, 0x4C1C67C7 },
{ { 5125.902, -3163.843, 22.1569 }, { 0, 0, 0, 1 }, 0xC08574DB, { 0x2F7A499E }, 0x4C1C67C7 },
{ { 5656.047, -3030.789, 35.80426 }, { 0, 0, 0, 1 }, 0xC4534DBF, { 0x5902FB85 }, 0xF5A14658 },
{ { 5637.396, -3139.611, 34.1965 }, { 0, 0, 0, 1 }, 0x17F704E1, { 0x5902FB85 }, 0xF5A14658 },
{ { 5678.355, -3318.406, 1.735687 }, { 0, 0, 0, 1 }, 0x377B0CA8, { 0x5902FB85 }, 0xF5A14658 },
{ { 5248.261, -3487.543, 46.96133 }, { 0, 0, 0, 1 }, 0xB1244FFE, { 0x5902FB85 }, 0xF5A14658 },
{ { 5461.209, -3479.501, 26.05502 }, { 0, 0, 0, 1 }, 0xCBCA2EF6, { 0x5902FB85 }, 0xF5A14658 },
{ { 5773.616, -2974.862, 2.374572 }, { 0, 0, 0, 1 }, 0x936A43DA, { 0xE66CDC04 }, 0x0981B464 },
{ { 5733.726, -3151.02, -0.05300623 }, { 0, 0, -0.05056793, 0.9987206 }, 0x23960FE4, { 0xE66CDC04 }, 0x0981B464 },
{ { 5737.569, -3105.785, 1.958065 }, { 0, 0, 0, 1 }, 0xD133BF6C, { 0xE66CDC04 }, 0x0981B464 },
{ { 5367.656, -3446.648, 56.51572 }, { 0, 0, 0, 1 }, 0x90C86DAE, { 0xC6DBF859 }, 0x2E0FF9A8 },
{ { 5387.197, -3454.634, 52.85119 }, { 0, 0, 0.2164396, 0.976296 }, 0x47A0EDE7, { 0xECD859C4 }, 0x6441DAB5 },
{ { 5718.73, -3142.767, 8.376627 }, { 0, 0, 0.7071066, 0.7071066 }, 0x96FAE31D, { 0x271C0E6E }, 0x3B205371 },
{ { 5721.678, -3129.949, 2.561617 }, { 0, 0, -0.04275633, 0.9990855 }, 0x4ACD5790, { 0x271C0E6E }, 0x3B205371 },
{ { 5734.885, -3154.771, 3.517507 }, { 0, 0, -0.7681254, 0.6402994 }, 0x2D3A3FE1, { 0x271C0E6E }, 0x3B205371 },
{ { 5712.797, -3153.848, 3.690238 }, { 0, 0, -0.02185549, 0.999761 }, 0x750A0FC8, { 0x271C0E6E }, 0x3B205371 },
{ { 5706.703, -3133.764, 3.263409 }, { 0, 0, -0.02185551, 0.999761 }, 0xA37CD6E3, { 0x271C0E6E }, 0x3B205371 },
{ { 5718.92, -3103.675, 4.038195 }, { 0, 0, -0.02185551, 0.999761 }, 0x9646305A, { 0x271C0E6E }, 0x3B205371 },
{ { 5572.145, -3284.064, 0.1199298 }, { 0, 0, -0.08945073, 0.9959913 }, 0x43BB052A, { 0xF296772D }, 0x8FA895F2 },
{ { 5547.269, -3178.457, 0.4667543 }, { 0, 0, -0.06323636, 0.9979985 }, 0x477EE444, { 0xF296772D }, 0x8FA895F2 },
{ { 5540.94, -3100.773, 0.379234 }, { 0, 0, -0.2143092, 0.9767659 }, 0x534A53D3, { 0xF296772D }, 0x8FA895F2 },
{ { 5655.031, -2911.2, 20.79774 }, { 0, 0, 0, 1 }, 0xD101555B, { 0xD902C826 }, 0x242E9EFC },
{ { 5596.664, -3047.793, 15.60729 }, { 0, 0, 0, 1 }, 0x265EBE5B, { 0xD902C826 }, 0x242E9EFC },
{ { 5633.785, -3206.374, 15.3705 }, { 0, 0, 0, 1 }, 0x8881D93E, { 0xD902C826 }, 0x242E9EFC },
{ { 5257.15, -3485.258, 22.14492 }, { 0, 0, 0, 1 }, 0x85DBFF45, { 0xD902C826 }, 0x242E9EFC },
{ { 5685.821, -3304.028, 18.10659 }, { 0, 0, -0.3826833, 0.9238795 }, 0x1D080AE5, { 0xD902C826 }, 0x242E9EFC },
{ { 4505.753, -3254.906, 22.56097 }, { 0, 0, 0, 1 }, 0xBE699BCF, { 0x435DD03B }, 0x03D7E2C5 },
{ { 4521.427, -2754.321, 7.250229 }, { 0, 0, 0, 1 }, 0x7CF79741, { 0x435DD03B }, 0x03D7E2C5 },
{ { 4525.785, -3069.885, 7.632561 }, { 0, 0, 0, 1 }, 0x4A173453, { 0x435DD03B }, 0x03D7E2C5 },
{ { 4525.348, -2888.025, 10.56189 }, { 0, 0, 0, 1 }, 0xAB799A5A, { 0x435DD03B }, 0x03D7E2C5 },
{ { 4517.671, -2762.136, 17.25812 }, { 0, 0, 0, 1 }, 0x6F411726, { 0x445508B2 }, 0xC51BD222 },
{ { 4588.293, -2762.136, 17.25812 }, { 0, 0, 0, 1 }, 0xA5A8BDB9, { 0x445508B2 }, 0xC51BD222 },
{ { 4600.43, -2807.301, 17.25812 }, { 0, 0, 0, 1 }, 0x3CA69D95, { 0x445508B2 }, 0xC51BD222 },
{ { 4530.465, -2807.301, 17.25812 }, { 0, 0, 0, 1 }, 0x3D43E924, { 0x445508B2 }, 0xC51BD222 },
{ { 4453.722, -2807.301, 17.2581 }, { 0, 0, 0, 1 }, 0xA4CAE8EA, { 0x445508B2 }, 0xC51BD222 },
{ { 4634.577, -3286.163, 11.98178 }, { 0, 0, 0, 1 }, 0x22AFAD8D, { 0x65C53281 }, 0x3CE7462B },
{ { 4634.577, -3250.79, 11.98178 }, { 0, 0, 0, 1 }, 0xE44AC640, { 0x65C53281 }, 0x3CE7462B },
{ { 4634.577, -3242.79, 11.98178 }, { 0, 0, 0, 1 }, 0x7EFE8BE6, { 0x65C53281 }, 0x3CE7462B },
{ { 4634.577, -3198.79, 11.98178 }, { 0, 0, 0, 1 }, 0xE6627C76, { 0x65C53281 }, 0x3CE7462B },
{ { 4634.577, -3168.79, 11.98178 }, { 0, 0, 0, 1 }, 0x8C46E2E1, { 0x65C53281 }, 0x3CE7462B },
{ { 4634.577, -3124.79, 11.98178 }, { 0, 0, 0, 1 }, 0x1952857F, { 0xD3A1F959 }, 0x171E4452 },
{ { 4556.289, -3091.254, 12.04182 }, { 0, 0, 0, 1 }, 0xAAD5753E, { 0x65C53281 }, 0x3CE7462B },
{ { 4420.55, -3101.215, -2.040787 }, { 9E-08, 0, -9E-08, 1 }, 0x2E783030, { 0xD3A1F959 }, 0x171E4452 },
{ { 4485.795, -3101.251, -2.040787 }, { 0, 0, -9E-08, 1 }, 0xD123858C, { 0xD3A1F959 }, 0x171E4452 },
{ { 4524.441, -3142.183, -2.30867 }, { 0, 0, 1.5E-07, 1 }, 0x82CF6AFA, { 0xD3A1F959 }, 0x171E4452 },
{ { 4473.508, -2874.134, 6.839889 }, { 0, 0, 0.0130896, 0.9999142 }, 0xDBC2385B, { 0x40050FEA }, 0x9C37BAF5 },
{ { 4550.404, -2765.361, 14.17255 }, { 0, 0, 0, 1 }, 0x1C911A86, { 0x445508B2 }, 0xC51BD222 },
{ { 4641.969, -3240.967, 3.661963 }, { 0, 0, 0, 1 }, 0xDD564D38, { 0x65C53281 }, 0x3CE7462B },
{ { 4551.672, -3076.799, 3.811964 }, { 0, 0, 0, 1 }, 0x1BB306DE, { 0x65C53281 }, 0x3CE7462B },
{ { 4561.898, -2705.488, 10.19015 }, { -6E-08, 6E-08, -0.7071066, 0.7071069 }, 0xAF0BEC0E, { 0x445508B2 }, 0xC51BD222 },
{ { 4615.998, -3051.302, 7.107871 }, { 0, 0, 0.7071068, 0.7071066 }, 0x1499849B, { 0x65C53281 }, 0x3CE7462B },
{ { 4604.449, -2778.531, 15.62965 }, { 0, 0, 0, 1 }, 0x062814D4, { 0x445508B2 }, 0xC51BD222 },
{ { 4610.426, -2859.628, 4.368914 }, { 0, 0, 0, 1 }, 0x313B6B5E, { 0x5CB1329C }, 0x13C822A2 },
{ { 4420.594, -2981.304, 0.8146515 }, { 0, 0, 0, 1 }, 0xC0E5ECBA, { 0x65C53281 }, 0x3CE7462B },
{ { 4598.007, -2747.117, 9.486443 }, { 0, 0, 0, 1 }, 0xCE7E6CD2, { 0x40050FEA }, 0x9C37BAF5 },
{ { 4492.922, -2981.061, 0.7367554 }, { 0, 0, -1E-08, 1 }, 0xD2024153, { 0x65C53281 }, 0x3CE7462B },
{ { 4467.9, -3100.898, 9.557729 }, { 0, 0, 1, -6.4E-07 }, 0x899A2F7F, { 0x65C53281 }, 0x3CE7462B },
{ { 4419.933, -3100.883, 9.558309 }, { 0, 0, 0.3826839, 0.9238793 }, 0xE2D21F60, { 0x65C53281 }, 0x3CE7462B },
{ { 4524.897, -3101.658, 9.783547 }, { 0, 0, 0.4617481, 0.8870111 }, 0x9F4D3131, { 0x65C53281 }, 0x3CE7462B },
{ { 4479.087, -2898.616, 8.795045 }, { 0, 0, 0.2588191, 0.9659258 }, 0xC5972CD6, { 0x5CB1329C }, 0x13C822A2 },
{ { 4547.616, -2865.498, 3.367834 }, { 0, 0, 0.7050367, 0.7091709 }, 0x686FA6D4, { 0x40050FEA }, 0x9C37BAF5 },
{ { 4448.814, -2916.402, 4.657532 }, { 0, 0, 0, 1 }, 0xBADA9AF6, { 0x40050FEA }, 0x9C37BAF5 },
{ { 4561.938, -2665.37, 16.02232 }, { 0, 0, 9E-08, 1 }, 0x8FDFB9F9, { 0x40050FEA }, 0x9C37BAF5 },
{ { 4415.624, -3183.316, 12.19383 }, { 0, 0, -0.5753623, 0.8178988 }, 0xE2228E12, { 0x65C53281 }, 0x3CE7462B },
{ { 4477.754, -3269.647, 14.1869 }, { 0, 0, 0.8870108, 0.4617488 }, 0xEDBFC2D6, { 0x65C53281 }, 0x3CE7462B },
{ { 4387.063, -3269.5, 14.17305 }, { 0, 0, 0.4617486, 0.8870108 }, 0xFFF7AD69, { 0x65C53281 }, 0x3CE7462B },
{ { 4525.854, -2667.832, 0.8535385 }, { 2.4E-07, 0, -9E-08, 1 }, 0x8CACDAE8, { 0x445508B2 }, 0xC51BD222 },
{ { 4525.426, -2727.642, 1.306488 }, { 2.4E-07, 0, -9E-08, 1 }, 0x962ACF18, { 0x445508B2 }, 0xC51BD222 },
{ { 4527.901, -2899.074, 2.379731 }, { 0, 0, 0, 1 }, 0x6E59DC5B, { 0x5CB1329C }, 0x13C822A2 },
{ { 4555.506, -2976.656, 22.35767 }, { 0, 0, 0, 1 }, 0xA84C4751, { 0x65C53281 }, 0x3CE7462B },
{ { 4587.616, -2890.704, 16.26304 }, { 0, 0, 0.209371, 0.9778364 }, 0x532510B7, { 0x40050FEA }, 0x9C37BAF5 },
{ { 4584.688, -2946.208, 15.8401 }, { 0, 0, 0, 1 }, 0xD57E549F, { 0x40050FEA }, 0x9C37BAF5 },
{ { 4592.522, -2703.725, 9.489856 }, { 0, 0, -0.7115121, 0.7026739 }, 0xB8BA8BAA, { 0x40050FEA }, 0x9C37BAF5 },
{ { 4499.962, -3290.569, 28.97883 }, { 0, 0, 0, 1 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4454.427, -3290.569, 28.97883 }, { 0, 0, 0, 1 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4408.927, -3290.569, 28.97883 }, { 0, 0, 0, 1 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4363.462, -3290.569, 28.97883 }, { 0, 0, 0, 1 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4363.468, -3248.827, 28.97883 }, { 0, 0, 1, 8E-08 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4408.968, -3248.827, 28.97883 }, { 0, 0, 1, 8E-08 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4454.468, -3248.827, 28.97883 }, { 0, 0, 1, 8E-08 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4499.968, -3248.827, 28.97883 }, { 0, 0, 1, 8E-08 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4545.468, -3248.827, 28.97883 }, { 0, 0, 1, 8E-08 }, 0xDB8D75D7, { 0x40E38687 }, 0x44AAD076 },
{ { 4432.36, -3289.496, 29.61837 }, { 0, 0, 0, 1 }, 0xB99091DC, { 0x40E38687 }, 0x44AAD076 },
{ { 4386.211, -3289.496, 29.61883 }, { 0, 0, 0, 1 }, 0xB99091DC, { 0x40E38687 }, 0x44AAD076 },
{ { 4386.219, -3249.9, 29.61866 }, { 0, 0, 1, 4E-08 }, 0xB99091DC, { 0x40E38687 }, 0x44AAD076 },
{ { 4431.719, -3249.901, 29.61864 }, { 0, 0, 1, 4E-08 }, 0xB99091DC, { 0x40E38687 }, 0x44AAD076 },
{ { 4477.219, -3249.901, 29.61817 }, { 0, 0, 1, 4E-08 }, 0xB99091DC, { 0x40E38687 }, 0x44AAD076 },
{ { 4584.938, -3299.045, 15.96182 }, { 0, 0, 0, 1 }, 0xCE39F8E2, { 0x65C53281 }, 0x3CE7462B },
{ { 4490.242, -3181.7, 20.11877 }, { 0, 0, 0, 1 }, 0x94D20A21, { 0x65C53281 }, 0x3CE7462B },
{ { 4526.507, -3036.917, 1.27842 }, { 2.4E-07, 0, -9E-08, 1 }, 0x0EBBBAC5, { 0x65C53281 }, 0x3CE7462B },
{ { 4482.607, -3073.537, 1.841108 }, { 0, 0, -9E-08, 1 }, 0x00961E7A, { 0x65C53281 }, 0x3CE7462B },
{ { 4582.748, -3233.787, 16.5244 }, { 0, 0, 0, 1 }, 0xE6DA22E4, { 0x65C53281 }, 0x3CE7462B },
{ { 4597.173, -3156.609, 23.65102 }, { 0, 0, 0, 1 }, 0x154A2DB7, { 0x65C53281 }, 0x3CE7462B },
{ { 4559.607, -3165.286, 16.9596 }, { 0, 0, 0, 1 }, 0x9A05B63E, { 0x65C53281 }, 0x3CE7462B },
{ { 4616.928, -3170.765, 4.265026 }, { 0, 0, 0, 1 }, 0x0267D648, { 0x40E38687 }, 0x8DF617F5 },
{ { 4643.145, -3272.101, 13.76516 }, { 0, 0, 1, 8E-08 }, 0xAF829471, { 0xD72D60EF }, 0x97D3A71B },
{ { 4642.701, -3207.767, 13.77179 }, { 0, 0, 0, 1 }, 0xD1C8CE7C, { 0xD72D60EF }, 0x97D3A71B },
{ { 4643.979, -3144.366, 13.7848 }, { 0, 0, 0, 1 }, 0x5951BF1C, { 0xD72D60EF }, 0x97D3A71B },
{ { 4642.65, -3077.504, 13.74962 }, { 0, 0, 1, 8E-08 }, 0x3B5E23C7, { 0xD72D60EF }, 0x97D3A71B },
{ { 4615.276, -3002.666, 13.44386 }, { 0, 0, -0.7071068, 0.7071066 }, 0x186DE86E, { 0xD72D60EF }, 0x97D3A71B },
{ { 4618.363, -2932.093, 12.15628 }, { 0, 0, 0, 1 }, 0x2618DFF2, { 0xD72D60EF }, 0x97D3A71B },
{ { 5288.822, -1553.915, 8.059567 }, { 9E-08, 0, 9E-08, 1 }, 0xC13ED321, { 0xDC9D82A8 }, 0x663CA765 },
{ { 5398.257, -1595.306, 15.34142 }, { 0, 0, -0.07845911, 0.9969174 }, 0xBE2A0113, { 0xDC9D82A8 }, 0x663CA765 },
{ { 5189.428, -1816.722, 12.78017 }, { 0, 0, 0, 1 }, 0x80D9C6B9, { 0xDC9D82A8 }, 0x663CA765 },
{ { 5034.511, -1488.659, 24.849 }, { 0, 0, 1, -1E-08 }, 0x1C32A621, { 0xDC9D82A8 }, 0x663CA765 },
{ { 4795.055, -1562.831, 35.23131 }, { 0, 0, 0, 1 }, 0x77890691, { 0xDC9D82A8 }, 0x663CA765 },
{ { 4610.312, -1788.776, 7.072754 }, { 0, 0, 0, 1 }, 0x0789ACCA, { 0xDC9D82A8 }, 0x663CA765 },
{ { 5299.417, -2027.033, 14.25545 }, { 0, 0, 0, 1 }, 0x81C55888, { 0xDC9D82A8 }, 0x663CA765 },
{ { 4563.937, -2001.628, 8.208634 }, { 0, 0, 0, 1 }, 0x92415DD1, { 0xEF87020C }, 0x1E96954F },
{ { 4945.089, -1661.965, 19.42831 }, { 0, 0, -0.7071066, 0.7071066 }, 0x5A6E4D37, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 4632.168, -1603.724, 0.3090324 }, { 0, 0, -0.08547774, 0.99634 }, 0xF18F60D4, { 0xCB09A814 }, 0xDA29AAF7 },
{ { 4564.278, -1927.025, 2.0494 }, { 0, 0, -0.1423725, 0.9898132 }, 0x4BAE6CB7, { 0xCB09A814 }, 0xDA29AAF7 },
{ { 4993.269, -1484.875, 3.087982 }, { -9.999997E-09, -9.999997E-09, -0.7071066, 0.7071066 }, 0x36C69ACC, { 0xCB09A814 }, 0xDA29AAF7 },
{ { 4893.21, -1495.28, 0.3508282 }, { -9.999997E-09, -9.999997E-09, -0.7071066, 0.7071066 }, 0x8249D6CE, { 0xCB09A814 }, 0xDA29AAF7 },
{ { 5123.879, -1510.181, 2.998215 }, { 0, 0, 0.9922957, -0.1238921 }, 0xAE145150, { 0xCB09A814 }, 0xDA29AAF7 },
{ { 4993.468, -1673.134, 25.06182 }, { 0, 0, 0.2487603, 0.968565 }, 0xD033A846, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 5093.672, -1657.036, 29.67645 }, { 0, 0, 0.2487603, 0.968565 }, 0x19979770, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 5157.308, -1626.208, 29.67645 }, { 0, 0, 0.2487603, 0.968565 }, 0xD4F8C901, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 5226.864, -1587.956, 29.67645 }, { 0, 0, 0.2487603, 0.968565 }, 0xE51E7B9B, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 5286.625, -1555.064, 29.67645 }, { 0, 0, 0.2487603, 0.968565 }, 0x2BAD23C4, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 5347.261, -1521.754, 29.67645 }, { 0, 0, 0.2487603, 0.968565 }, 0x31670DEC, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 5392.8, -1496.777, 29.67645 }, { 0, 0, 0.2487603, 0.968565 }, 0x42391301, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 5438.762, -1479.845, 26.53351 }, { 0, 0, 0.2487603, 0.968565 }, 0xC2F6873D, { 0xA7A81F15 }, 0xD8CBA306 },
{ { 4625.137, -1876.105, 23.06061 }, { 0, 0, -0.3826833, 0.9238795 }, 0xE2C1CF5C, { 0xC34BABD0 }, 0xA05F091F },
{ { 4733.05, -1591.207, 23.61535 }, { 0, 0, 0, 1 }, 0x2F5F6B5E, { 0xC34BABD0 }, 0xA05F091F },
{ { 4912.23, -1563.64, 22.41661 }, { 0, 0, 0, 1 }, 0xF4A82BA5, { 0xC34BABD0 }, 0xA05F091F },
{ { 5111.878, -1605.199, 21.6929 }, { 0, 0, 0, 1 }, 0x06F6D405, { 0xC34BABD0 }, 0xA05F091F },
{ { 5210.458, -1815.366, 22.49324 }, { 0, 0, -0.3826833, 0.9238795 }, 0xF3E77DC3, { 0xC34BABD0 }, 0xA05F091F },
{ { 5302.759, -2026.973, 19.49409 }, { 0, 0, 0.9238796, -0.3826834 }, 0x62B39E6D, { 0xC34BABD0 }, 0xA05F091F },
{ { 4700.782, -1922.561, 16.9693 }, { 0, 0, 1, 7.54979E-08 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4700.848, -1904.161, 16.96881 }, { 0, 0, 1, 7.54979E-08 }, 0xA34097CB, { 0x5C06F7BE }, 0x00000000 },
{ { 5098.675, -1829.264, 19.91278 }, { 0, 0, 1, 7.54979E-08 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 5116.842, -1844.574, 19.91278 }, { 0, 0, 1, 7.54979E-08 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4852.324, -1829.248, 11.9704 }, { 0, 0, 1, 7.54979E-08 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 4823.655, -1823.442, 11.70715 }, { 0, 0, 1, -1.192488E-08 }, 0x26D69F35, { 0x5C06F7BE }, 0x00000000 },
{ { 5119.651, -3289.809, 14.31017 }, { 0, 0, 0.7071067, 0.7071066 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 5152.519, -3278.345, 14.26019 }, { 0, 0, -0.7071067, 0.7071066 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4811.397, -3622.956, 4.396118 }, { 0, 0, 0.2588189, 0.9659259 }, 0xFDA9F5C0, { 0x5C06F7BE }, 0x00000000 },
{ { 4842.05, -3594.822, 4.370911 }, { 0, 0, -0.3826834, 0.9238796 }, 0xFDA9F5C0, { 0x5C06F7BE }, 0x00000000 },
{ { 5043.112, -3732.721, 14.17542 }, { 0, 0, -0.6087615, 0.7933533 }, 0xFDA9F5C0, { 0x5C06F7BE }, 0x00000000 },
{ { 5072.648, -3735.637, 14.24426 }, { 0, 0, 5.960464E-08, 1 }, 0xFDA9F5C0, { 0x5C06F7BE }, 0x00000000 },
{ { 4647.089, -2330.006, 9.425904 }, { 0, 0, 0, 1 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 4837.85, -2576.459, 14.18439 }, { 0, 0, 0.7071067, 0.7071066 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 4647.154, -2315.321, 9.438904 }, { 0, 0, 0, 1 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4796.866, -2549.362, 14.19083 }, { -5.421011E-20, -3.72529E-09, 1.455191E-11, 1 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4811.07, -2576.461, 14.16534 }, { 0, 0, 0.7071381, 0.7070754 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 5287.531, -2733.044, 12.7229 }, { 0, 0, -1.192093E-07, 1 }, 0xFDA9F5C0, { 0x5C06F7BE }, 0x00000000 },
{ { 4862.05, -3626.786, 4.383057 }, { 0, 0, 0.8660253, 0.4999999 }, 0x26D69F35, { 0x5C06F7BE }, 0x00000000 },
{ { 5057.072, -2102.341, 14.26483 }, { 0, 0, 0, 1 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 5319.349, -2730.165, 14.10498 }, { 0, 0, 0.7071067, 0.7071066 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 4641.987, -3246.375, 4.194763 }, { 0, 0, 0, 1 }, 0xFDA9F5C0, { 0x5C06F7BE }, 0x00000000 },
{ { 4932.112, -3427.849, 13.93658 }, { 0, 0, 1, -4.371139E-08 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4887.893, -3400.603, 14.0628 }, { 0, 0, 0, 1 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4930.287, -3400.594, 13.94257 }, { 0, 0, 1, -4.371139E-08 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 5300.657, -3574.499, 14.26019 }, { 0, 0, 1, -4.371139E-08 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 5300.637, -3559.208, 14.26019 }, { 0, 0, 1, -4.371139E-08 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 5259.871, -3981.458, 4.455566 }, { 0, 0, 0.258819, 0.9659259 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 5077.081, -3757.361, 14.24268 }, { 0, 0, -8.742277E-08, 1 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 4680.062, -2729.562, 6.162354 }, { 0, 0, -0.7070755, 0.7071381 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4726.823, -2729.562, 9.349854 }, { 0, 0, -0.7071067, 0.7071066 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 4810.803, -2888.271, 14.25128 }, { 0, 0, -0.7071067, 0.7071066 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 4838.117, -2888.257, 14.25391 }, { 0, 0, -0.7070755, 0.7071381 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4694.674, -3239.728, 7.348389 }, { 0, 0, -0.7070755, 0.7071381 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 4660.769, -3239.974, 7.348389 }, { 0, 0, -0.7071067, 0.7071066 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 5323.052, -2340.569, 14.20917 }, { 0, 0, 0.7071066, 0.7071067 }, 0xFDA9F5C0, { 0x5C06F7BE }, 0x00000000 },
{ { 5289.889, -2330.359, 14.18695 }, { 0, 0, 0, 1 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 5152.518, -2618.318, 14.15503 }, { 0, 0, 0.7071067, 0.7071066 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 5304.045, -2342.091, 14.20923 }, { 0, 0, 0.7071067, 0.7071066 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 5274.827, -4011.81, 4.45697 }, { 0, 0, 0.2588188, 0.9659259 }, 0x3051A740, { 0x5C06F7BE }, 0x00000000 },
{ { 4783.984, -1829.248, 11.74652 }, { 0, 0, 0, 1 }, 0xC54CBC26, { 0x5C06F7BE }, 0x00000000 },
{ { 5223.876, -3356.368, 15.38306 }, { 0, 0, 0.7071071, 0.7071064 }, 0x234F3A6A, { 0x3ADB3357 }, 0x00000000 },
{ { 6601.088, -1547.09, 8.206667 }, { 0, 0, 1, 4E-08 }, 0x6C89FF08, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6424.917, -1407.653, 16.34769 }, { 0, 0, -0.7933533, 0.6087614 }, 0x47DD35AF, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6574.009, -1314.687, 0.2887344 }, { 0, 0, -0.7933533, 0.6087614 }, 0x43A82D15, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6317.403, -1419.187, 56.27024 }, { 0, 0, 0.7962995, 0.6049027 }, 0x356A1099, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6449.461, -1236.102, 1.229644 }, { 0, 0, -0.3167863, 0.948497 }, 0x6723F40C, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6683.421, -1373.11, -6.413978 }, { 0, 0, -0.6942797, 0.7197054 }, 0x5AC0DB46, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6250.129, -1455.77, 53.74854 }, { 0, 0, 0, 1 }, 0xDBB25D5F, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6612.729, -1761.084, -0.7159381 }, { 0, 0, 9E-08, 1 }, 0x09F038BE, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6173.271, -1251.95, 16.40574 }, { 0, 0, 0, 1 }, 0x46FFB2A4, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6346.444, -1248.857, 3.069048 }, { 0, 0, -0.3167863, 0.948497 }, 0x666671A9, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6522.573, -1536.033, 25.25561 }, { 0, 0, -0.6049024, 0.7962995 }, 0x393C186D, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6411.827, -1593.965, 20.904 }, { 0, 0, 0, 1 }, 0x581ED51A, { 0x71BAD4B1 }, 0xE368C233 },
{ { 6337.167, -1536.338, 23.27823 }, { 0, 0, 0, 1 }, 0x5ED6CC11, { 0xC7A2F041 }, 0xFB6DD6EE },
{ { 6423.701, -1513.97, 22.01969 }, { 0, 0, 0, 1 }, 0x4691CCD1, { 0xC7A2F041 }, 0xFB6DD6EE },
{ { 6471.308, -1471.357, 21.84821 }, { 0, 0, 0, 1 }, 0xF787BF1A, { 0xC7A2F041 }, 0xFB6DD6EE },
{ { 6453.337, -1389.586, 23.29469 }, { 0, 0, 0, 1 }, 0x4203B0AA, { 0xC7A2F041 }, 0xFB6DD6EE },
{ { 6352.685, -1363.519, 27.74363 }, { 0, 0, 0, 1 }, 0x68DBF6A5, { 0xC7A2F041 }, 0xFB6DD6EE },
{ { 6217.391, -1362.311, 35.67406 }, { 0, 0, 0, 1 }, 0xDE3088F3, { 0xC7A2F041 }, 0xFB6DD6EE },
{ { 6402.677, -1569.572, 20.0177 }, { 0, 0, 0, 1 }, 0x4C3F5D26, { 0x57ED8C04 }, 0xCCC8D52C },
{ { 6492.349, -1376.464, 21.11443 }, { 0, 0, -0.7685134, 0.6398337 }, 0xCF98D7E4, { 0x0618DEFF }, 0x4015385F },
{ { 6337.27, -1333.065, 12.5445 }, { 0, 0, -0.6891729, 0.7245969 }, 0x61E10125, { 0x0618DEFF }, 0x4015385F },
{ { 6492.344, -1376.816, 6.181217 }, { 0, 0, -0.7685136, 0.6398336 }, 0x92A6E2B0, { 0x0618DEFF }, 0x4015385F },
{ { 6569.831, -1419.401, 12.65982 }, { 0, 0, 0.9245006, -0.3811806 }, 0x5A14F18D, { 0x64C41C4C }, 0x5252DCDA },
{ { 6603.288, -1410.755, 15.44049 }, { 0, 0, 0.6184976, 0.7857867 }, 0x4C065570, { 0x64C41C4C }, 0x5252DCDA },
{ { 6607.41, -1501.333, 15.47929 }, { 0, 0, -0.5588807, 0.8292481 }, 0x9302BFEF, { 0x22219710 }, 0xBCC4B1C8 },
{ { 6652.721, -1492.924, 14.8287 }, { 0, 0, 0.1532453, 0.9881881 }, 0x0E3B578D, { 0x22219710 }, 0xBCC4B1C8 },
{ { 6571.805, -1700.182, 1.650407 }, { 0, 0, 0.2834524, 0.9589863 }, 0x2F539C0B, { 0x862D6883 }, 0x50D3DD44 },
{ { 6571.545, -1700.42, 16.58362 }, { 0, 0, 0.2834524, 0.9589863 }, 0x9807C649, { 0x862D6883 }, 0x50D3DD44 },
{ { 6687.993, -1523.092, 19.22934 }, { 0, 0, 0.03089532, 0.9995226 }, 0x455F769D, { 0x3249D152 }, 0x5A849326 },
{ { 6602.732, -1688.952, 9.149139 }, { 0, 0, 0.00156757, 0.9999989 }, 0x3320D220, { 0x3249D152 }, 0x5A849326 },
{ { 6561.761, -1658.089, 21.49005 }, { 0, 0, -0.7220249, 0.6918672 }, 0xE2DBB197, { 0x3249D152 }, 0x5A849326 },
{ { 6645.55, -1481.773, 21.72667 }, { 0, 0, 0.9211337, 0.3892463 }, 0x43B97339, { 0x3249D152 }, 0x5A849326 },
{ { 6521.152, -1429.22, 14.39266 }, { 0.00447699, -0.00078446, 0.9849835, 0.1725887 }, 0x31404E47, { 0x3249D152 }, 0x5A849326 },
{ { 6565.586, -1480.676, 14.02158 }, { 0.00173934, -0.00419922, 0.3826735, 0.9238725 }, 0x58871B98, { 0x3249D152 }, 0x5A849326 },
{ { 6340.326, -1555.685, 28.16503 }, { 0, 0, 0.3826829, 0.9238797 }, 0x235EDED9, { 0x5CBD458F }, 0x398F2403 },
{ { 6523.022, -1605.43, 35.24837 }, { 0, 0, 0, 1 }, 0x7F3AFE92, { 0x5CBD458F }, 0x398F2403 },
{ { 6428.505, -1430.308, 36.19338 }, { 0, 0, 0, 1 }, 0x0E4E2115, { 0x5CBD458F }, 0x398F2403 },
{ { 6459.042, -1408.929, 36.22804 }, { 0, 0, 1, 8E-08 }, 0x64ECCE51, { 0x5CBD458F }, 0x398F2403 },
{ { 6371.522, -1564.746, 23.76688 }, { 0, 0, 0.3826833, 0.9238795 }, 0xA2329183, { 0x5CBD458F }, 0x398F2403 },
{ { 6444.361, -1555.8, 36.72725 }, { 0, 0, 0, 1 }, 0xB4253568, { 0x5CBD458F }, 0x398F2403 },
{ { 6492.976, -1461.366, 35.87506 }, { -4E-08, 4E-08, -0.7071068, 0.7071066 }, 0x3D477FB7, { 0x5CBD458F }, 0x398F2403 },
{ { 6548.376, -1698.055, 5.156329 }, { 0, 0, 1, -8E-08 }, 0x306DC109, { 0x862D6883 }, 0x50D3DD44 },
{ { 6582.031, -1679.786, 7.191162 }, { 0, 0, 0.05294503, 0.9985975 }, 0x4ADFC826, { 0x862D6883 }, 0x50D3DD44 },
{ { 6633.111, -1657.703, 8.181146 }, { 0, 0, 0.2002601, 0.9797426 }, 0x2DE20E9C, { 0x862D6883 }, 0x50D3DD44 },
{ { 6463.827, -1313.417, 10.06772 }, { 0, 0, 0.8901224, -0.4557219 }, 0x12576213, { 0x0618DEFF }, 0x4015385F },
{ { 6403.279, -1315.117, 10.0912 }, { 0, 0, -0.6696929, 0.7426382 }, 0x051CC79E, { 0x0618DEFF }, 0x4015385F },
{ { 6363.064, -1566.544, 19.80549 }, { 0, 0, 0, 1 }, 0xEF14CADD, { 0xC7A2F041 }, 0xFB6DD6EE },
{ { 6166.689, -1370.167, 24.98956 }, { 0, 0, 0, 1 }, 0xE7881528, { 0x7F0A5465 }, 0xA9332062 },
{ { 6292.097, -1518.955, 23.50877 }, { 0, 0, 0, 1 }, 0x66D21950, { 0x677F21C2 }, 0x9CA07174 },
{ { 6393.567, -1632.179, 20.73195 }, { 0, 0, 0, 1 }, 0xB72F067D, { 0x57ED8C04 }, 0xCCC8D52C },
{ { 6477.898, -1443.606, 14.26553 }, { 0, 0, 0.7046281, 0.7095768 }, 0xFDCBE830, { 0xC7A2F041 }, 0xFB6DD6EE },
{ { 6646.59, -1498.782, 15.82399 }, { 0, 0, 0.7071066, 0.7071066 }, 0x2B5B9389, { 0x22219710 }, 0xBCC4B1C8 },
{ { 6627.171, -1499.073, 15.82399 }, { 0, 0, 0.7933533, 0.6087614 }, 0x5E95F9FD, { 0x22219710 }, 0xBCC4B1C8 },
{ { 6075.825, -1340.792, 38.5456 }, { 0, 0, 0.7071066, 0.7071066 }, 0x5ACD2024, { 0x3249D152 }, 0x5A849326 },
{ { 6278.57, -1476.606, 11.69728 }, { 0, 0, 0.9696115, 0.2446497 }, 0xF03F3BBE, { 0x7F0A5465 }, 0xA9332062 },
{ { 6363.043, -1568.196, 21.38111 }, { 0, 0, 0, 1 }, 0x98C5DBCD, { 0x8E06782F }, 0xE934109F },
{ { 6551.348, -1424.578, 17.67929 }, { 0, 0, 0, 1 }, 0x1217EAD8, { 0x2F7A499E }, 0x7B377173 },
{ { 6662.301, -1451.271, 17.98067 }, { 0, 0, 0, 1 }, 0xDA81FBAD, { 0x2F7A499E }, 0x7B377173 },
{ { 6412.271, -1277.232, 21.05704 }, { 0, 0, 0, 1 }, 0xCC435F30, { 0x2F7A499E }, 0x7B377173 },
{ { 6362.884, -1430.507, 21.63224 }, { 0, 0, 0, 1 }, 0x6CECA084, { 0x2F7A499E }, 0x7B377173 },
{ { 6266.29, -1454.219, 26.10527 }, { 0, 0, 0, 1 }, 0x5EC70439, { 0x2F7A499E }, 0x7B377173 },
{ { 6266.023, -1272.982, 25.15378 }, { 0, 0, 0, 1 }, 0x26BC1424, { 0x2F7A499E }, 0x7B377173 },
{ { 6113.926, -1280.49, 33.8151 }, { 0, 0, 0, 1 }, 0x817DC9A6, { 0x2F7A499E }, 0x7B377173 },
{ { 6400.661, -1597.116, 23.93701 }, { 0, 0, 0, 1 }, 0x4B49DD3F, { 0x2F7A499E }, 0x7B377173 },
{ { 6570.26, -1552.908, 24.7492 }, { 0, 0, 0, 1 }, 0xD972F993, { 0x2F7A499E }, 0x7B377173 },
{ { 6355.902, -1752.375, 27.43817 }, { 0, 0, 0, 1 }, 0x46F18B67, { 0x2A2F46DE }, 0x4B874B79 },
{ { 6171.206, -1554.197, 60.64859 }, { 0, 0, -0.3826833, 0.9238795 }, 0xA29242A7, { 0x2A2F46DE }, 0x4B874B79 },
{ { 6188.35, -1764.747, 15.81685 }, { 0, 0, 0, 1 }, 0xD847AE11, { 0x2A2F46DE }, 0x4B874B79 },
{ { 6033.94, -1604.483, 24.05252 }, { 0, 0, 0, 1 }, 0xB4ADE6DE, { 0x2A2F46DE }, 0x4B874B79 },
{ { 6076.929, -1443.894, 30.25744 }, { 0, 0, 0, 1 }, 0xA00BBD9A, { 0x2A2F46DE }, 0x4B874B79 },
{ { 6477.159, -1772.327, 14.49214 }, { 0, 0, 0, 1 }, 0x58302DE4, { 0x2A2F46DE }, 0x4B874B79 },
{ { 5971.401, -1762.306, 24.98314 }, { 0, 0, 0, 1 }, 0x4D421818, { 0x2A2F46DE }, 0x4B874B79 },
{ { 5920.514, -1939.081, 19.52437 }, { 0, 0, 0, 1 }, 0x63024398, { 0x2A2F46DE }, 0x4B874B79 },
{ { 6076.929, -1443.894, 30.25744 }, { 0, 0, 0, 1 }, 0x53AFCAF0, { 0x2A2F46DE }, 0x4B874B79 },
{ { 6148.496, -1586.307, 43.93963 }, { 0, 0, -0.3826833, 0.9238795 }, 0xB52ACC91, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6143.726, -1591.077, 43.93963 }, { 0, 0, -0.3826833, 0.9238795 }, 0x9FDEA1F5, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6138.957, -1595.846, 43.93963 }, { 0, 0, -0.3826833, 0.9238795 }, 0x8CF07C11, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6134.188, -1600.616, 43.93963 }, { 0, 0, -0.3826833, 0.9238795 }, 0x5B6198F4, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6129.418, -1605.385, 43.93963 }, { 0, 0, -0.3826833, 0.9238795 }, 0x6A6CB70A, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5912.451, -1943.95, 42.46784 }, { 9.999999E-09, 9.999999E-09, -0.7071066, 0.7071068 }, 0xF984560B, { 0x4A526722 }, 0x585F8BDC },
{ { 6166.367, -1617.288, 15.69092 }, { 0, 0, 0.9238796, -0.3826834 }, 0x4F887780, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6170.81, -1612.846, 15.69092 }, { 0, 0, 0.9238796, -0.3826834 }, 0xD1EA2932, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6143.2, -1604.772, 39.96033 }, { 0, 0, -0.3826833, 0.9238795 }, 0x9AF3016B, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6149.192, -1598.78, 39.96033 }, { 0, 0, -0.3826833, 0.9238795 }, 0x7D674668, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6072.688, -1652.029, 15.94708 }, { 0, 0, 0.9239778, -0.3824461 }, 0x8F26E648, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6012.115, -1685.145, 16.25591 }, { -0.0007420601, 0.0208433, -0.2649375, 0.96404 }, 0x1CFF3303, { 0x3898C3AF }, 0x6291A040 },
{ { 6078.862, -1556.034, 16.01326 }, { -0.00292113, 0.00113863, -0.3631754, 0.9317155 }, 0xB6C5B803, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6076.154, -1553.545, 16.01326 }, { -0.00292144, 0.0011385, -0.3631755, 0.9317154 }, 0x091EDCB4, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6125.602, -1562.348, 15.76586 }, { -0.00292113, 0.00113863, -0.3631755, 0.9317154 }, 0x60858B80, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6122.894, -1559.859, 15.76586 }, { -0.00292144, 0.0011385, -0.3631755, 0.9317154 }, 0x72652F3F, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6016.959, -1688.999, 16.01005 }, { 0.0027538, 0.02067391, -0.4224137, 0.9061633 }, 0x2F40D786, { 0x3898C3AF }, 0x6291A040 },
{ { 6041.393, -1703.201, 15.79282 }, { 0, 0, 0.3767649, 0.926309 }, 0x4089FA18, { 0x3898C3AF }, 0x6291A040 },
{ { 6050.505, -1712.915, 15.79282 }, { -1E-08, 3E-08, 0.3767649, 0.9263089 }, 0xD2C39E8D, { 0x3898C3AF }, 0x6291A040 },
{ { 6074.42, -1645.459, 15.94708 }, { 0, 0, 0.9214892, -0.3884038 }, 0xF0362869, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6107.969, -1630.326, 16.00211 }, { 0, 0, 0.3792318, 0.9253018 }, 0x0247CC8C, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6056.469, -1582.876, 15.97429 }, { 0, 0, 0.9270952, 0.3748261 }, 0x4CB36162, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6032.329, -1559.776, 15.97429 }, { 0, 0, 0.9270952, 0.3748261 }, 0xDEDD05B7, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6069.293, -1535.655, 15.97429 }, { 0, 0, 0.9270952, 0.3748261 }, 0x10266845, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6063.337, -1529.087, 15.97429 }, { 0, 0, 0.9270952, 0.3748261 }, 0xA27B0CF0, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6039.832, -1733.459, -0.08750725 }, { 0, 0, 0.9152086, -0.4029803 }, 0x8A5FF525, { 0x3898C3AF }, 0x6291A040 },
{ { 6050.694, -1719.929, 3.987205 }, { 0, 0, -0.7071068, 0.7071066 }, 0x72B708BF, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 6082.048, -1650.559, 30.32123 }, { 0, 0, 0.3826834, 0.9238796 }, 0x173EE579, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5970.771, -1732.562, 16.59304 }, { 0, 0, 0.9998626, 0.01657942 }, 0xC00AFAF8, { 0x3898C3AF }, 0x6291A040 },
{ { 6095.327, -1694.836, 15.86399 }, { 0, 0, 0.08715579, 0.9961947 }, 0x91C51E6D, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6128.591, -1661.102, 15.86399 }, { 0, 0, 0.663402, 0.7482632 }, 0x5C57B393, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6088.446, -1759.562, 15.63708 }, { 0, 0, -0.7071068, 0.7071066 }, 0xAE29D736, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 5909.01, -1596.243, 21.00854 }, { 0.00941847, 0.00097667, 0.1031395, 0.9946219 }, 0xD3E895F7, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5961.637, -1731.653, 31.54816 }, { 0, 0, 0.9998626, 0.01657942 }, 0x2B3903F9, { 0x3898C3AF }, 0x6291A040 },
{ { 6494.388, -1852.277, 10.5346 }, { 0, 0, 0, 1 }, 0x779913D6, { 0x1AB587E9 }, 0xD058FBCD },
{ { 6290.946, -1607.35, 29.14465 }, { 0, 0, -0.3826833, 0.9238795 }, 0x90B56D04, { 0x14D8FC24 }, 0xABE132D6 },
{ { 6280.951, -1619.552, 24.58733 }, { -3.099999E-07, -1.3E-07, -0.3826833, 0.9238795 }, 0x621A8FCF, { 0x14D8FC24 }, 0xABE132D6 },
{ { 6318.044, -1664.027, 35.21197 }, { 0, 0, 0.9238796, -0.3826834 }, 0xF984EF3A, { 0x031C58AB }, 0xC20A5F30 },
{ { 5956.464, -1543.841, 20.01905 }, { 0, 0, 0.4226182, 0.9063078 }, 0x1FB17427, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5972.595, -1527.158, 21.44692 }, { 0, 0, 0.4226183, 0.9063078 }, 0xEE469152, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5995.699, -1501.905, 23.31724 }, { 0, 0, 0.4226183, 0.9063078 }, 0xC46D3DA0, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6016.41, -1480.269, 19.55491 }, { -9.999998E-09, 3E-08, 0.4226183, 0.9063077 }, 0x11B85835, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6038.533, -1460.275, 22.61154 }, { -5.999999E-08, 1.3E-07, 0.4226183, 0.9063077 }, 0xD8F9E6B9, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6053.17, -1442.841, 24.64821 }, { -1.1E-07, 1.1E-07, 0.7071066, 0.7071066 }, 0xA63B013C, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6042.548, -1421.568, 27.01764 }, { -1.4E-07, 5E-08, 0.9396926, 0.3420202 }, 0x7D39AF36, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6027.605, -1406.69, 28.70978 }, { -1.9E-07, 1.9E-07, 0.9396926, 0.3420202 }, 0xCB7ECBC3, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6011.76, -1392.596, 29.85866 }, { -2.8E-07, 4.4E-07, 0.9396926, 0.3420202 }, 0x754D9F62, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6021.07, -1437.265, 30.81182 }, { 1.1E-07, 5.1E-07, 0.4226182, 0.9063078 }, 0x5A1DE7DF, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6019.779, -1534.185, 16.09291 }, { -0.01126789, 0.00535874, -0.3826198, 0.9238216 }, 0x2350C3E4, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6029.564, -1524.245, 16.03886 }, { 0, 0, -0.3826833, 0.9238795 }, 0x389AEE78, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6382.534, -1849.225, 15.97638 }, { 0, 0, -0.7071068, 0.7071066 }, 0x06DE0AFF, { 0x031C58AB }, 0xC20A5F30 },
{ { 6068.602, -1747.476, -2.894766 }, { 6E-08, -1E-07, -0.8573369, 0.5147557 }, 0xC0A471CF, { 0x23BFE7B9 }, 0x867DB1E8 },
{ { 6080.936, -1760.896, -2.894766 }, { -1.1E-07, -2.8E-07, -0.8573369, 0.5147557 }, 0x8A688558, { 0x23BFE7B9 }, 0x867DB1E8 },
{ { 6097.895, -1767.91, 17.18628 }, { 0, 0, 0.9238796, -0.3826832 }, 0xD2D0F38D, { 0x34529D8F }, 0x87BCF383 },
{ { 6100.227, -1765.684, 17.18628 }, { -3E-08, 6E-08, 0.9238796, -0.3826832 }, 0xA00F8E0B, { 0x34529D8F }, 0x87BCF383 },
{ { 6099.683, -1766.349, 19.94571 }, { -2E-08, 6E-08, 0.95502, -0.2965414 }, 0x1FB30D54, { 0x34529D8F }, 0x87BCF383 },
{ { 6100.897, -1758.872, 17.18628 }, { -3E-08, 6E-08, 0.9238796, -0.3826832 }, 0x2DF5A9D9, { 0x34529D8F }, 0x87BCF383 },
{ { 6103.229, -1756.646, 17.18628 }, { -3E-08, 8E-08, 0.9238796, -0.3826832 }, 0xFB4A447F, { 0x34529D8F }, 0x87BCF383 },
{ { 6108.735, -1764.247, 19.98246 }, { 3E-08, 7.999999E-08, 0.9236084, 0.3833374 }, 0x0983E0F2, { 0x34529D8F }, 0x87BCF383 },
{ { 6107.545, -1763.055, 17.18628 }, { 3E-08, 8.000001E-08, 0.9236084, 0.3833375 }, 0x68BE9F6A, { 0x34529D8F }, 0x87BCF383 },
{ { 6109.768, -1765.39, 17.18628 }, { -1E-08, 1.8E-07, 0.9236084, 0.3833375 }, 0x7384B4F6, { 0x34529D8F }, 0x87BCF383 },
{ { 6363.228, -1870.469, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0x7E434B57, { 0x031C58AB }, 0xC20A5F30 },
{ { 6363.228, -1873.803, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0x6C8127D3, { 0x031C58AB }, 0xC20A5F30 },
{ { 6384.09, -1860.125, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0x59AD822C, { 0x031C58AB }, 0xC20A5F30 },
{ { 6393.002, -1835.8, 17.16654 }, { 0, 0, 1, 1.6E-07 }, 0xFD56497F, { 0x34529D8F }, 0x87BCF383 },
{ { 6395.159, -1832.101, 19.95959 }, { 0, 0, 0.7071066, 0.7071069 }, 0x55D0893E, { 0x34529D8F }, 0x87BCF383 },
{ { 6392.796, -1829.206, 19.95373 }, { -7.000001E-08, 7.000001E-08, 0.7071066, 0.7071069 }, 0x1EF69B8B, { 0x34529D8F }, 0x87BCF383 },
{ { 6392.167, -1828.56, 17.16654 }, { -1.6E-07, 1.6E-07, 0.7071066, 0.7071069 }, 0x31A5C0E9, { 0x34529D8F }, 0x87BCF383 },
{ { 6392.167, -1831.251, 17.16654 }, { -3.4E-07, 3.4E-07, 0.7071066, 0.7071069 }, 0xB124BFDD, { 0x34529D8F }, 0x87BCF383 },
{ { 6396.85, -1830.701, 17.16654 }, { 0, 0, 1, 1.6E-07 }, 0x9EEE1B70, { 0x34529D8F }, 0x87BCF383 },
{ { 6319.772, -1744.47, 15.44372 }, { 0, 0, 0.3826833, 0.9238795 }, 0xCF5FA62F, { 0x031C58AB }, 0xC20A5F30 },
{ { 6048.161, -1524.719, 15.99758 }, { -0.00292174, 0.00113836, -0.3631755, 0.9317154 }, 0x832250B9, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6042.883, -1519.903, 15.99758 }, { -0.00292236, 0.00113809, -0.3631755, 0.9317154 }, 0xFD4FC1E6, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6040.175, -1517.414, 15.99758 }, { -0.00292236, 0.00113809, -0.3631755, 0.9317154 }, 0x1003674D, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6498.689, -1924.389, -1.18737 }, { 0, 0, 0, 1 }, 0xB0B27017, { 0x1AB587E9 }, 0xD058FBCD },
{ { 6531.628, -1826.263, -1.101535 }, { 0, 0, 0, 1 }, 0x35A770C1, { 0x1AB587E9 }, 0xD058FBCD },
{ { 6531.758, -1728.895, -0.04487347 }, { 0, 0, 0, 1 }, 0x2785D47E, { 0x1AB587E9 }, 0xD058FBCD },
{ { 6399.549, -1926.623, -0.433752 }, { 0, 0, 0.9848077, 0.1736482 }, 0x8E822BB7, { 0x1AB587E9 }, 0xD058FBCD },
{ { 6443.849, -1767.102, 15.49745 }, { 0, 0, 0, 1 }, 0x7A1ED872, { 0x0117D9C4 }, 0x2791E18C },
{ { 6040.295, -1713.657, 7.195711 }, { 0, 0, -0.3880975, 0.9216183 }, 0x70E0CEAA, { 0x3898C3AF }, 0x6291A040 },
{ { 6407.678, -1871.013, 55.12638 }, { 0, 0, 0, 1 }, 0xE8939981, { 0x031C58AB }, 0xC20A5F30 },
{ { 6020.313, -1760.546, 18.46459 }, { 0, 0, 0, 1 }, 0xB44D5B3D, { 0x3898C3AF }, 0x6291A040 },
{ { 5925.765, -1649.274, 22.38275 }, { 0, 0, 0.9063078, 0.4226183 }, 0xA3703537, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5968.138, -1912.845, 14.45265 }, { 0, 0, 1, 1.6E-07 }, 0x78711265, { 0x4A526722 }, 0x585F8BDC },
{ { 5935.083, -1629.862, 19.05558 }, { 0, 0, 0, 1 }, 0x546AB17B, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5937.362, -1794.104, 21.27824 }, { 0, 0, -0.7071066, 0.7071066 }, 0x898D83F2, { 0x3898C3AF }, 0x6291A040 },
{ { 5920.621, -1743.336, 22.36338 }, { 0, 0, 1, 8E-08 }, 0x77C7E067, { 0x3898C3AF }, 0x6291A040 },
{ { 5908.515, -1757.158, 29.19843 }, { 0, 0, 1, 8E-08 }, 0xCF5731C6, { 0x3898C3AF }, 0x6291A040 },
{ { 5896.223, -1777.742, 29.26539 }, { 0, 0, 0, 1 }, 0xAD97EE44, { 0x3898C3AF }, 0x6291A040 },
{ { 5929.139, -1783.383, 28.10396 }, { 0, 0, 0, 1 }, 0x10897884, { 0x3898C3AF }, 0x6291A040 },
{ { 5908.608, -1755.225, 28.07998 }, { 0, 0, 0, 1 }, 0x69FE13F2, { 0x3898C3AF }, 0x6291A040 },
{ { 5908.766, -1767.758, 27.75429 }, { 0, 0, 0, 1 }, 0x519C9A46, { 0x3898C3AF }, 0x6291A040 },
{ { 6363.228, -1867.135, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0x8C6AF672, { 0x031C58AB }, 0xC20A5F30 },
{ { 6385.951, -1864.197, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0x7AA152DF, { 0x031C58AB }, 0xC20A5F30 },
{ { 6383.933, -1868.082, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0xF12B3FE9, { 0x031C58AB }, 0xC20A5F30 },
{ { 6384.447, -1873.123, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0x037C648B, { 0x031C58AB }, 0xC20A5F30 },
{ { 6385.899, -1876.685, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0xCC55763E, { 0x031C58AB }, 0xC20A5F30 },
{ { 6385.899, -1879.588, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0x92520390, { 0x031C58AB }, 0xC20A5F30 },
{ { 6385.899, -1882.598, 16.13919 }, { 0, 0, -0.7071068, 0.7071066 }, 0x6AE534BF, { 0x031C58AB }, 0xC20A5F30 },
{ { 5848.818, -1933.323, 8.177452 }, { 0, 0, 0.4097475, 0.9121989 }, 0x0E294FF3, { 0x4A526722 }, 0x585F8BDC },
{ { 5865.081, -1919.461, 11.33455 }, { 0, 0, 0.3980166, 0.9173782 }, 0x375EA261, { 0x4A526722 }, 0x585F8BDC },
{ { 5848.818, -1933.323, 16.06618 }, { -3.999999E-08, 7.999999E-08, 0.4097475, 0.9121989 }, 0xF18696AE, { 0x4A526722 }, 0x585F8BDC },
{ { 5865.081, -1919.461, 19.22328 }, { -3E-08, 8E-08, 0.3980167, 0.9173782 }, 0x20B47509, { 0x4A526722 }, 0x585F8BDC },
{ { 5856.806, -1983.381, 6.556898 }, { 0, 0, 0.3373637, 0.9413743 }, 0xF85AA45A, { 0x4A526722 }, 0x585F8BDC },
{ { 5867.384, -1975.431, 6.556898 }, { 0, 0, 0.3373637, 0.9413743 }, 0x2288F8B6, { 0x4A526722 }, 0x585F8BDC },
{ { 6024.928, -1643.787, 28.98598 }, { 0, 0, 0, 1 }, 0x63ADFE88, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6034.771, -1635.608, 28.98598 }, { 0, 0, 0, 1 }, 0xF6712D36, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6047.006, -1622.478, 28.98598 }, { 0, 0, 0.7071066, 0.7071066 }, 0x04AFC9B3, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6055.563, -1611.21, 28.98598 }, { 0, 0, 0.7071066, 0.7071066 }, 0xEA3820F7, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5925.583, -1633.819, 22.24704 }, { 0, 0, 0, 1 }, 0x2AE3FABF, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5985.124, -1657.124, 19.85987 }, { 0, 0, 0, 1 }, 0x1D655FC2, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5944.317, -1610.942, 33.54797 }, { 0, 0, -0.7131553, 0.7010063 }, 0x193DF2CF, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5965.539, -1587.673, 33.50534 }, { 0, 0, 0.9999002, -0.01413103 }, 0x294392DA, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5886.036, -1826.444, 22.15589 }, { 0, 0, 0.7071066, 0.7071066 }, 0x7DFE7254, { 0x3898C3AF }, 0x6291A040 },
{ { 5977.321, -1676.136, 21.06034 }, { 0, 0, 0.9317854, 0.3630096 }, 0xEF3930A4, { 0x3898C3AF }, 0x6291A040 },
{ { 5920.114, -1966.11, 6.274292 }, { 0, 0, 0.8970514, 0.4419262 }, 0xDAFACEE0, { 0x4A526722 }, 0x585F8BDC },
{ { 6084.745, -1542.108, 15.78296 }, { 0, 0, -0.3796289, 0.9251388 }, 0x2CD1F28D, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6074.194, -1531.612, 15.90803 }, { 0, 0, -0.381654, 0.9243053 }, 0x11E83CBA, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6072.685, -1528.733, 15.93502 }, { 0.00251546, -0.00103866, -0.3816526, 0.9243019 }, 0x1F0A56FE, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5975.349, -1808.863, 23.88464 }, { -1.999999E-08, -1.999999E-08, -0.7071066, 0.7071066 }, 0xF3E75F4C, { 0x3898C3AF }, 0x6291A040 },
{ { 6071.229, -1843.086, 25.11197 }, { 0, 0, -0.4613764, 0.8872044 }, 0xF5B86D3B, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 6273.69, -1743.904, 0.02383804 }, { 0, 0, 0, 1 }, 0x35209474, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 6068.554, -1517.622, 16.03348 }, { -0.02848531, 0.00023155, -0.2693404, 0.9626236 }, 0xF901E5AB, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6029.887, -1574.732, 16.02037 }, { -0.00289916, 0.00119346, -0.3806618, 0.924709 }, 0xA9C19ACB, { 0x6DD62E29 }, 0xC2405F94 },
{ { 5914.795, -1867.38, 13.30383 }, { 0, 0, -0.7071276, 0.7070858 }, 0xE7A1682D, { 0x3898C3AF }, 0x6291A040 },
{ { 5943.971, -1922.98, 25.17094 }, { 0, 0, 0, 1 }, 0x95ED3831, { 0x4A526722 }, 0x585F8BDC },
{ { 5893.367, -1674.395, 39.82641 }, { 0, 0, 0, 1 }, 0x4F330FBF, { 0x3898C3AF }, 0x6291A040 },
{ { 6142.413, -1823.557, 25.09966 }, { 0, 0, -0.3496114, 0.9368948 }, 0x06720EAE, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 6414.741, -1833.319, 28.24815 }, { 0, 0, 0, 1 }, 0x12FD9A3B, { 0x031C58AB }, 0xC20A5F30 },
{ { 6371.673, -1783.031, 17.73939 }, { 0, 0, 0.999914, 0.01311901 }, 0xA42EBD1F, { 0x031C58AB }, 0xC20A5F30 },
{ { 6363.637, -1782.836, 17.73771 }, { 0, 0, 0.9618161, 0.273696 }, 0x4FF294B0, { 0x031C58AB }, 0xC20A5F30 },
{ { 6349.015, -1738.274, 17.45338 }, { 0, 0, -0.3748655, 0.9270792 }, 0x7334CC6C, { 0x031C58AB }, 0xC20A5F30 },
{ { 6354.776, -1743.889, 17.45338 }, { 0, 0, -0.608613, 0.7934672 }, 0x4825A677, { 0x031C58AB }, 0xC20A5F30 },
{ { 6199.091, -1554.756, 14.73874 }, { 0, 0, 0.3826835, 0.9238795 }, 0xA68EBBB1, { 0x14D8FC24 }, 0xABE132D6 },
{ { 5867.263, -1994.882, 5.003383 }, { 0.1813388, -0.1259898, 0.5370028, 0.8141687 }, 0x35872C4C, { 0xD6A8285E }, 0x592ABA64 },
{ { 6047.419, -1814.527, 8.425564 }, { 0, 0, 0.00591168, 0.9999826 }, 0xB4DA1329, { 0x3898C3AF }, 0x6291A040 },
{ { 6087.819, -1768.731, 30.59197 }, { 0, 0, 0.3826834, 0.9238796 }, 0x8275ED51, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 6228.34, -1688.545, 6.372677 }, { 0.09718798, 0.05821194, 0.9398631, 0.3222159 }, 0x6DA7790E, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 6452.823, -1740.09, 16.26805 }, { 0, 0, 0, 1 }, 0xEC0CD55D, { 0x0117D9C4 }, 0x2791E18C },
{ { 6482.737, -1737.97, 16.26805 }, { 0, 0, 0, 1 }, 0xF5BAF30D, { 0x0117D9C4 }, 0x2791E18C },
{ { 6258.155, -1596.587, 30.16651 }, { 0, 0, -0.6772432, 0.7357591 }, 0xA8252381, { 0x14D8FC24 }, 0xABE132D6 },
{ { 6147.69, -1718.511, 3.510229 }, { -0.0015868, 0.02692878, 0.4741735, 0.8800181 }, 0x4B189ED7, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 6215.321, -1649.33, 5.014863 }, { -0.008123068, -0.00174071, 0.9142476, -0.4050704 }, 0xB8D5FA50, { 0xDF9E91B0 }, 0x9D1B954B },
{ { 5955.701, -1793.18, 31.63316 }, { 0, 0, -0.7071066, 0.7071066 }, 0xCC2DCCE8, { 0x3898C3AF }, 0x6291A040 },
{ { 5946.478, -1877.045, 19.91463 }, { -9.999997E-09, -9.999997E-09, -0.7071066, 0.7071066 }, 0xBCED7D97, { 0x3898C3AF }, 0x6291A040 },
{ { 5965.191, -1635.312, 15.86908 }, { 0, 0, 0.9251742, 0.3795427 }, 0xE9E2B800, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6006.765, -1969.29, 13.79201 }, { 0, 0, 0, 1 }, 0x9DF09A40, { 0x4A526722 }, 0x585F8BDC },
{ { 6022.11, -1951.635, 13.79201 }, { 0, 0, 0, 1 }, 0x584C8EF9, { 0x4A526722 }, 0x585F8BDC },
{ { 6024.233, -1917.097, 13.77843 }, { 0, 0, 0, 1 }, 0x6152A105, { 0x4A526722 }, 0x585F8BDC },
{ { 5973.778, -1970.06, 13.79201 }, { 0, 0, 0, 1 }, 0xC0295EAD, { 0x4A526722 }, 0x585F8BDC },
{ { 5938.124, -1970.06, 13.566 }, { 0, 0, 0, 1 }, 0xC9637121, { 0x4A526722 }, 0x585F8BDC },
{ { 5990.608, -1915.182, 22.79812 }, { 0, 0, 0, 1 }, 0x83C665E8, { 0x4A526722 }, 0x585F8BDC },
{ { 5898.134, -1914.834, 23.73952 }, { 0, 0, 0, 1 }, 0xAD0FB87A, { 0x4A526722 }, 0x585F8BDC },
{ { 5936.629, -1633.637, 18.67466 }, { 0, 0, 0, 1 }, 0x78EADB70, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6014.143, -1697.183, 23.55788 }, { 0, 0, 0.3420201, 0.9396927 }, 0xF510C87B, { 0x3898C3AF }, 0x6291A040 },
{ { 5999.913, -1683.232, 24.72483 }, { 0, 0, 0.3420201, 0.9396927 }, 0x074E6CF6, { 0x3898C3AF }, 0x6291A040 },
{ { 6018.35, -1523.758, 17.45887 }, { 0, 0, 0, 1 }, 0x02F4A11D, { 0x6DD62E29 }, 0xC2405F94 },
{ { 6000.396, -1630.171, 33.34385 }, { 0, 0, 0, 1 }, 0x2F90552F, { 0x2F7A499E }, 0x607DC548 },
{ { 6126.643, -1456.061, 33.41807 }, { 0, 0, 0, 1 }, 0x213D3889, { 0x2F7A499E }, 0x607DC548 },
{ { 6233.476, -1574.707, 24.95966 }, { 0, 0, 0, 1 }, 0x76CCE40B, { 0x2F7A499E }, 0x607DC548 },
{ { 6465.184, -1812.479, 20.14797 }, { 0, 0, 0, 1 }, 0x651740A0, { 0x2F7A499E }, 0x607DC548 },
{ { 5745.488, -2474.355, 11.91254 }, { 0, 0, 0, 1 }, 0x98B17AF6, { 0xFED05D06 }, 0xE24C234B },
{ { 5701.487, -2190.424, 25.29133 }, { 0, 0, 0, 1 }, 0x2A591E43, { 0xFED05D06 }, 0xE24C234B },
{ { 5756.753, -2520.429, 5.213078 }, { 0, 0, 0, 1 }, 0x144E10B1, { 0x9015C93A }, 0xD047C500 },
{ { 5756.201, -2520.429, 11.04491 }, { 0, 0, 0, 1 }, 0x8599C2FC, { 0x9015C93A }, 0xD047C500 },
{ { 5738.404, -2376.72, 29.04327 }, { 0, 0, 0, 1 }, 0x9E6259A9, { 0x9015C93A }, 0xD047C500 },
{ { 5736.856, -2499.862, 29.05008 }, { 0, 0, 0.7100244, 0.7041772 }, 0xC2BBD119, { 0x9015C93A }, 0xD047C500 },
{ { 5736.614, -2495.43, 27.8457 }, { 0, 0, 0, 1 }, 0x77E4CD8D, { 0x9015C93A }, 0xD047C500 },
{ { 5723.813, -2266.555, 29.05008 }, { 0, 0, 0.006865059, 0.9999763 }, 0xC575E558, { 0x91F8895F }, 0xA7B40F0F },
{ { 5650.255, -2146.019, 1.536312 }, { 0, 0, 0.7071066, 0.7071066 }, 0x43D47298, { 0x91F8895F }, 0xA7B40F0F },
{ { 5640.025, -2166.689, 6.878936 }, { 0, 0, 0, 1 }, 0x2AA34E44, { 0x91F8895F }, 0xA7B40F0F },
{ { 5786.571, -2366.337, 3.268728 }, { 0, 0, 0.08715583, 0.9961947 }, 0xFE9A1D5A, { 0x9015C93A }, 0xD047C500 },
{ { 5808.252, -2489.601, 3.268728 }, { 0, 0, 0, 1 }, 0x010C0540, { 0x9015C93A }, 0xD047C500 },
{ { 5656.393, -2169.393, 13.06302 }, { 0, 0, 0, 1 }, 0x30D21394, { 0x91F8895F }, 0xA7B40F0F },
{ { 5677.915, -2189.524, 1.744557 }, { 0, 0, 0, 1 }, 0x95DCE8F6, { 0x91F8895F }, 0xA7B40F0F },
{ { 5738.628, -2579.142, 23.69111 }, { 0, 0, 0, 1 }, 0x022ABB60, { 0x9015C93A }, 0xD047C500 },
{ { 5743.203, -2579.38, 27.65263 }, { 0, 0, 0, 1 }, 0x5DF25E5D, { 0x9015C93A }, 0xD047C500 },
{ { 5804.642, -2499.632, 12.21606 }, { 0, 0, 0, 1 }, 0x5EE4D1AA, { 0x9015C93A }, 0xD047C500 },
{ { 5833.505, -2477.245, 12.21606 }, { 0, 0, 0, 1 }, 0x92908A0F, { 0x9015C93A }, 0xD047C500 },
{ { 5873.823, -2506.908, 12.21606 }, { 0, 0, 0, 1 }, 0xC53B6F64, { 0x9015C93A }, 0xD047C500 },
{ { 5738.68, -2388.915, 20.22503 }, { 0, 0, 0, 1 }, 0x15FE88B1, { 0x9015C93A }, 0xD047C500 },
{ { 5772.559, -2584.704, 20.08696 }, { 0, 0, 0, 1 }, 0x08F41F7F, { 0x9015C93A }, 0xD047C500 },
{ { 5820.958, -2495.492, 7.965477 }, { 0, 0, 0, 1 }, 0x77B1599B, { 0x9015C93A }, 0xD047C500 },
{ { 5661.799, -2175.78, 7.448132 }, { 0, 0, 0, 1 }, 0x71B05545, { 0x91F8895F }, 0xA7B40F0F },
{ { 5814.353, -2224.033, 13.15895 }, { 0, 0, 0, 1 }, 0xBFBF6E02, { 0x2F7A499E }, 0xC25C6ADD },
{ { 5741.684, -2166.997, 12.77802 }, { 0, 0, 0, 1 }, 0x61D8B232, { 0x2F7A499E }, 0xC25C6ADD },
{ { 5675.244, -2291.095, 13.32893 }, { 0, 0, 0, 1 }, 0x5407168F, { 0x2F7A499E }, 0xC25C6ADD },
{ { 5666.304, -2399.857, 13.1082 }, { 0, 0, 0, 1 }, 0x7AF7646F, { 0x2F7A499E }, 0xC25C6ADD },
{ { 5683.6, -2491.121, 13.62663 }, { 0, 0, 0, 1 }, 0x76B05BE1, { 0x2F7A499E }, 0xC25C6ADD },
{ { 5738.997, -2607.576, 10.2549 }, { 0, 0, 0, 1 }, 0x0A8F83A1, { 0x2F7A499E }, 0xC25C6ADD },
{ { 5729.463, -2614.023, 2.738737 }, { 0, 0, 0, 1 }, 0x229DDA08, { 0xF9CB7ED6 }, 0x3FA01B0F },
{ { 5718.283, -2493.528, 3.254355 }, { 0, 0, 0, 1 }, 0xD4FEBECB, { 0xF9CB7ED6 }, 0x3FA01B0F },
{ { 5670.964, -2386.477, 2.081531 }, { 0, 0, 0, 1 }, 0xFF451357, { 0xF9CB7ED6 }, 0x3FA01B0F },
{ { 5683.388, -2307.183, 2.960272 }, { 0, 0, 0, 1 }, 0x3189F7E0, { 0xF9CB7ED6 }, 0x3FA01B0F },
{ { 5799.338, -2271.063, 3.093381 }, { 0, 0, 0, 1 }, 0xD618C0FF, { 0xF9CB7ED6 }, 0x3FA01B0F },
{ { 5712.083, -2190.387, 3.564596 }, { 0, 0, 0, 1 }, 0x8BACAC0C, { 0xF9CB7ED6 }, 0x3FA01B0F },
{ { 5816.685, -2185.744, 3.37346 }, { 0, 0, 0, 1 }, 0xC54A9F63, { 0xF9CB7ED6 }, 0x3FA01B0F },
{ { 5574.868, -1264.712, 9.864773 }, { 0, 0, 0, 1 }, 0xC5505DD1, { 0x15433B72 }, 0x44F183CA },
{ { 5752.69, -1919.095, 10.27065 }, { 0, 0, 0, 1 }, 0xD2CE78BD, { 0x15433B72 }, 0x44F183CA },
{ { 5781.986, -1703.686, 34.49774 }, { 0, 0, 0, 1 }, 0x8C18EB63, { 0x15433B72 }, 0x44F183CA },
{ { 5649.504, -1691.761, 24.61555 }, { 0, 0, 0, 1 }, 0x59CB86C9, { 0x15433B72 }, 0x44F183CA },
{ { 5523.212, -1502.06, 15.20859 }, { 0, 0, 0, 1 }, 0x7E2E4F8E, { 0x15433B72 }, 0x44F183CA },
{ { 5685.388, -1532.899, 39.70407 }, { 0, 0, 0, 1 }, 0xA06E940E, { 0x15433B72 }, 0x44F183CA },
{ { 5880.126, -1467.381, 45.9609 }, { 0, 0, 0, 1 }, 0xD2BCF8AA, { 0x15433B72 }, 0x44F183CA },
{ { 5853.219, -1361.12, 31.18866 }, { 0, 0, 0, 1 }, 0x278187BD, { 0x15433B72 }, 0x44F183CA },
{ { 5945.601, -1214.684, 17.57967 }, { 0, 0, 0, 1 }, 0x8AFCCEB2, { 0x15433B72 }, 0x44F183CA },
{ { 5730.861, -1207.96, 8.052512 }, { 0, 0, 0, 1 }, 0xD843ADDA, { 0x15433B72 }, 0x44F183CA },
{ { 5602.079, -1900.8, 37.12418 }, { 0, 0, 0, 1 }, 0xC46FDC00, { 0x15433B72 }, 0x44F183CA },
{ { 5526.141, -1710.171, 47.52297 }, { 0, 0, 0, 1 }, 0x805CC90F, { 0x15433B72 }, 0x44F183CA },
{ { 5799.355, -1762.587, 31.856 }, { 0, 0, 0, 1 }, 0x943B3F4E, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5711.13, -1293.885, 25.31346 }, { 0, 0, 0, 1 }, 0xF77043FE, { 0xD1AE00C8 }, 0x9CF5A2FD },
{ { 5892.189, -1338.003, 30.25211 }, { 0, 0, 0, 1 }, 0x4C0DA3E8, { 0xD1AE00C8 }, 0x9CF5A2FD },
{ { 5738.461, -1968.43, 25.03018 }, { 0, 0, 0, 1 }, 0x091CA2D7, { 0xC3E5E52C }, 0x440BF12F },
{ { 5751.943, -1473.174, 36.38371 }, { 0, 0, 0.7071066, 0.7071066 }, 0xF3F17111, { 0x5E14197E }, 0x527F0E15 },
{ { 5730.966, -1281.4, 27.61022 }, { 0, 0, 0.7071066, 0.7071066 }, 0xC1388BA0, { 0xD1AE00C8 }, 0x9CF5A2FD },
{ { 5699.745, -1769.126, 34.34001 }, { 0, 0, 0, 1 }, 0xA2761CC1, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5568.959, -1508.35, 50.89593 }, { 0, 0, 0, 1 }, 0x179620A7, { 0x8E5FFA15 }, 0xC14A6BA6 },
{ { 5696.167, -1869.284, 25.58297 }, { 0, 0, 0.8870109, 0.4617486 }, 0x80A70CF9, { 0xC3E5E52C }, 0x440BF12F },
{ { 5749.289, -1893.315, 12.11064 }, { 0, 0, 0, 1 }, 0x820BC72D, { 0xC3E5E52C }, 0x440BF12F },
{ { 5700.713, -1878.914, 13.06646 }, { 0, 0, 0, 1 }, 0xB3A22A59, { 0xC3E5E52C }, 0x440BF12F },
{ { 5671.967, -1785.464, 11.44354 }, { 0, 0, 0, 1 }, 0xA36889E6, { 0x7967D025 }, 0x7CE9E2EA },
{ { 5757.46, -1785.145, 12.21785 }, { 0, 0, 0, 1 }, 0xD4F76D03, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5814.486, -1780.495, 14.07413 }, { 0, 0, -9E-08, 1 }, 0xF6C9B0A7, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5771.061, -1507.044, 34.09373 }, { 0, 0, -9E-08, 1 }, 0x0E1EDF51, { 0x5E14197E }, 0x527F0E15 },
{ { 5829.828, -1691.203, 24.61525 }, { 0, 0, 0, 1 }, 0xE87B140A, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5584.457, -1494.687, 22.5552 }, { 0, 0, 0, 1 }, 0x1C747BFC, { 0x8E5FFA15 }, 0xC14A6BA6 },
{ { 5838.771, -1845.28, 46.54572 }, { 0, 0, 0, 1 }, 0x8830E6BB, { 0xC3E5E52C }, 0x440BF12F },
{ { 5729.359, -1760.322, 16.86969 }, { -0.00848101, 0.00879801, 0.7065743, 0.7075333 }, 0xC88986EB, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5836.721, -1699.904, 46.87609 }, { 0, 0, 0, 1 }, 0xA6FFDCA6, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5850.242, -1570, 33.13189 }, { 0, 0, 0, 1 }, 0x6B655FD1, { 0x5E14197E }, 0x527F0E15 },
{ { 5575.734, -1608.905, 16.91959 }, { 0, 0, 9E-08, 1 }, 0x9CD400B2, { 0x7967D025 }, 0x7CE9E2EA },
{ { 5604.083, -1660.946, 18.63148 }, { 0, 0, 0, 1 }, 0x551C22C5, { 0x7967D025 }, 0x7CE9E2EA },
{ { 5643.468, -1518.613, 18.14321 }, { 0, 0, 0, 1 }, 0xCB730530, { 0x8E5FFA15 }, 0xC14A6BA6 },
{ { 5800.464, -1571.499, 85.42181 }, { 0, 0, 0, 1 }, 0xD4B0320D, { 0x5E14197E }, 0x527F0E15 },
{ { 5704.531, -1545.35, 55.17906 }, { 0, 0, 0, 1 }, 0xBE77B646, { 0x8E5FFA15 }, 0xC14A6BA6 },
{ { 5914.352, -1528.651, 37.08054 }, { 0, 0, 0, 1 }, 0x1F3781B0, { 0x5E14197E }, 0x527F0E15 },
{ { 5987.224, -1455.6, 36.97401 }, { 0, 0, 0, 1 }, 0xECA1F4A3, { 0x5E14197E }, 0x527F0E15 },
{ { 5708.41, -1694.306, 33.49917 }, { 0, 0, 1.2E-07, 1 }, 0x7832630C, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5725.424, -1782.664, 34.94005 }, { 0, 0, 0, 1 }, 0x89236DF8, { 0xA7B92CD3 }, 0x0DCB84AF },
{ { 5667.205, -1776.338, 26.98258 }, { 0, 0, -9E-08, 1 }, 0x73624276, { 0x7967D025 }, 0x7CE9E2EA },
{ { 5692.598, -1820.218, 33.74273 }, { 0, 0, 0, 1 }, 0x6D97B6E1, { 0xC3E5E52C }, 0x440BF12F },
{ { 5737.066, -1828.003, 27.50195 }, { 0, 0, -9E-08, 1 }, 0x57D78B61, { 0xC3E5E52C }, 0x440BF12F },
{ { 5700.286, -1866.47, 35.95545 }, { 0, 0, 0, 1 }, 0xC3E97E4F, { 0xC3E5E52C }, 0x440BF12F },
{ { 5738.442, -1882.171, 27.91618 }, { 0, 0, -9E-08, 1 }, 0xB239DAF0, { 0xC3E5E52C }, 0x440BF12F },
{ { 5755.905, -1861.623, 34.84999 }, { 0, 0, 0, 1 }, 0xF45DC473, { 0xC3E5E52C }, 0x440BF12F },
{ { 5785.593, -1965.958, 16.92804 }, { 0, 0, 0, 1 }, 0x4CB4A2E9, { 0x2F7A499E }, 0x19195F56 },
{ { 5796.087, -1848.457, 22.23767 }, { 0, 0, 0, 1 }, 0x8B5720F2, { 0x2F7A499E }, 0x19195F56 },
{ { 5616.44, -1893.453, 21.53937 }, { 0, 0, 0, 1 }, 0x818C0D5C, { 0x2F7A499E }, 0x19195F56 },
{ { 5563.072, -1807.609, 23.48747 }, { 0, 0, 0, 1 }, 0xB1F2EE29, { 0x2F7A499E }, 0x19195F56 },
{ { 5527.466, -1679.501, 25.28973 }, { 0, 0, 0, 1 }, 0x5651B6E4, { 0x2F7A499E }, 0x19195F56 },
{ { 5616.304, -1594.426, 24.90947 }, { 0, 0, 0, 1 }, 0x44A5138B, { 0x2F7A499E }, 0x19195F56 },
{ { 5789.649, -1559.26, 37.15662 }, { 0, 0, 0, 1 }, 0x78FF7C3F, { 0x2F7A499E }, 0x19195F56 },
{ { 5594.006, -1517.578, 28.5757 }, { 0, 0, 0, 1 }, 0x67A5D98C, { 0x2F7A499E }, 0x19195F56 },
{ { 5937.325, -1496.455, 41.72505 }, { 0, 0, 0, 1 }, 0xFB467D6F, { 0x2F7A499E }, 0x19195F56 },
{ { 5554.793, -1406.154, 29.61878 }, { 0, 0, 0, 1 }, 0xFD6E051E, { 0x2F7A499E }, 0x19195F56 },
{ { 5542.943, -1290.77, 25.35309 }, { 0, 0, 0, 1 }, 0xE8CED880, { 0x2F7A499E }, 0x19195F56 },
{ { 5654.026, -1405.364, 36.02243 }, { 0, 0, 0, 1 }, 0xD6A8B434, { 0x2F7A499E }, 0x19195F56 },
{ { 5645.912, -1292.026, 27.51015 }, { 0, 0, 0, 1 }, 0x345F6FA0, { 0x2F7A499E }, 0x19195F56 },
{ { 5729.148, -1409.223, 39.76802 }, { 0, 0, 0, 1 }, 0x222DCB3D, { 0x2F7A499E }, 0x19195F56 },
{ { 5829.893, -1413.205, 41.87315 }, { 0, 0, 0, 1 }, 0x0FDCA69B, { 0x2F7A499E }, 0x19195F56 },
{ { 5742.304, -1280.333, 31.71339 }, { 0, 0, 0, 1 }, 0xFD8201E6, { 0x2F7A499E }, 0x19195F56 },
{ { 5833.167, -1287.3, 30.021 }, { 0, 0, 0, 1 }, 0x78D87891, { 0x2F7A499E }, 0x19195F56 },
{ { 5968.154, -1281.217, 31.35775 }, { 0, 0, 0, 1 }, 0x672E553D, { 0x2F7A499E }, 0x19195F56 },
{ { 5968.077, -1402.167, 43.46361 }, { 0, 0, 0, 1 }, 0x5574B1CA, { 0x2F7A499E }, 0x19195F56 },
{ { 5804.485, -1713.696, 28.11262 }, { 0, 0, 0, 1 }, 0x9FFF4A42, { 0x2F7A499E }, 0x19195F56 },
{ { 5443.754, -1681.047, 7.157558 }, { 0, 0, -0.3420201, 0.9396926 }, 0x156935FA, { 0x75774850 }, 0x216AABED },
{ { 5497.636, -1824.15, 5.537985 }, { 0, 0, -9E-08, 1 }, 0x06B71896, { 0x75774850 }, 0x216AABED },
{ { 5550.856, -1914.627, 5.537985 }, { 0, 0, -9E-08, 1 }, 0x62652D72, { 0x75774850 }, 0x216AABED },
{ { 6730.775, -3284.02, 4.375515 }, { 0, 0, 0, 1 }, 0xB28958D4, { 0x0C48CF78 }, 0x0E6790CC },
{ { 6587.713, -3276.845, 24.57089 }, { 0, 0, 0, 1 }, 0xE0C6B54E, { 0x0C48CF78 }, 0x0E6790CC },
{ { 6798.216, -3079.723, 18.74886 }, { 0, 0, 0, 1 }, 0xFD5B6E77, { 0x0C48CF78 }, 0x0E6790CC },
{ { 6964.182, -3125.611, 10.0705 }, { 0, 0, 0, 1 }, 0xCF1911F3, { 0x0C48CF78 }, 0x0E6790CC },
{ { 6959.421, -3265.199, -7.57048 }, { 0, 0, 0, 1 }, 0x35ECDF9D, { 0x0C48CF78 }, 0x0E6790CC },
{ { 6565.358, -3106.712, 39.13364 }, { 0, 0, 0, 1 }, 0x7AE7E98E, { 0x0C48CF78 }, 0x0E6790CC },
{ { 6911.667, -3052.815, 30.07167 }, { 0, 0, -0.7650035, 0.6440261 }, 0xF4F305DE, { 0xF31C1367 }, 0x5EA26055 },
{ { 6624.043, -3288.656, 28.54236 }, { 0, 0, -0.3194569, 0.9476008 }, 0x06352862, { 0xD62C5988 }, 0x4B07B920 },
{ { 6525.048, -3294.537, 36.94773 }, { 0, 0, 0, 1 }, 0x0C8C7868, { 0xD62C5988 }, 0x4B07B920 },
{ { 6486.984, -3252.56, 36.39106 }, { 0, 0, 0, 1 }, 0xCC2A8DF8, { 0xD62C5988 }, 0x4B07B920 },
{ { 6588.908, -2974.912, 25.76238 }, { 0, 0, 0, 1 }, 0x2A0B1C72, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6959.814, -3096.276, 32.90275 }, { 0, 0, 0, 1 }, 0xD1300261, { 0xF31C1367 }, 0x5EA26055 },
{ { 6540.195, -3119.771, 41.12955 }, { 0, 0, 0, 1 }, 0x1E11269A, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6986.567, -3160.657, 14.73235 }, { 0, 0, -0.7872298, 0.6166598 }, 0x02C18A86, { 0xF31C1367 }, 0x5EA26055 },
{ { 6995.266, -3054.801, 14.74135 }, { 0, 0, -0.6578847, 0.7531186 }, 0x462C115A, { 0xF31C1367 }, 0x5EA26055 },
{ { 6958.485, -3032.06, 21.18367 }, { 0, 0, -0.5714138, 0.8206621 }, 0x64634DC8, { 0xF31C1367 }, 0x5EA26055 },
{ { 6925.347, -3087.613, 21.24 }, { 0, 0, 0.5317757, 0.8468853 }, 0xA7F7D4F0, { 0xF31C1367 }, 0x5EA26055 },
{ { 6850.776, -3078.445, 23.23845 }, { 0.00732013, 0.02417743, 0.9411349, 0.3370859 }, 0x97A8BAFC, { 0xF31C1367 }, 0x5EA26055 },
{ { 6645.222, -2963.247, 17.37275 }, { 0, 0, -0.5299209, 0.8480471 }, 0x5749B395, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6872.155, -3031.439, 24.74404 }, { 0, 0, -0.6801061, 0.7331137 }, 0x2A584F68, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6721.872, -3223.249, 19.95047 }, { 0, 0, -0.4564004, 0.8897745 }, 0xC76D46C5, { 0xD62C5988 }, 0x4B07B920 },
{ { 6744.403, -3158.486, 22.73109 }, { 0, 0, 0.972135, -0.2344217 }, 0xDBFB2B61, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6694.373, -2953.532, 13.08976 }, { 0, 0, -0.6232658, 0.7820099 }, 0xB532A250, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6470.185, -3283.91, 25.5476 }, { 0, 0, -0.5378913, 0.8430142 }, 0x56DF6364, { 0xD62C5988 }, 0x4B07B920 },
{ { 6706.787, -3183.7, 23.73706 }, { 0, 0, 0.1962064, 0.9805626 }, 0xADCF4F0A, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6846.63, -3073.963, 38.65704 }, { 0, 0, -9E-08, 1 }, 0x515BED03, { 0xF31C1367 }, 0x5EA26055 },
{ { 6674.136, -2969.67, 24.8664 }, { -0.02801777, -0.04718726, -0.650619, 0.7574189 }, 0xCC00277E, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6481.86, -3176.652, 58.2297 }, { 0, 0, 0, 1 }, 0x04190297, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6563.396, -3043.455, 46.80126 }, { 0, 0, 0.2739702, 0.9617382 }, 0xDD065487, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6628.585, -3136.007, 36.51511 }, { 0, 0, 0, 1 }, 0x44D1996A, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6813.833, -3051.191, 34.19594 }, { 0, 0, 0, 1 }, 0xF49EEF8F, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6642.928, -3212.12, 34.44093 }, { 0, 0, 0, 1 }, 0x1F3BAEF9, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6820.724, -3016.127, 34.00793 }, { 0, 0, 1, -8E-08 }, 0x63A468A2, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6519.559, -3022.464, 51.75967 }, { 0, 0, 0.01035132, 0.9999464 }, 0xAE423895, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6523.551, -3280.883, 33.66513 }, { 0, 0, 0.7071066, 0.7071066 }, 0xA3790A75, { 0xD62C5988 }, 0x4B07B920 },
{ { 6526.214, -3257.59, 36.9164 }, { 0, 0, 0.7071066, 0.7071066 }, 0xA86C69FE, { 0xD62C5988 }, 0x4B07B920 },
{ { 6472.483, -3262.44, 47.5064 }, { 0, 0, 0.2413521, 0.9704376 }, 0xC8679C36, { 0xD62C5988 }, 0x4B07B920 },
{ { 6485.184, -3239.173, 43.2233 }, { 0, 0, 0.0002407801, 1 }, 0xD37C4019, { 0xD62C5988 }, 0x4B07B920 },
{ { 6521.931, -3166.592, 45.2777 }, { 0, 0, 0.6785793, 0.7345272 }, 0x8B68AFF3, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6632.598, -3079.978, 47.42807 }, { 0, 0, -0.0002809201, 1 }, 0x7796084E, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6826.819, -3032.872, 31.16462 }, { 0, 0, 0, 1 }, 0x930BA707, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6813.833, -3051.183, 30.37066 }, { 0, 0, 0, 1 }, 0xA359534B, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6569.108, -3185.525, 38.26388 }, { 0, 0, 0.7071068, 0.7071066 }, 0x7808F4D2, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6514.114, -3168.931, 45.95502 }, { 0, 0, -0.201549, 0.9794784 }, 0x6FEB4420, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6514.665, -3179.385, 43.29458 }, { 0, 0, 0.532899, 0.8461788 }, 0x7F19E27D, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6581.888, -3127.877, 34.74096 }, { 0, 0, 0, 1 }, 0xF2CC1258, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6646.584, -3208.577, 37.29169 }, { 0, 0, 0, 1 }, 0x58FF7802, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6700.978, -3130.609, 27.32759 }, { 0, 0, 0, 1 }, 0x13DFF3B0, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6708.396, -3004.553, 24.042 }, { 0, 0, -0.6856685, 0.727914 }, 0x69CBDAB7, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6914.201, -3016.7, 25.98884 }, { 0, 0, 0.1996792, 0.9798613 }, 0x285BD7DD, { 0xF31C1367 }, 0x5EA26055 },
{ { 6941.201, -3031.382, 25.98884 }, { 0, 0, 0.1996792, 0.9798613 }, 0xBB703A3C, { 0xF31C1367 }, 0x5EA26055 },
{ { 6659.786, -3001.305, 32.69125 }, { 0, 0, 0.9905692, 0.1370131 }, 0x388A34BE, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6631.732, -3234.89, 29.78967 }, { 0, 0, 0.7071066, 0.7071066 }, 0xBA0C1823, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6631.832, -3206.8, 30.24688 }, { 0, 0, 0.7071066, 0.7071066 }, 0xCCE8BDDC, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6637.435, -3206.8, 37.41655 }, { 0, 0, 0.7071066, 0.7071066 }, 0xD832D478, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6653.922, -3214.05, 41.82556 }, { 0, 0, 0.7071066, 0.7071066 }, 0xEAE3F9DA, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6572.271, -3132.585, 33.65134 }, { 0, 0, 0, 1 }, 0x05C16EA3, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6883.753, -3051.31, 29.90527 }, { 0, 0, 0.9999987, -0.00161786 }, 0x812AC45A, { 0xF31C1367 }, 0x5EA26055 },
{ { 6933.934, -3137.629, 12.70199 }, { 0, 0, 0, 1 }, 0xD0817BE7, { 0xF31C1367 }, 0x5EA26055 },
{ { 6554.016, -3096.51, 34.65341 }, { 0, 0, 0, 1 }, 0x0D38D6DC, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6555.265, -3093.609, 41.9928 }, { 0, 0, 0.7071066, 0.7071066 }, 0x31199366, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6502.511, -3331.255, 39.18538 }, { 0, 0, -0.7071066, 0.7071066 }, 0x1B76AD69, { 0xD62C5988 }, 0x4B07B920 },
{ { 6632.662, -3161.455, 39.72255 }, { 0, 0, 9E-08, 1 }, 0x331E05DE, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6598.056, -3291.978, 24.73091 }, { 0, 0, 0.5082529, 0.8612078 }, 0x59B08AE8, { 0xD62C5988 }, 0x4B07B920 },
{ { 6609.978, -3240.177, 31.61117 }, { 0, -0.01142569, 0, 0.9999348 }, 0xA1D7D9E9, { 0xD62C5988 }, 0x4B07B920 },
{ { 6783.703, -3084.843, 22.7584 }, { -0.01428988, 0.01567888, 0.7389202, 0.6734591 }, 0x11A1EEAC, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6574.956, -3037.739, 37.45633 }, { 0, 0, 0, 1 }, 0x01488936, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6690.176, -3160.757, 22.71637 }, { 0, 0, 0, 1 }, 0x8D60ABFA, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6695.053, -3170.936, 25.64265 }, { 0.00671084, 0.03820614, 0.904884, 0.423887 }, 0xF7CD6C54, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6702.871, -3085.83, 25.18456 }, { -0.01215299, -0.04251638, 0.662253, 0.7479743 }, 0xB835F75D, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6592.092, -3269.585, 26.51835 }, { 0, 0, 0.7071066, 0.7071066 }, 0x6AF66210, { 0xD62C5988 }, 0x4B07B920 },
{ { 6568.315, -3016.582, 45.4044 }, { 0, 0, 0.7071066, 0.7071066 }, 0x3F31E69C, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6560.212, -3038.94, 46.79666 }, { 0, 0, 2.4E-07, 1 }, 0xDE32D210, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6560.576, -3046.333, 46.78376 }, { 0, 0, 1, -1.9E-07 }, 0x918C38C4, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6574.8, -3016.326, 45.3753 }, { 0, 0, 1, -3.1E-07 }, 0x83159BD7, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6568.801, -3036.108, 47.53762 }, { 0.07149108, -2.000001E-08, -2.4E-07, 0.9974414 }, 0x955CC065, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6573.929, -3025.37, 45.4044 }, { 0, 0, 0, 1 }, 0xA60CDBFF, { 0x4A5B41E0 }, 0x19BA5686 },
{ { 6603.942, -3276.082, 23.96506 }, { 0, 0, -0.6315712, 0.7753179 }, 0x0AE50299, { 0xD62C5988 }, 0x4B07B920 },
{ { 6862.888, -3030.515, 24.94315 }, { 0, 0, 0, 1 }, 0xE9ABD145, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6816.452, -3013.221, 23.03736 }, { 0, 0, 0, 1 }, 0x4B081BE5, { 0x047EB62C }, 0x3C9E1C4D },
{ { 7009.976, -3094.076, 29.42437 }, { 0, 0, -0.7071068, 0.7071066 }, 0x6C5492FE, { 0x2F7A499E }, 0x73AC254D },
{ { 6935.064, -3137.347, 24.29202 }, { 0, 0, 0, 1 }, 0xC5F04688, { 0x2F7A499E }, 0x73AC254D },
{ { 6713.981, -3200.49, 33.11599 }, { 0, 0, -0.3826833, 0.9238795 }, 0xD3B5E213, { 0x2F7A499E }, 0x73AC254D },
{ { 6733.707, -3065.004, 34.27964 }, { 0, 0, 0, 1 }, 0x7E563701, { 0x2F7A499E }, 0x73AC254D },
{ { 6839.631, -3039.949, 26.06766 }, { 0, 0, 0, 1 }, 0x4F21084B, { 0x047EB62C }, 0x3C9E1C4D },
{ { 6045.306, -2996.027, 0 }, { 0, 0, 0, 1 }, 0x4D05216A, { 0xA2E6239C }, 0x6C8BA378 },
{ { 5952.931, -3104.924, 21.22548 }, { 0, 0, 0, 1 }, 0x79B9FAD3, { 0xA2E6239C }, 0x6C8BA378 },
{ { 6011.255, -2883.51, 32.47707 }, { 0, 0, 0, 1 }, 0x0C419FC4, { 0xA2E6239C }, 0x6C8BA378 },
{ { 6160.553, -3180.465, 25.76432 }, { 0, 0, 0, 1 }, 0x5ED4C509, { 0xA2E6239C }, 0x6C8BA378 },
{ { 6373.075, -3199.305, 58.37415 }, { 0, 0, 0, 1 }, 0xB0A368A5, { 0xA2E6239C }, 0x6C8BA378 },
{ { 6402.446, -3013.569, 38.58034 }, { 0, 0, 0, 1 }, 0x42590C12, { 0xA2E6239C }, 0x6C8BA378 },
{ { 6185.601, -2995.576, 49.58417 }, { 0, 0, 0, 1 }, 0x87FC9758, { 0xA2E6239C }, 0x6C8BA378 },
{ { 6216.203, -3187.426, 30.23683 }, { 0, 0, 0, 1 }, 0xA0C2C8E4, { 0xA2E6239C }, 0x6C8BA378 },
{ { 6163.581, -2848.707, 25.64844 }, { 0, 0, 0, 1 }, 0xEC00DF5F, { 0xA2E6239C }, 0x6C8BA378 },
{ { 5844.055, -3008.427, 57.67042 }, { 0, 0, -9E-08, 1 }, 0x2091CBCC, { 0xA2E6239C }, 0x6C8BA378 },
{ { 5519.762, -3008.106, 57.67042 }, { 0, 0, -9E-08, 1 }, 0xEDD66656, { 0xA2E6239C }, 0x6C8BA378 },
{ { 5497.197, -2995.34, 34.31346 }, { 0, 0, 0, 1 }, 0x8BBC5D0F, { 0xD1068BDA }, 0xE26CB391 },
{ { 5891.375, -3141.524, 8.502241 }, { 0, 0, 0, 1 }, 0xC6DE3E87, { 0x855250D8 }, 0x701B8FD4 },
{ { 5876.969, -3105.737, 3.028085 }, { 0, 0, 0, 1 }, 0x9608434E, { 0x855250D8 }, 0x701B8FD4 },
{ { 5916.845, -3041.604, -0.2307365 }, { 0, 0, 0, 1 }, 0x7E77FB6C, { 0x855250D8 }, 0x701B8FD4 },
{ { 6154.352, -3281.352, 26.26579 }, { 0, 0, 0, 1 }, 0x148BFC29, { 0x2DC721C3 }, 0xA107F1AC },
{ { 6070.607, -2987.257, 34.06645 }, { 0, 0, 0, 1 }, 0xB8EACED3, { 0x1F6D8510 }, 0x92DA5551 },
{ { 6361.849, -3065.363, 33.9929 }, { 0, 0, 0.7071066, 0.7071066 }, 0x3B853C5D, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6440.488, -3001.304, 36.19331 }, { 0, 0, 0.7071066, 0.7071066 }, 0x3A237B54, { 0xEC781F26 }, 0xC483B8A7 },
{ { 5966.75, -2820.159, 10.05421 }, { 0, 0, 0.9914448, 0.1305262 }, 0xDFDD7866, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6383.874, -3079.544, 56.55863 }, { 0, 0, 0, 1 }, 0x605394DD, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6406.655, -3075.109, 42.44882 }, { 0, 0, 0, 1 }, 0xBEE756C2, { 0xEC781F26 }, 0xC483B8A7 },
{ { 5891.903, -3134.214, 4.957559 }, { 0, 0, 0, 1 }, 0x827D9555, { 0x855250D8 }, 0x701B8FD4 },
{ { 5947.27, -3022.757, 3.201272 }, { 0, 0, 0, 1 }, 0xE431828E, { 0x1F6D8510 }, 0x92DA5551 },
{ { 6182, -2924.718, 33.94695 }, { 0, 0, 0.9839528, -0.1784293 }, 0x151047CD, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6339.038, -2965.892, 29.41158 }, { 0, 0, 0.7071066, 0.7071066 }, 0x4CD30ACA, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6258.653, -2978.997, 26.05053 }, { 0, 0, 0.3502074, 0.9366722 }, 0xF85C3C0A, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6406.467, -2963.187, 31.37605 }, { 0, 0, -0.5842634, 0.8115641 }, 0xF373965F, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6380.068, -2972.015, 29.37011 }, { 0, 0, 0.7913647, 0.6113443 }, 0x420080D4, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6433.94, -3017.788, 46.6808 }, { 0, 0, 1, -8E-08 }, 0xFB8C8C78, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6445.658, -3043.787, 41.14429 }, { 0, 0, 0.7071066, 0.7071066 }, 0x4423A1E8, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6340.993, -3077.828, 36.86311 }, { 0, 0, 0.7383088, 0.6744629 }, 0xE8D8F021, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6376.621, -3020.71, 33.40949 }, { 0, 0, -0.5787015, 0.8155393 }, 0x8754956D, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6339.989, -3052.156, 30.84464 }, { 0, 0, 0.7961095, 0.6051526 }, 0x5280ABDA, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6032.108, -2834.843, 24.71286 }, { 0, 0, 0.4770302, 0.878887 }, 0xDAB34B5C, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6107.796, -2816.186, 31.83636 }, { 0, 0, 0, 1 }, 0x625DF8FC, { 0x93986D64 }, 0x7D4E2A39 },
{ { 5988.054, -3117.925, 27.80249 }, { 0, 0, 0.9659258, 0.2588191 }, 0x3A31D1A4, { 0x855250D8 }, 0x701B8FD4 },
{ { 6309.968, -2967.245, 29.21569 }, { 0, 0, 0.9538347, -0.300332 }, 0x31A71B9D, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6442.381, -2999.975, 29.4753 }, { 0, 0, 0.8852267, 0.4651598 }, 0x7C533706, { 0xEC781F26 }, 0xC483B8A7 },
{ { 5940.257, -3063.099, 14.83889 }, { 0, 0, 0.1305262, 0.9914448 }, 0xDB76912C, { 0x855250D8 }, 0x701B8FD4 },
{ { 5939.699, -3111.368, 5.000001 }, { 0, 0, 0.1305262, 0.9914448 }, 0x7DB855AD, { 0x855250D8 }, 0x701B8FD4 },
{ { 6397.358, -3256.992, 44.41633 }, { 0, 0, 0, 1 }, 0x20C3F3FA, { 0x3E40C2B6 }, 0xB64D1C3A },
{ { 6397.288, -3263.956, 36.51048 }, { 0, 0, 1, -8E-08 }, 0x25215270, { 0x3E40C2B6 }, 0xB64D1C3A },
{ { 6272.026, -3182.102, 35.61032 }, { 0, 0, 0, 1 }, 0x9A2CD396, { 0x3E40C2B6 }, 0xB64D1C3A },
{ { 6154.243, -3196.867, 35.52623 }, { 0, 0, 0, 1 }, 0x521185EE, { 0x2DC721C3 }, 0xA107F1AC },
{ { 6154.092, -3113.314, 33.47347 }, { 0, 0, 0, 1 }, 0x205BB727, { 0xCD10604F }, 0xD0F4517C },
{ { 6140.159, -3033.279, 32.41503 }, { 0, 0, 0, 1 }, 0x7682AFBF, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6242.124, -2930.561, 39.70878 }, { 0, 0, 0, 1 }, 0x357A2F3D, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6158.069, -2956.46, 37.32343 }, { 0, 0, 0, 1 }, 0xA28A1392, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6442.597, -3217.578, 41.87942 }, { -4E-08, 0, 0, 1 }, 0x84A381B5, { 0x3E40C2B6 }, 0xB64D1C3A },
{ { 5869.261, -3078.818, -3.458731 }, { 0, 0, 0, 1 }, 0xA7B9048B, { 0x855250D8 }, 0x701B8FD4 },
{ { 5917.717, -3022.025, -0.6914198 }, { 0, 0, 0, 1 }, 0xCA3ED4E3, { 0x855250D8 }, 0x701B8FD4 },
{ { 6242.529, -3202.282, 70.88202 }, { 0, 0, 0, 1 }, 0x717EDB04, { 0x2DC721C3 }, 0xA107F1AC },
{ { 6025.147, -2880.515, 7.264769 }, { 0, 0, 0, 1 }, 0xBD6FAB7E, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6022.655, -3035.008, 13.36935 }, { 0, 0, 0, 1 }, 0xCA898EC0, { 0x855250D8 }, 0x701B8FD4 },
{ { 6437.072, -3210.549, 50.73926 }, { 0, 0, 0, 1 }, 0x1272F2D9, { 0xF26F98F0 }, 0x4FB04CE9 },
{ { 6432.126, -3183.776, 46.94201 }, { 0, 0, 0.7071068, 0.7071066 }, 0xEC127A93, { 0x3E40C2B6 }, 0xB64D1C3A },
{ { 6134.925, -2815.09, 16.14185 }, { 0, 0, 0, 1 }, 0x0712C6F2, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6110.664, -3248.577, 31.43955 }, { 0, 0, 0.7071066, 0.7071068 }, 0xFBFD9756, { 0x2DC721C3 }, 0xA107F1AC },
{ { 6050.99, -2870.795, 10.84429 }, { 0, 0, 0.4114058, 0.9114523 }, 0x5AEAD6B6, { 0x93986D64 }, 0x7D4E2A39 },
{ { 5971.803, -2832.573, 14.05688 }, { 0, 0, 0.9217741, -0.3877274 }, 0x81A5755A, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6458.106, -2967.18, 34.04034 }, { 0, 0, -0.686425, 0.7272006 }, 0x9E0A331D, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6286.169, -2945.245, 34.83337 }, { 0, 0, 0.0002847901, 1 }, 0x3A856652, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6284.804, -3060.544, 35.48824 }, { 0, 0, 0.0002847901, 1 }, 0x405914E3, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6117.937, -2993.95, 48.43416 }, { 0, 0, 0.7316889, 0.6816388 }, 0xDE1AA531, { 0x1F6D8510 }, 0x92DA5551 },
{ { 5973.528, -2996.298, 14.55463 }, { 0, 0, 0.716302, 0.6977904 }, 0x5E838853, { 0xF26F98F0 }, 0x4FB04CE9 },
{ { 6132.828, -2874.608, 35.82541 }, { 0, 0, 0.5628048, 0.8265898 }, 0xD5131E57, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6257.368, -3120.689, 40.88527 }, { 0, 0, 0, 1 }, 0xD89606D0, { 0x2DC721C3 }, 0xA107F1AC },
{ { 5961.28, -2930.631, 5.038091 }, { 0, 0, 0.9988484, 0.04797802 }, 0x7DDBBFBA, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6242.946, -2933.024, 32.67621 }, { 0, 0, -0.8433914, 0.5372997 }, 0xAB82D320, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6338.503, -3046.939, 45.73483 }, { 0, 0, 0.7933533, 0.6087614 }, 0xEEE674B5, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6468.149, -2995.973, 42.6963 }, { 0, 0, 0, 1 }, 0x44292821, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6089.248, -3136.411, 44.68593 }, { 0, 0, 1, 8E-08 }, 0x8FCB816D, { 0x2DC721C3 }, 0xA107F1AC },
{ { 5925.919, -2850.595, 10.28935 }, { 0, 0, 0.8710198, 0.4912479 }, 0xACC7BCCA, { 0x93986D64 }, 0x7D4E2A39 },
{ { 5952.14, -2927.125, 2.896209 }, { -0.4999993, 0.5000007, 0.5000007, -0.4999993 }, 0x1990FB51, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6309.901, -3127.91, 62.64799 }, { 0, 0, 0.9659259, 0.258819 }, 0xB1CF9FFF, { 0x3E40C2B6 }, 0xB64D1C3A },
{ { 6392.219, -2964.239, 45.30864 }, { 0, 0, 0.7933533, 0.6087614 }, 0x0991C328, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6250.049, -3110.759, 111.3523 }, { 0, 0, 0, 1 }, 0xD13FAE66, { 0x2DC721C3 }, 0xA107F1AC },
{ { 5541.136, -2995.072, 41.08929 }, { 0, 0, 0, 1 }, 0xDB25D61A, { 0xD1068BDA }, 0xE26CB391 },
{ { 6268.6, -2922.709, 41.14613 }, { 0, 0, 0, 1 }, 0x568FC7BC, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6201.952, -2943.191, 39.10192 }, { 0, 0, 0, 1 }, 0xB5D685FC, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6113.286, -2968.564, 36.40748 }, { 0, 0, 0, 1 }, 0x7CDEFDFF, { 0x1F6D8510 }, 0x92DA5551 },
{ { 5891.489, -3158.52, 7.843666 }, { 0, 0, 0.1478094, 0.9890159 }, 0xEDD98754, { 0x855250D8 }, 0x701B8FD4 },
{ { 5896.641, -3137.672, 7.84321 }, { 0, 0, -0.5985325, 0.8010985 }, 0xA5C9A485, { 0x855250D8 }, 0x701B8FD4 },
{ { 6018.154, -3070.329, 32.15261 }, { 0, 0, -0.258819, 0.9659259 }, 0x2BDEE589, { 0x855250D8 }, 0x701B8FD4 },
{ { 5980.5, -3134.139, 25.99464 }, { 0, 0, -0.258819, 0.9659259 }, 0x2DF5B511, { 0x855250D8 }, 0x701B8FD4 },
{ { 5979.103, -2937.434, 26.29316 }, { 0, 0, 0.1918143, 0.9814313 }, 0x0CBA832F, { 0x93986D64 }, 0x7D4E2A39 },
{ { 5978.795, -2951.395, 7.771504 }, { 0, 0, -9E-08, 1 }, 0x3EB41495, { 0x93986D64 }, 0x7D4E2A39 },
{ { 5998.937, -2924.524, 7.568501 }, { 0, 0, -0.7071066, 0.7071066 }, 0x6B388DE6, { 0x93986D64 }, 0x7D4E2A39 },
{ { 6390.575, -3199.547, 34.64493 }, { 0, 0, 1, 8E-08 }, 0x97C971BA, { 0x3E40C2B6 }, 0xB64D1C3A },
{ { 5933.012, -3102.239, 22.28027 }, { 0, 0, 0.1736482, 0.9848077 }, 0x689F390A, { 0x855250D8 }, 0x701B8FD4 },
{ { 6166.171, -2841.832, 44.12441 }, { 0, 0, -0.8224629, 0.5688187 }, 0x00976F91, { 0x9ADAFBE5 }, 0xE63F7C12 },
{ { 6225.258, -3002.293, 45.31512 }, { 0, 0, 0.02067852, 0.9997861 }, 0x052EF171, { 0x7B35BC9B }, 0xC31AB5C9 },
{ { 6333.819, -3134.224, 62.04315 }, { 0, 0, 0, 1 }, 0xD91E293E, { 0x3E40C2B6 }, 0xB64D1C3A },
{ { 6411.744, -3047.594, 67.39819 }, { 0, 0, 0, 1 }, 0x4C89DC7A, { 0xEC781F26 }, 0xC483B8A7 },
{ { 6002.438, -3161.468, 27.923 }, { 0, 0, 0, 1 }, 0x3ED312F5, { 0x855250D8 }, 0x701B8FD4 },
{ { 6188.748, -3235.421, 94.34101 }, { 0, 0, 0, 1 }, 0x3F1AC83B, { 0xF26F98F0 }, 0x4FB04CE9 },
{ { 5889.918, -3133.909, 9.890995 }, { 0, 0, 0, 1 }, 0x9ED99E9F, { 0x61AA80FB }, 0x2C21115D },
{ { 6388.379, -3275.632, 42.53461 }, { 0, 0, 0, 1 }, 0x63C5DE24, { 0x61AA80FB }, 0x2C21115D },
{ { 6399.156, -3044.647, 36.5941 }, { 0, 0, 0, 1 }, 0x6E471563, { 0x61AA80FB }, 0x2C21115D },
{ { 6389.746, -3153.069, 42.6578 }, { 0, 0, 0, 1 }, 0x898658BB, { 0x61AA80FB }, 0x2C21115D },
{ { 6224.533, -3094.468, 39.85481 }, { 0, 0, 0, 1 }, 0xB874D86E, { 0x61AA80FB }, 0x2C21115D },
{ { 6185.229, -3191.263, 32.80582 }, { 0, 0, 0, 1 }, 0x6DD95541, { 0x61AA80FB }, 0x2C21115D },
{ { 6104.441, -3074.637, 39.50066 }, { 0, 0, 0, 1 }, 0xEFE860F2, { 0x61AA80FB }, 0x2C21115D },
{ { 6080.984, -3182.362, 38.25229 }, { 0, 0, 0, 1 }, 0xEB166D79, { 0x61AA80FB }, 0x2C21115D },
{ { 6084.956, -3282.638, 33.56238 }, { 0, 0, 0, 1 }, 0xFB8499DE, { 0x61AA80FB }, 0x2C21115D },
{ { 6372.274, -3436.768, 57.97428 }, { 0, 0, 0, 1 }, 0xA266CA87, { 0x53A101C9 }, 0x6A43712A },
{ { 6573.002, -3439.064, 16.86536 }, { 0, 0, 0, 1 }, 0x694C5857, { 0x53A101C9 }, 0x6A43712A },
{ { 6534.871, -3682.196, 49.40482 }, { 0, 0, 0, 1 }, 0xA914AD28, { 0x53A101C9 }, 0x6A43712A },
{ { 6604.907, -3711.749, 8.883163 }, { 0, 0, 0, 1 }, 0x747904FE, { 0x53A101C9 }, 0x6A43712A },
{ { 6499.683, -3997.715, 16.4045 }, { 0, 0, 0, 1 }, 0x6CAAD811, { 0x53A101C9 }, 0x6A43712A },
{ { 6622.666, -3516.414, 20.04363 }, { 0, 0, -0.3034413, 0.9528502 }, 0xC286DD91, { 0xEFD0529F }, 0xD2F68C1C },
{ { 6424.701, -3597.134, 22.15859 }, { 0, 0, 0, 1 }, 0x6A55CB80, { 0x8ADCAC4E }, 0x8638038B },
{ { 6497.875, -3617.413, 26.6554 }, { 0, 0, -0.4617488, 0.8870109 }, 0xE44B721A, { 0x8ADCAC4E }, 0x8638038B },
{ { 6356.579, -3341.929, 84.35938 }, { 0, 0, -0.7071068, 0.7071066 }, 0x4F2A67FB, { 0xDD992685 }, 0xCF400714 },
{ { 6435.874, -3501.055, 97.73779 }, { 0, 0, 0, 1 }, 0x0C8F8482, { 0xDD992685 }, 0xCF400714 },
{ { 6373.091, -3420.013, 49.1867 }, { 0, 0, 0, 1 }, 0xAD96B56D, { 0xDD992685 }, 0xCF400714 },
{ { 6333.038, -3383.246, 90.86856 }, { 0, 0, -0.7071068, 0.7071066 }, 0xBFD90C78, { 0xDD992685 }, 0xCF400714 },
{ { 6524.098, -3333.095, 37.90484 }, { 0, 0, 0, 1 }, 0x7B90D60F, { 0xDD992685 }, 0xCF400714 },
{ { 6544.741, -3424.723, 42.67496 }, { 0, 0, 0, 1 }, 0xC1722B93, { 0xDD992685 }, 0xCF400714 },
{ { 6544.741, -3438.688, 42.67496 }, { 0, 0, 0, 1 }, 0xD35ACF64, { 0xDD992685 }, 0xCF400714 },
{ { 6544.741, -3452.698, 42.67496 }, { 0, 0, 0, 1 }, 0xDCFCE2A8, { 0xDD992685 }, 0xCF400714 },
{ { 6362.717, -3418.817, 57.04232 }, { 0, 0, 0, 1 }, 0x172DBB20, { 0xDD992685 }, 0xCF400714 },
{ { 6354.707, -3451.446, 52.34028 }, { 0, 0, 0, 1 }, 0xAA8DE1E2, { 0xDD992685 }, 0xCF400714 },
{ { 6373.315, -3512.136, 48.02834 }, { 0, 0, 0, 1 }, 0x9C4B455D, { 0xDD992685 }, 0xCF400714 },
{ { 6324.365, -3351.77, 46.98413 }, { 0, 0, -0.8336115, 0.5523512 }, 0x1A628B39, { 0x667DBFF0 }, 0xAB5E3CE8 },
{ { 6489.79, -3717.598, 28.83796 }, { 0, 0, -0.713512, 0.7006431 }, 0xCB53B98D, { 0xC2A05F58 }, 0x4C8A6184 },
{ { 6587.659, -3446.392, 34.72218 }, { 0, 0, 0.9853836, -0.1703502 }, 0x847A2BDB, { 0xC2A05F58 }, 0x4C8A6184 },
{ { 6679.14, -3596.033, 28.30984 }, { 0, 0, -0.1219408, 0.9925374 }, 0x71E706B5, { 0xC2A05F58 }, 0x4C8A6184 },
{ { 6573.364, -3638.914, 32.02115 }, { 0, 0, 0.3804433, 0.9248043 }, 0xA10CE500, { 0xC2A05F58 }, 0x4C8A6184 },
{ { 6453.325, -3596.257, 35.03165 }, { 0, 0, 0.7053909, 0.7088184 }, 0x9D20DD28, { 0xC2A05F58 }, 0x4C8A6184 },
{ { 6497.925, -3617.403, 32.70499 }, { 0, 0, -0.462575, 0.8865802 }, 0xB89E9423, { 0xC2A05F58 }, 0x4C8A6184 },
{ { 6451.789, -3385.053, 36.59227 }, { 0, 0, 3.3E-07, 1 }, 0xD79FD225, { 0xC2A05F58 }, 0x4C8A6184 },
{ { 6425.762, -3454.31, 43.7741 }, { 0, 0.1173135, 0, 0.9930949 }, 0x8812F133, { 0xF6ABDBBC }, 0x8E193821 },
{ { 6470.832, -3594.752, 22.02056 }, { 0, 0, 0.7071068, 0.7071066 }, 0x9A5495B6, { 0xF6ABDBBC }, 0x8E193821 },
{ { 6380.537, -3339.475, 38.45396 }, { -6E-08, 6E-08, 0.7071068, 0.7071066 }, 0x2279AD00, { 0x155B4849 }, 0x1AA6F240 },
{ { 6581.009, -3962.334, 5.470474 }, { 0, 0, 0.8833534, 0.4687079 }, 0x2F51265B, { 0xC2648ECB }, 0x9B8868B4 },
{ { 6441.029, -3842.926, 16.53653 }, { 0, 0, 0, 1 }, 0x913FF1FB, { 0x913FF1FB }, 0x570DAFD9 },
{ { 6566.431, -3644.928, 17.06603 }, { 0, 0, 0.3804433, 0.9248043 }, 0x87753969, { 0x8ADCAC4E }, 0x8638038B },
{ { 6490.562, -3708.451, 13.88283 }, { 0, 0, -0.713512, 0.7006431 }, 0xAA2FB66D, { 0x8ADCAC4E }, 0x8638038B },
{ { 6452.675, -3605.413, 20.07652 }, { 0, 0, 0.7053912, 0.7088182 }, 0x1F905430, { 0x8ADCAC4E }, 0x8638038B },
{ { 6587.353, -3446.564, 19.78897 }, { 0, 0, 0.9853836, -0.1703502 }, 0xAB57BBEB, { 0xEFD0529F }, 0xD2F68C1C },
{ { 6679.488, -3596.06, 13.37663 }, { 0, 0, -0.1219408, 0.9925374 }, 0xC241CC4A, { 0xAA780470 }, 0x0E6356B5 },
{ { 6571.248, -3768.888, 13.35924 }, { 0, 0, 0.9191448, 0.3939196 }, 0xED256565, { 0xAA780470 }, 0x0E6356B5 },
{ { 6291.913, -3327.631, 42.68024 }, { 0, 0, 0.4233778, 0.9059531 }, 0xDECB9F44, { 0x667DBFF0 }, 0xAB5E3CE8 },
{ { 6679.297, -3542.168, 18.43956 }, { 0, 0, 0.2202739, 0.975438 }, 0xAB4B3573, { 0xAA780470 }, 0x0E6356B5 },
{ { 6640.645, -3656.001, 38.55289 }, { 0, 0, 0.524445, 0.8514444 }, 0x0803F342, { 0xAA780470 }, 0x0E6356B5 },
{ { 6401.854, -3549.521, 43.30079 }, { 0, 0, -0.7001385, 0.7140072 }, 0x6E4E7C63, { 0xAA780470 }, 0x0E6356B5 },
{ { 6311.757, -3359.059, 40.2244 }, { 0, 0, 0.9972494, -0.07411981 }, 0x08C84C4F, { 0xAA780470 }, 0x0E6356B5 },
{ { 6480.971, -3830.216, 17.89827 }, { 0, 0, 0.7043356, 0.7098672 }, 0xBA126070, { 0xAA780470 }, 0x0E6356B5 },
{ { 6571.529, -3769.097, 28.29245 }, { 0, 0, 0.9191448, 0.3939196 }, 0xFA3A7C63, { 0xFA3A7C63 }, 0x50DFBB49 },
{ { 6579.528, -4157.023, 3.112297 }, { 0, 0, 0, 1 }, 0xBAABDAF5, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6535.952, -3507.462, 24.14505 }, { 0, 0, 0.938185, 0.3461343 }, 0x07D15AA8, { 0xEFD0529F }, 0xD2F68C1C },
{ { 6453.786, -3666.315, 17.17938 }, { 0, 0, 0.2797594, 0.9600702 }, 0x3B0B9D68, { 0x8ADCAC4E }, 0x8638038B },
{ { 6603.458, -3589.521, 24.715 }, { 0, 0, 0, 1 }, 0xDDCD639B, { 0x8ADCAC4E }, 0x8638038B },
{ { 6638.756, -3594.653, 13.71663 }, { 0, 0, 1, -8E-08 }, 0xBE444512, { 0x8ADCAC4E }, 0x8638038B },
{ { 6657.329, -3548.76, 13.66432 }, { 0, 0, 0.7071068, 0.7071066 }, 0x97833D94, { 0x8ADCAC4E }, 0x8638038B },
{ { 6579.497, -3751.218, 14.3797 }, { 0, 0, 0, 1 }, 0x9DE7E814, { 0xAA780470 }, 0x0E6356B5 },
{ { 6608.278, -3707.325, 14.3797 }, { -1E-08, 3E-08, 0.4338916, 0.9009651 }, 0x84723529, { 0xAA780470 }, 0x0E6356B5 },
{ { 6637.314, -3664.145, 14.36107 }, { 0, 0, 0.498415, 0.8669386 }, 0x66EE7A22, { 0xAA780470 }, 0x0E6356B5 },
{ { 6661.631, -3628.44, 14.33248 }, { -8.499999E-07, 5.3E-07, 0.4992729, 0.8664448 }, 0x5786DB4F, { 0xAA780470 }, 0x0E6356B5 },
{ { 6637.717, -3679.183, 14.35723 }, { 0, 0, 0.5852546, 0.8108495 }, 0x59ABDF9D, { 0xAA780470 }, 0x0E6356B5 },
{ { 6614.443, -3734.899, 14.39455 }, { -1.4E-07, 2.4E-07, 0.5010893, 0.8653956 }, 0x755696F2, { 0xAA780470 }, 0x0E6356B5 },
{ { 6579.666, -4119.545, 7.308514 }, { 0, 1.15E-06, 1, -1E-06 }, 0x9C232478, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6597.551, -4096.358, 7.812424 }, { -2E-08, 2.6E-07, 0.997941, -0.06413959 }, 0xABFDC42D, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6488.95, -3952.967, 8.179916 }, { 0, 0, -0.8571251, 0.5151083 }, 0xBE26E87F, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6491.759, -3978.183, 7.633801 }, { 0, 0, 0.7071069, 0.7071066 }, 0x0929FE84, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6552.958, -3946.177, 7.350819 }, { -4.700001E-07, 6.000001E-08, 0.7914301, 0.61126 }, 0xDA81A134, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6597.605, -4043.145, 8.088169 }, { 0, 0, 0.1088902, 0.9940538 }, 0x27DD3BEE, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6519.079, -4031.835, 8.020912 }, { 0, 0, 0.7230638, 0.6907812 }, 0xF92E5E8D, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6521.887, -4099.652, 8.038734 }, { 0, 0, 0, 1 }, 0xC51229EE, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6561.45, -4046.052, 7.771934 }, { 0, 0, 0, 1 }, 0xBB6C33D1, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6598.386, -3648.252, 21.88953 }, { 0, 0, 0, 1 }, 0x34D07142, { 0x8ADCAC4E }, 0x8638038B },
{ { 6627.566, -3615.449, 21.79064 }, { 0, 0, 0, 1 }, 0x5E155281, { 0x8ADCAC4E }, 0x8638038B },
{ { 6492.106, -3972.605, 9.702738 }, { 0, 0, 0, 1 }, 0x0F0FFF51, { 0x9B2BE324 }, 0x6C2BB558 },
{ { 6528.841, -3419.045, 24.04958 }, { 0.06245098, -0.05258712, 0.7644578, 0.6394832 }, 0x7C4A8CDA, { 0xEFD0529F }, 0xD2F68C1C },
{ { 6528.411, -3639.589, 26.59637 }, { 0, 0, 0, 1 }, 0x4C13597C, { 0x2F7A499E }, 0xF38EE16D },
{ { 6553.655, -3333.21, 29.51886 }, { 0, 0, 0, 1 }, 0xA9AB14AA, { 0x2F7A499E }, 0xF38EE16D },
{ { 6575.579, -3971.165, 17.97169 }, { 0, 0, 0, 1 }, 0xB769B027, { 0x2F7A499E }, 0xF38EE16D },
{ { 6465.02, -3971.453, 22.1801 }, { 0, 0, 0, 1 }, 0x753D2BCF, { 0x2F7A499E }, 0xF38EE16D },
{ { 6483.429, -3869.468, 22.46321 }, { 0, 0, 0, 1 }, 0x9AFC774D, { 0x2F7A499E }, 0xF38EE16D },
{ { 6411.048, -3696.936, 23.10625 }, { 0, 0, 0, 1 }, 0xEC821A57, { 0x2F7A499E }, 0xF38EE16D },
{ { 6539.7, -3761.695, 25.80811 }, { 0, 0, 0, 1 }, 0xF2BB26C9, { 0x2F7A499E }, 0xF38EE16D },
{ { 6514.891, -3399.264, 28.97423 }, { 0, 0, 0, 1 }, 0xC804515C, { 0x2F7A499E }, 0xF38EE16D },
{ { 6561.59, -3500.893, 25.30288 }, { 0, 0, 0, 1 }, 0xDE467DE0, { 0x2F7A499E }, 0xF38EE16D },
{ { 6392.15, -3490.745, 29.07874 }, { 0, 0, 0, 1 }, 0x23778841, { 0x2F7A499E }, 0xF38EE16D },
{ { 6403.526, -3405.702, 34.61955 }, { 0, 0, 0, 1 }, 0x7872A4CA, { 0x2F7A499E }, 0xF38EE16D },
{ { 6260.59, -3333.382, 41.60641 }, { 0, 0, 0, 1 }, 0x66400065, { 0x2F7A499E }, 0xF38EE16D },
{ { 6488.963, -4086.661, 19.93741 }, { 0, 0, 0, 1 }, 0xECAD0D3D, { 0x2F7A499E }, 0xF38EE16D },
{ { 6587.12, -4104.419, 18.12757 }, { 0, 0, 0, 1 }, 0x1A59E896, { 0x2F7A499E }, 0xF38EE16D },
{ { 6432.887, -3329.431, 34.29711 }, { 0, 0, 0, 1 }, 0x08BBC55A, { 0x2F7A499E }, 0xF38EE16D },
{ { 6208.023, -3540.015, 32.72419 }, { 0, 0, 0.7071066, 0.7071066 }, 0xF10B0222, { 0xF10B0222 }, 0xA056AE5B },
{ { 6199.6, -3395.384, 46.26645 }, { 0, 0, 0, 1 }, 0x0359A6BF, { 0x0359A6BF }, 0x70FDCFAE },
{ { 6156.545, -3662.127, 27.13748 }, { 0, 0, 0, 1 }, 0x481B3041, { 0x481B3041 }, 0xB7455C38 },
{ { 6254.187, -3714.702, 28.48577 }, { 0, 0, 0, 1 }, 0xDE0EDC32, { 0xDE0EDC32 }, 0x8E9B0AE4 },
{ { 6288.916, -3609.627, 27.93008 }, { 0, 0, 0, 1 }, 0xCC5938C7, { 0xCC5938C7 }, 0xC1DA7162 },
{ { 6226.685, -3603.476, 26.38605 }, { 0, 0, 0, 1 }, 0x419ADF2F, { 0x4E3C66DF }, 0x31EE5BFF },
{ { 6292.508, -3551.993, 27.52094 }, { 0, 0, 0, 1 }, 0x742FA443, { 0x4E3C66DF }, 0x31EE5BFF },
{ { 6314.165, -3744.332, 20.65366 }, { 0, 0, 0, 1 }, 0xD7958AE7, { 0x3BDCC220 }, 0x466F855D },
{ { 6354.971, -3702.904, 30.14434 }, { 0, 0, -9E-08, 1 }, 0xCFEF6D4C, { 0x3BDCC220 }, 0x466F855D },
{ { 6132.6, -3662.121, 41.353 }, { 0, 0, 0.9999982, -0.0018876 }, 0x4182AF65, { 0xCCFBAFC6 }, 0x24984A74 },
{ { 6311.98, -3569.95, 42.93526 }, { 0, 0, 0.9962848, -0.08611924 }, 0x69827F58, { 0xCCFBAFC6 }, 0x24984A74 },
{ { 6211.72, -3724.776, 22.98873 }, { 0, 0, 0, 1 }, 0x7A4A93E2, { 0x3BDCC220 }, 0x466F855D },
{ { 6345.046, -3806.029, 21.11191 }, { 0, 0, 0, 1 }, 0xC7803D66, { 0x3BDCC220 }, 0x466F855D },
{ { 6218.276, -3575.313, 20.46739 }, { 0, 0, 0, 1 }, 0xBCC8D58D, { 0x4E3C66DF }, 0x31EE5BFF },
{ { 6284.47, -3733.899, 21.1673 }, { 0, 0, 0, 1 }, 0xE6B09F5E, { 0x2F7A499E }, 0xF59F4982 },
{ { 6175.273, -3618.446, 21.03691 }, { 0, 0, 0, 1 }, 0xAF4C6A41, { 0x2F7A499E }, 0xF59F4982 },
{ { 6253.654, -3475.917, 30.6966 }, { 0, 0, 0, 1 }, 0xA199CEDC, { 0x2F7A499E }, 0xF59F4982 },
{ { 6133.203, -3417.166, 27.45043 }, { 0, 0, 0, 1 }, 0x4BC2232E, { 0x2F7A499E }, 0xF59F4982 },
{ { 6201.348, -3343.534, 35.4088 }, { 0, 0, 0, 1 }, 0x3DF48793, { 0x2F7A499E }, 0xF59F4982 },
{ { 6348.684, -3637.167, 20.70197 }, { 0, 0, 0, 1 }, 0xE23EB952, { 0x3BDCC220 }, 0x466F855D },
{ { 6370.956, -3626.412, 21.00214 }, { 0, 0, 0, 1 }, 0x9D6CF10D, { 0x3BDCC220 }, 0x466F855D },
{ { 6368.493, -3618.755, 27.1686 }, { 0, 0, 0, 1 }, 0xDAE1B39B, { 0x3BDCC220 }, 0x466F855D },
{ { 6075.291, -3543.971, 15.96271 }, { 0, 0, 0, 1 }, 0xD7ED4830, { 0x722C6861 }, 0x13FE09AE },
{ { 6000.491, -3207.729, 29.98804 }, { 0, 0, 0, 1 }, 0x21135A7B, { 0x722C6861 }, 0x13FE09AE },
{ { 5874.573, -3195.596, 20.42027 }, { 0, 0, 0, 1 }, 0xB46C812F, { 0x722C6861 }, 0x13FE09AE },
{ { 6032.67, -3417.646, 20.08467 }, { 0, 0, 0, 1 }, 0x0FC237D5, { 0x722C6861 }, 0x13FE09AE },
{ { 6117.468, -3892.497, 14.86152 }, { 0, 0, 0, 1 }, 0x12DABE0A, { 0x722C6861 }, 0x13FE09AE },
{ { 6014.214, -3855.696, 5.264253 }, { 0, 0, 0, 1 }, 0xF6BE011F, { 0x722C6861 }, 0x13FE09AE },
{ { 6060.196, -3951.165, 7.023767 }, { 0, 0, 0, 1 }, 0x7B4B0A53, { 0x722C6861 }, 0x13FE09AE },
{ { 5858.888, -3956.85, -4.226194 }, { 0, 0, 0, 1 }, 0xEECCF159, { 0x722C6861 }, 0x13FE09AE },
{ { 6256.803, -4089.863, -1.835454 }, { 0, 0, 0, 1 }, 0x6A5266EA, { 0x722C6861 }, 0x13FE09AE },
{ { 6390.945, -3890.666, 14.41051 }, { 0, 0, 0, 1 }, 0xF7E20815, { 0x722C6861 }, 0x13FE09AE },
{ { 5939.721, -3354.914, -5.315002 }, { 0, 0, 0, 1 }, 0xFD179284, { 0x722C6861 }, 0x13FE09AE },
{ { 5683.064, -3664.128, 38.48955 }, { 0, 0, 0, 1 }, 0x54C541E2, { 0x722C6861 }, 0x13FE09AE },
{ { 6050.405, -3718.07, 36.25895 }, { 0, 0, 0, 1 }, 0xCA30ACB7, { 0x722C6861 }, 0x13FE09AE },
{ { 6320.195, -3862.089, 24.54476 }, { 0, 0, 0, 1 }, 0x0184B13B, { 0xC206035F }, 0x4D2CC0C7 },
{ { 5985.505, -3641.592, 7.910797 }, { 0, 0, -0.548651, 0.8360515 }, 0xC0146082, { 0x34E5691C }, 0x278C757B },
{ { 6143.401, -3795.184, 20.79422 }, { 0, 0, 0, 1 }, 0xC62FCF37, { 0x34E5691C }, 0x278C757B },
{ { 5959.708, -3186.294, 5.074529 }, { 0, 0, -0.5948227, 0.8038569 }, 0x12D1B2DA, { 0xFE997C85 }, 0xCCE3402A },
{ { 5924.57, -3412.979, 4.818526 }, { 0, 0, -0.2164396, 0.976296 }, 0x808D452D, { 0xEE79DC46 }, 0xDF18E495 },
{ { 5870.358, -3248.377, 6.22204 }, { 0, 0, 0, 1 }, 0xD33950E5, { 0xFE997C85 }, 0xCCE3402A },
{ { 6239.905, -3837.037, 14.28779 }, { 0, 0, 1, -8E-08 }, 0xC03ECE6D, { 0xDCB838C3 }, 0x4373AD55 },
{ { 5950.543, -3943.751, 0.0779914 }, { 0, 0, 0, 1 }, 0xC2DF178F, { 0x2B034267 }, 0xE383165E },
{ { 6203.254, -3940.983, 14.77224 }, { 0, 0, 0, 1 }, 0xEF2C13E1, { 0xDCB838C3 }, 0x4373AD55 },
{ { 6183.083, -3925.484, 15.51663 }, { 0, 0, 0, 1 }, 0xA20E0EC5, { 0xDCB838C3 }, 0x4373AD55 },
{ { 6043.552, -3450.588, 9.76615 }, { 0, 0, 0.3826833, 0.9238795 }, 0x5750862B, { 0xB83BEFCB }, 0xA7E5F638 },
{ { 5902, -3538.271, 8.909016 }, { 0, 0, -0.2095377, 0.9778005 }, 0x3E5CB04B, { 0x1B1E358E }, 0x88EF384B },
{ { 5896.5, -3548.95, 8.906505 }, { 0, 0, -0.2095377, 0.9778005 }, 0x4BBDE2FC, { 0x1B1E358E }, 0x88EF384B },
{ { 7278.105, -2567.734, 14.26041 }, { 0, 0, 0, 1 }, 0x4A9752F7, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7583.261, -3221.515, 23.57256 }, { 0, 0, 0, 1 }, 0xB9A339ED, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7269.77, -3033.217, 18.22677 }, { 0, 0, 0, 1 }, 0x75A65300, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7209.374, -2762.993, 29.67193 }, { 0, 0, 0, 1 }, 0x6773C962, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7365.203, -2863.053, 20.58787 }, { 0, 0, 0, 1 }, 0x71121053, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7830.486, -2688.569, 9.266144 }, { 0, 0, 0, 1 }, 0x1EAECEBB, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7810.287, -2949.015, 41.12674 }, { 0, 0, 0, 1 }, 0x424AE92A, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7187.203, -2308.056, 9.850566 }, { 0, 0, 0, 1 }, 0x235DDCED, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7482.824, -2419.626, 8.762216 }, { 0, 0, 0, 1 }, 0x34C77FC0, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7695.507, -2504.554, -3.208409 }, { 0, 0, 0, 1 }, 0xC2761B23, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7470.708, -2657.488, 16.1649 }, { 0, 0, 0, 1 }, 0xCD8C314F, { 0x868CBF9D }, 0xBF8E42CB },
{ { 7509.101, -3225.31, 28.58679 }, { 0, 0, 0, 1 }, 0xDD9DBB21, { 0x0063B10D }, 0xE3AEA594 },
{ { 7551.851, -3004.323, 5.313016 }, { 0, 0, 0, 1 }, 0x707FD0EA, { 0x0063B10D }, 0xE3AEA594 },
{ { 7556.27, -2617.486, 4.999686 }, { 0, 0, -0.4278296, 0.9038594 }, 0xF72A5CD8, { 0xB60950ED }, 0x90CDB629 },
{ { 7064.493, -3149.218, 21.70865 }, { 0, 0, 0, 1 }, 0xE1BD04CB, { 0x0063B10D }, 0xE3AEA594 },
{ { 7269.93, -2946.855, 23.41623 }, { 0, 0, 0.6819984, 0.7313538 }, 0x632F87B2, { 0x0063B10D }, 0xE3AEA594 },
{ { 7269.176, -2777.96, 26.7083 }, { 0, 0, 0.7253745, 0.6883546 }, 0x70F5233D, { 0x756D27BA }, 0x13B4622E },
{ { 7366.064, -2861.323, 9.883779 }, { 0, 0, 0, 1 }, 0x83FB4949, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7235.658, -2994.209, 9.64864 }, { 0, 0, 0, 1 }, 0x4D76DC4D, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7294.743, -2997.261, 10.21823 }, { 0, 0, 0, 1 }, 0xCCC7B39B, { 0x0063B10D }, 0xE3AEA594 },
{ { 7371.6, -2922.141, 10.23402 }, { 0, 0, -0.7071066, 0.7071066 }, 0x19604CCB, { 0x0063B10D }, 0xE3AEA594 },
{ { 7506.156, -2728.749, 10.23402 }, { 0, 0, 0.8386705, 0.5446391 }, 0x27A5E956, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7271.556, -2911.152, 24.07161 }, { 0, 0, 0, 1 }, 0xA59BE53C, { 0x0063B10D }, 0xE3AEA594 },
{ { 7168.39, -3124.248, 22.65417 }, { 0, 0, -0.5, 0.8660254 }, 0x766206C9, { 0x0063B10D }, 0xE3AEA594 },
{ { 7265.284, -2731.183, 27.92605 }, { 0, 0, 0.04361938, 0.9990483 }, 0x847AA2FA, { 0x756D27BA }, 0x13B4622E },
{ { 7191.592, -2454.116, 13.53202 }, { 0, 0, 0.2923717, 0.9563047 }, 0xF2AE7F68, { 0x756D27BA }, 0x13B4622E },
{ { 7226.144, -2526.255, 16.37838 }, { 0, 0, 0, 1 }, 0xE27FDF37, { 0x756D27BA }, 0x13B4622E },
{ { 7503.981, -2959.252, 8.474731 }, { 0, 0, 0, 1 }, 0xC4893AC5, { 0x0063B10D }, 0xE3AEA594 },
{ { 7354.155, -3057.567, 8.393894 }, { 0, 0, -0.668488, 0.7437229 }, 0xD69DDEEE, { 0x0063B10D }, 0xE3AEA594 },
{ { 7269.702, -2952.521, 22.07868 }, { 0, 0, -0.01745219, 0.9998477 }, 0xE5E07D73, { 0x0063B10D }, 0xE3AEA594 },
{ { 7270.06, -2941.329, 22.07868 }, { 0, 0, 1, -8E-08 }, 0xB56C9C94, { 0x0063B10D }, 0xE3AEA594 },
{ { 7228.003, -2707.221, 20.03396 }, { 0, 1.2E-07, 1, -8E-08 }, 0xBF36B028, { 0x756D27BA }, 0x13B4622E },
{ { 7246.527, -2593.448, 18.27843 }, { 1.2E-07, 2E-08, 0.1305261, 0.9914449 }, 0xD2C8574B, { 0x756D27BA }, 0x13B4622E },
{ { 7246.06, -2591.717, 18.27843 }, { -2E-08, 1.2E-07, 0.9914448, -0.1305264 }, 0xDC8D6AD5, { 0x756D27BA }, 0x13B4622E },
{ { 7080.37, -2649.47, 22.98053 }, { -1.2E-07, 0, -1.5E-07, 1 }, 0xE8628283, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7102.77, -2530.353, 25.8921 }, { 8E-08, -8E-08, -0.7071337, 0.7070798 }, 0xFA60267E, { 0x756D27BA }, 0x13B4622E },
{ { 7079.97, -2550.523, 24.89079 }, { 0, 0, 0, 1 }, 0xE7EC5308, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7104.057, -2716.517, 19.11484 }, { 0, 0, 0.5455465, 0.8380806 }, 0x3E2AFF84, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7082.528, -2723.181, 19.61402 }, { 0, 0, 0.7078717, 0.7063411 }, 0x0C6C1C07, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7227.956, -2775.668, 18.71515 }, { 0, 0, 0.9999994, -0.00108234 }, 0xAA41D754, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7269.276, -2780.142, 23.30021 }, { 0, 0, 0.9993908, -0.03489934 }, 0xBC04FADA, { 0x0063B10D }, 0xE3AEA594 },
{ { 7269.276, -2784.65, 22.79636 }, { 0, 0, 0.03489942, 0.9993908 }, 0xF740F1B1, { 0x0063B10D }, 0xE3AEA594 },
{ { 7224.279, -2973.445, 6.235846 }, { 0, 0, 0.7253746, 0.6883544 }, 0xD5C32EB6, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7356.599, -3038.005, 6.430562 }, { 0, 0, -0.6156614, 0.7880107 }, 0x882F132B, { 0x0063B10D }, 0xE3AEA594 },
{ { 7380.955, -3028.266, 6.47578 }, { 0, 0, 0.9123141, -0.4094913 }, 0xA5E74E9B, { 0x0063B10D }, 0xE3AEA594 },
{ { 7361.073, -2912.286, 6.728209 }, { 0, 0, 0.9996574, 0.02617684 }, 0x2202C3BC, { 0x0063B10D }, 0xE3AEA594 },
{ { 7361.028, -2745.083, 6.773446 }, { 0, 0, 0.9563047, 0.2923718 }, 0x13B92729, { 0x756D27BA }, 0x13B4622E },
{ { 7343.939, -2837.262, 25.1555 }, { 0, 0, 0.9238796, 0.3826834 }, 0x8F34B073, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7337.938, -2994.769, 12.54017 }, { 0, 0, 0, 1 }, 0x69510EEC, { 0x0063B10D }, 0xE3AEA594 },
{ { 7156.663, -2518.945, 18.6525 }, { 0, 0, 0, 1 }, 0xA8F72BA2, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7078.834, -2474.214, 18.76688 }, { 0, 0, 0, 1 }, 0x4E04E28E, { 0x756D27BA }, 0x13B4622E },
{ { 7531.005, -2737.187, 5.04559 }, { 0, 0, -0.7184569, 0.6955715 }, 0x93B6CF2D, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7527.684, -2981.837, 5.057256 }, { 0, 0, -0.5440186, 0.8390731 }, 0xA0436846, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7347.145, -2806.413, 27.39293 }, { 0, 0, 0, 1 }, 0xDB809B84, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7207.32, -2991.063, 11.19591 }, { 0, 0, 0, 1 }, 0x52556EE3, { 0x0063B10D }, 0xE3AEA594 },
{ { 7551.902, -2622.511, 8.711009 }, { 0, 0, 0.9414728, -0.3370894 }, 0xFB36394C, { 0xB60950ED }, 0x90CDB629 },
{ { 7505.652, -2867.383, 8.068386 }, { 0, 0, -0.7071068, 0.7071066 }, 0xC13F7359, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7532.054, -2738.855, 6.7679 }, { 0, 0, 0, 1 }, 0x2C37C4E9, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7529.369, -2985.635, 6.7679 }, { 0, 0, 0.2438103, 0.9698231 }, 0x942614C8, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7724.703, -3069.091, 5.716024 }, { 0, 0, 0, 1 }, 0x31CA1AF2, { 0x0063B10D }, 0xE3AEA594 },
{ { 8034.25, -2776.163, -2.03906 }, { 0, 0, -0.352631, 0.9357624 }, 0xDB5EC61F, { 0xB60950ED }, 0x90CDB629 },
{ { 8028.653, -2771.137, -2.03906 }, { 0, 0, -0.352631, 0.9357624 }, 0xC99B2298, { 0xB60950ED }, 0x90CDB629 },
{ { 8023.011, -2766.173, -2.03906 }, { 0, 0, -0.352631, 0.9357624 }, 0x05AA9AB6, { 0xB60950ED }, 0x90CDB629 },
{ { 8038.991, -2770.258, -2.124705 }, { 0, 0, -0.3463889, 0.938091 }, 0xEBC666EE, { 0xB60950ED }, 0x90CDB629 },
{ { 8033.327, -2765.306, -2.124705 }, { 0, 0, -0.3463889, 0.938091 }, 0x22985491, { 0xB60950ED }, 0x90CDB629 },
{ { 8027.619, -2760.418, -2.124705 }, { 0, 0, -0.3463889, 0.938091 }, 0x1142B1E6, { 0xB60950ED }, 0x90CDB629 },
{ { 7599.844, -2932.821, 15.6018 }, { 2E-08, -2E-08, -0.819152, 0.5735765 }, 0x43A8C884, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7593.305, -2951.199, 15.6018 }, { 2E-08, -3E-08, -0.8191521, 0.5735763 }, 0x556A6C07, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7596.921, -2797.521, 15.6018 }, { 3E-08, -2E-08, -0.5863907, 0.8100284 }, 0xE91E9371, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7590.834, -2779.004, 15.6018 }, { 3E-08, -2E-08, -0.5863907, 0.8100284 }, 0xFB2D378E, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7155.861, -2707.705, 19.87591 }, { 0, 0, 0, 1 }, 0x3A12551C, { 0x6AF43CF0 }, 0xCC1FDA70 },
{ { 7299.113, -3130.029, 7.589143 }, { 0, 0, 0, 1 }, 0x2CF93AEA, { 0x0063B10D }, 0xE3AEA594 },
{ { 8006.453, -2797.399, 5.367902 }, { 0, 0, -0.7071066, 0.7071066 }, 0x3343114D, { 0xB60950ED }, 0x90CDB629 },
{ { 8002.523, -2857.729, 5.345247 }, { 0, 0, 1, 4E-08 }, 0x457CB5C0, { 0x613F9ACE }, 0x8FD00F13 },
{ { 7502.848, -3379.548, 5.35935 }, { 0, 0, 0.7071066, 0.7071068 }, 0x522B4F1D, { 0x0063B10D }, 0xE3AEA594 },
{ { 7573.575, -2789.157, 11.60442 }, { 0, 0, 0, 1 }, 0x8F4EE705, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7575.046, -2943.087, 11.60442 }, { 0, 0, 0, 1 }, 0x40E94A37, { 0x8AAEF687 }, 0xC8389D8E },
{ { 7207.32, -2991.063, 11.19591 }, { 0, 0, 0, 1 }, 0x0782A8F1, { 0x0063B10D }, 0xE3AEA594 },
{ { 7085.101, -2223.482, 17.31951 }, { 0, 0, 0, 1 }, 0xE443BEDE, { 0x2F7A499E }, 0x94977D21 },
{ { 7124.541, -2310.502, 20.54692 }, { 0, 0, 0, 1 }, 0xEF21D49A, { 0x2F7A499E }, 0x94977D21 },
{ { 7126.585, -2443.165, 23.60547 }, { 0, 0, 0, 1 }, 0x00C777E5, { 0x2F7A499E }, 0x94977D21 },
{ { 7192.949, -2417.326, 19.22641 }, { 0, 0, 0, 1 }, 0xB3DC5E14, { 0x2F7A499E }, 0x94977D21 },
{ { 7122.696, -2521.197, 30.34906 }, { 0, 0, 0, 1 }, 0x7B7B6D4F, { 0x2F7A499E }, 0x94977D21 },
{ { 7298.146, -2416.395, 16.92195 }, { 0, 0, 0, 1 }, 0xD2971B85, { 0x2F7A499E }, 0x94977D21 },
{ { 7317.957, -2572.151, 21.52089 }, { 0, 0, 0, 1 }, 0x827AB561, { 0x2F7A499E }, 0x94977D21 },
{ { 7231.985, -2672.66, 27.31343 }, { 0, 0, 0, 1 }, 0x90BC51E4, { 0x2F7A499E }, 0x94977D21 },
{ { 7233.177, -2804.551, 27.67393 }, { 0, 0, 0, 1 }, 0xBF3CAEE0, { 0x2F7A499E }, 0x94977D21 },
{ { 7314.41, -2703.696, 22.67739 }, { 0, 0, 0, 1 }, 0xBB64A734, { 0x2F7A499E }, 0x94977D21 },
{ { 7476.995, -2668.578, 16.73334 }, { 0, 0, 0, 1 }, 0xF78B6571, { 0x2F7A499E }, 0x94977D21 },
{ { 7075.26, -2604.697, 34.76093 }, { 0, 0, 0, 1 }, 0x88BA06EC, { 0x2F7A499E }, 0x94977D21 },
{ { 7078.132, -2727.26, 33.04692 }, { 0, 0, 0, 1 }, 0xB7EFE557, { 0x2F7A499E }, 0x94977D21 },
{ { 7095.494, -2835.027, 32.81385 }, { 0, 0, 0, 1 }, 0xB230D9D9, { 0x2F7A499E }, 0x94977D21 },
{ { 7098.481, -2925.487, 24.27576 }, { 0, 0, 0, 1 }, 0xD1B398DA, { 0x2F7A499E }, 0x94977D21 },
{ { 7177.86, -2927.829, 22.72505 }, { 0, 0, 0, 1 }, 0x04F67F5F, { 0x2F7A499E }, 0x94977D21 },
{ { 7061.552, -3081.583, 21.7752 }, { 0, 0, 0, 1 }, 0x3BC9ED09, { 0x2F7A499E }, 0x94977D21 },
{ { 7128.21, -3061.066, 16.16288 }, { 0, 0, 0, 1 }, 0x2622C1BB, { 0x2F7A499E }, 0x94977D21 },
{ { 7082.817, -3187.696, 22.68714 }, { 0, 0, 0, 1 }, 0x0C8B0E8C, { 0x2F7A499E }, 0x94977D21 },
{ { 7226.396, -2944.37, 19.64178 }, { 0, 0, 0, 1 }, 0xEE595225, { 0x2F7A499E }, 0x94977D21 },
{ { 7405, -2848.72, 10.53674 }, { 0, 0, 0, 1 }, 0xA223BAA3, { 0x2F7A499E }, 0x94977D21 },
{ { 7326.399, -2909.405, 19.41446 }, { 0, 0, 0, 1 }, 0xCCFF4A65, { 0x2F7A499E }, 0x94977D21 },
{ { 7337.79, -2990.139, 17.19132 }, { 0, 0, 0, 1 }, 0x93D05808, { 0x2F7A499E }, 0x94977D21 },
{ { 7308.94, -3038.436, 15.92005 }, { 0, 0, 0, 1 }, 0xA999839A, { 0x2F7A499E }, 0x94977D21 },
{ { 7197.832, -3054.014, 16.11018 }, { 0, 0, 0, 1 }, 0x863BBCDF, { 0x2F7A499E }, 0x94977D21 },
{ { 7251.192, -3100.502, 16.27592 }, { 0, 0, 0, 1 }, 0x70759153, { 0x2F7A499E }, 0x94977D21 },
{ { 7215.235, -3191.053, 17.16873 }, { 0, 0, 0, 1 }, 0xA09E36B0, { 0x2F7A499E }, 0x94977D21 },
{ { 6915.701, -2587.986, 45.52475 }, { 0, 0, 0, 1 }, 0xE9361C61, { 0xE7F12064 }, 0xBF4739F1 },
{ { 6643.765, -2202.321, 13.62904 }, { 0, 0, 0, 1 }, 0x7ABC3F77, { 0xE7F12064 }, 0xBF4739F1 },
{ { 6688.943, -2619.679, 40.60717 }, { 0, 0, 0, 1 }, 0xCC99E325, { 0xE7F12064 }, 0xBF4739F1 },
{ { 6745.801, -2836.09, 42.3353 }, { 0, 0, 0, 1 }, 0xBDB6455E, { 0xE7F12064 }, 0xBF4739F1 },
{ { 6911.213, -2849.354, 26.72299 }, { 0, 0, 0, 1 }, 0xF74EB892, { 0xE7F12064 }, 0xBF4739F1 },
{ { 6953.231, -2312.15, 46.38253 }, { 0, 0, 0, 1 }, 0x3BDDC1B3, { 0xE7F12064 }, 0xBF4739F1 },
{ { 6931.746, -2910.643, 22.55761 }, { 0, 0, 0, 1 }, 0xEC11FC6E, { 0x41B4FF1B }, 0x599240F3 },
{ { 6769.182, -2901.835, 25.67956 }, { 0, 0, 0, 1 }, 0x53173FB6, { 0x41B4FF1B }, 0x599240F3 },
{ { 6992.65, -2987.11, 12.1841 }, { 0, 0, 0, 1 }, 0x75E3AF32, { 0x41B4FF1B }, 0x599240F3 },
{ { 6673.247, -2356.735, 18.58587 }, { 0, 0, 0, 1 }, 0xD7A6BF70, { 0x726A6085 }, 0xCF692CA3 },
{ { 7026.775, -2732.974, 34.53723 }, { 0, 0, 0, 1 }, 0x35A30BD4, { 0x41B4FF1B }, 0x599240F3 },
{ { 6968.423, -2762.171, 35.55855 }, { 0, 0, 0, 1 }, 0x6704D8DA, { 0x41B4FF1B }, 0x599240F3 },
{ { 7022.816, -2784.107, 40.78513 }, { 0, 0, 0, 1 }, 0x7E3A5737, { 0x41B4FF1B }, 0x599240F3 },
{ { 7029.923, -2847.265, 31.97407 }, { 0, 0, 0, 1 }, 0x23803230, { 0x41B4FF1B }, 0x599240F3 },
{ { 6958.723, -2833.846, 34.76637 }, { 0, 0, 0, 1 }, 0xE6889865, { 0x41B4FF1B }, 0x599240F3 },
{ { 6797.898, -2766.358, 50.53392 }, { 0, 0, 0, 1 }, 0x71FCFD35, { 0x5828AC02 }, 0xABDB6588 },
{ { 6796.861, -2815.2, 46.24396 }, { 0, 0, 0, 1 }, 0x3106D13F, { 0x5828AC02 }, 0xABDB6588 },
{ { 7027.859, -2591.786, 41.33852 }, { 0, 0, 0, 1 }, 0x20586119, { 0x0FEB1B88 }, 0x46E21B93 },
{ { 7027.283, -2664.339, 41.33852 }, { 0, 0, 0, 1 }, 0xF29420E6, { 0x0FEB1B88 }, 0x46E21B93 },
{ { 6893.117, -2587.441, 55.53001 }, { 0, 0, 0, 1 }, 0xAB978096, { 0x0FEB1B88 }, 0x46E21B93 },
{ { 6860.254, -2587.385, 55.53002 }, { 0, 0, 1, -4E-08 }, 0xD5751F7D, { 0x0FEB1B88 }, 0x46E21B93 },
{ { 6957.56, -2871.89, 32.10481 }, { 0, 0, 0, 1 }, 0x8EEB4CB8, { 0x41B4FF1B }, 0x599240F3 },
{ { 6879.886, -2886.316, 31.33868 }, { 0, 0, 0, 1 }, 0x4A540F5D, { 0x5828AC02 }, 0xABDB6588 },
{ { 6808.645, -2887.291, 33.05497 }, { 0, 0, 0, 1 }, 0xA98B6279, { 0x5828AC02 }, 0xABDB6588 },
{ { 6965.304, -2838.749, 36.69212 }, { 0, 0, 0, 1 }, 0xED56BCD0, { 0x41B4FF1B }, 0x599240F3 },
{ { 6944.563, -2676.064, 36.9407 }, { 0, 0, 0, 1 }, 0x9EF255F5, { 0x0FEB1B88 }, 0x46E21B93 },
{ { 7051.296, -2773.846, 28.66566 }, { 0, 0, 0, 1 }, 0xC98A21D9, { 0x41B4FF1B }, 0x599240F3 },
{ { 6865.315, -2811.832, 51.81661 }, { 0, 0, 0, 1 }, 0xBFD37322, { 0x5828AC02 }, 0xABDB6588 },
{ { 6626.369, -2508.173, 30.5962 }, { 0, 0, 0, 1 }, 0x14CFABCE, { 0x42D78160 }, 0x9D2A4826 },
{ { 6685.665, -2505.907, 30.45929 }, { 0, 0, -9E-08, 1 }, 0xECEFB091, { 0x42D78160 }, 0x9D2A4826 },
{ { 6676.957, -2693.493, 38.73234 }, { 0, 0, 0, 1 }, 0x7527A767, { 0x42D78160 }, 0x9D2A4826 },
{ { 6953.045, -2744.072, 38.14449 }, { 0, 0, -9E-08, 1 }, 0x79F4D5BA, { 0x41B4FF1B }, 0x599240F3 },
{ { 6876.706, -2587.437, 42.83051 }, { 0, 0, -9E-08, 1 }, 0x3E874AC0, { 0x0FEB1B88 }, 0x46E21B93 },
{ { 6677.277, -2642.692, 28.90334 }, { 0, 0, 0, 1 }, 0x48A84129, { 0x42D78160 }, 0x9D2A4826 },
{ { 6971.335, -2594.891, 36.63216 }, { 0, 0, 0.4962165, 0.8681988 }, 0x4594A36B, { 0x0FEB1B88 }, 0x46E21B93 },
{ { 6955.339, -2413.084, 21.27517 }, { 0, 0, -9E-08, 1 }, 0x25BAC0A4, { 0x6524C5FA }, 0xC2BD134B },
{ { 6677.159, -2616.273, 38.65293 }, { 0, 0, 0, 1 }, 0xA21E2B5F, { 0x42D78160 }, 0x9D2A4826 },
{ { 6848.929, -2229.891, 21.76537 }, { 0, 0, 0, 1 }, 0x12EBE549, { 0x2F7A499E }, 0x4C491024 },
{ { 6948.077, -2213.489, 23.09986 }, { 0, 0, 0, 1 }, 0x36E1B06C, { 0x2F7A499E }, 0x4C491024 },
{ { 7022.598, -2239.107, 21.97731 }, { 0, 0, 0, 1 }, 0x0D385D1A, { 0x2F7A499E }, 0x4C491024 },
{ { 6835.243, -2327.661, 19.87493 }, { 0, 0, 0, 1 }, 0x5F7301BE, { 0x2F7A499E }, 0x4C491024 },
{ { 6940.678, -2384.969, 26.30894 }, { 0, 0, 0, 1 }, 0x711CA511, { 0x2F7A499E }, 0x4C491024 },
{ { 7027.434, -2400.079, 21.0524 }, { 0, 0, 0, 1 }, 0x8BF7DAC7, { 0x2F7A499E }, 0x4C491024 },
{ { 6740.45, -2403.911, 27.20941 }, { 0, 0, 0, 1 }, 0x8D6EDD89, { 0x2F7A499E }, 0x4C491024 },
{ { 6844.367, -2406.915, 28.77763 }, { 0, 0, 0, 1 }, 0x9B657976, { 0x2F7A499E }, 0x4C491024 },
{ { 6933.254, -2499.541, 31.55883 }, { 0, 0, 0, 1 }, 0xC369440D, { 0x2F7A499E }, 0x4C491024 },
{ { 7025.51, -2506.135, 29.19266 }, { 0, 0, 0, 1 }, 0xD5AB6891, { 0x2F7A499E }, 0x4C491024 },
{ { 6830.526, -2498.364, 34.83371 }, { 0, 0, 0, 1 }, 0xA8DB8EF2, { 0x2F7A499E }, 0x4C491024 },
{ { 6643.109, -2496.023, 37.45084 }, { 0, 0, 0, 1 }, 0x8B0A5350, { 0x2F7A499E }, 0x4C491024 },
{ { 6736.693, -2499.468, 36.4225 }, { 0, 0, 0, 1 }, 0xB90EAF24, { 0x2F7A499E }, 0x4C491024 },
{ { 6645.804, -2605.042, 42.47207 }, { 0, 0, 0, 1 }, 0x9ACF72A6, { 0x2F7A499E }, 0x4C491024 },
{ { 6740.904, -2605.163, 35.96131 }, { 0, 0, 0, 1 }, 0x6CEA96DD, { 0x2F7A499E }, 0x4C491024 },
{ { 6844.768, -2604.716, 35.95251 }, { 0, 0, 0, 1 }, 0x7F34BB71, { 0x2F7A499E }, 0x4C491024 },
{ { 6943.149, -2603.273, 34.94673 }, { 0, 0, 0, 1 }, 0x4E4ED9D6, { 0x2F7A499E }, 0x4C491024 },
{ { 6951.333, -2702.492, 35.97678 }, { 0, 0, 0, 1 }, 0x60017D3B, { 0x2F7A499E }, 0x4C491024 },
{ { 6841.501, -2704.093, 36.23222 }, { 0, 0, 0, 1 }, 0x983467E8, { 0x2F7A499E }, 0x4C491024 },
{ { 6647.063, -2711.469, 39.79441 }, { 0, 0, 0, 1 }, 0x85DDC33B, { 0x2F7A499E }, 0x4C491024 },
{ { 6738.724, -2703.04, 36.43524 }, { 0, 0, 0, 1 }, 0x7B97AEAF, { 0x2F7A499E }, 0x4C491024 },
{ { 6728.72, -2800.814, 35.83888 }, { 0, 0, 0, 1 }, 0x69500A20, { 0x2F7A499E }, 0x4C491024 },
{ { 6837.291, -2803.338, 36.71156 }, { 0, 0, 0, 1 }, 0xD92369C5, { 0x2F7A499E }, 0x4C491024 },
{ { 6939.029, -2808.533, 36.03247 }, { 0, 0, 0, 1 }, 0xBE813481, { 0x2F7A499E }, 0x4C491024 },
{ { 6660.688, -2850.337, 28.08445 }, { 0, 0, 0, 1 }, 0xAC4E901C, { 0x2F7A499E }, 0x4C491024 },
{ { 7033.999, -2895.917, 21.55012 }, { 0, 0, 0, 1 }, 0x5B9AA96B, { 0x41B4FF1B }, 0x599240F3 },
{ { 6646.387, -2402.292, 29.21537 }, { 0, 0, 0, 1 }, 0x9DE67EA4, { 0x2F7A499E }, 0x4C491024 },
{ { 6637.973, -2322.406, 21.64866 }, { 0, 0, 0, 1 }, 0x46544F51, { 0x2F7A499E }, 0x4C491024 },
{ { 6733.233, -2318.228, 21.70116 }, { 0, 0, 0, 1 }, 0x54A8EBFA, { 0x2F7A499E }, 0x4C491024 },
{ { 6571.459, -2278.094, 17.40641 }, { 0, 0, 0, 1 }, 0x2F041D79, { 0x2F7A499E }, 0x4C491024 },
{ { 6639.114, -2241.413, 17.08913 }, { 0, 0, 0, 1 }, 0x3F49BE04, { 0x2F7A499E }, 0x4C491024 },
{ { 6742.231, -2218.876, 20.29282 }, { 0, 0, 0, 1 }, 0x013141D4, { 0x2F7A499E }, 0x4C491024 },
{ { 6827.775, -2928.035, 27.85155 }, { 0, 0, 0, 1 }, 0x23FC7F6A, { 0x2F7A499E }, 0x4C491024 },
{ { 7001.656, -2961.206, 23.9056 }, { 0, 0, 0, 1 }, 0xED94129E, { 0x2F7A499E }, 0x4C491024 },
{ { 6219.041, -2631.991, 36.37098 }, { 0, 0, 0, 1 }, 0x9778D322, { 0x03C13110 }, 0x8CC58CF5 },
{ { 6357.308, -2783.565, 14.41829 }, { 0, 0, 0, 1 }, 0x7AFA1A25, { 0x03C13110 }, 0x8CC58CF5 },
{ { 6541.919, -2796.902, 24.36119 }, { 0, 0, 0, 1 }, 0xCED1C1D7, { 0x03C13110 }, 0x8CC58CF5 },
{ { 6032.423, -2631.026, 45.38129 }, { 0, 0, 0, 1 }, 0xA92E768D, { 0x03C13110 }, 0x8CC58CF5 },
{ { 6398.292, -2665.928, 29.86254 }, { 0, 0, 0, 1 }, 0xCEA8C189, { 0x03C13110 }, 0x8CC58CF5 },
{ { 6323.588, -2667.672, 56.21764 }, { 0, 0, 0, 1 }, 0x3BF4E9CF, { 0x7961AB64 }, 0x312D483E },
{ { 6420.555, -2705.149, 45.87214 }, { 0, 0, 0, 1 }, 0xCE6AAC17, { 0x7961AB64 }, 0x312D483E },
{ { 6364.448, -2806.972, 44.28435 }, { 0, 0, 0, 1 }, 0x1EC28831, { 0x7961AB64 }, 0x312D483E },
{ { 6097.964, -2674.373, 32.66583 }, { 0, 0, 0, 1 }, 0x05BB70FF, { 0x7961AB64 }, 0x312D483E },
{ { 6450.532, -2814.017, 36.89194 }, { 0, 0, 0, 1 }, 0xF9669C05, { 0x7961AB64 }, 0x312D483E },
{ { 6473.745, -2734.635, 36.69446 }, { 0, 0, 0, 1 }, 0x1FCE06A9, { 0x30DFF035 }, 0x359E1B34 },
{ { 6499.696, -2734.703, 36.1432 }, { 0, 0, 0, 1 }, 0x91A8EA5D, { 0x7961AB64 }, 0x312D483E },
{ { 6211.748, -2667.953, 41.90421 }, { 0, 0, 0, 1 }, 0xC3BF4364, { 0x7961AB64 }, 0x312D483E },
{ { 6073.059, -2658.491, 23.98135 }, { 0, 0, 0, 1 }, 0x8D7D6C8D, { 0x7961AB64 }, 0x312D483E },
{ { 6235.449, -2747.442, 21.13236 }, { 0, 0, 0, 1 }, 0xBCB8C42E, { 0x7961AB64 }, 0x312D483E },
{ { 6243.999, -2667.953, 41.90421 }, { 0, 0, 0, 1 }, 0xCC88AD85, { 0x7961AB64 }, 0x312D483E },
{ { 6299.476, -2772.307, 47.88387 }, { 0, 0, 0, 1 }, 0x61578A73, { 0x7961AB64 }, 0x312D483E },
{ { 6300.614, -2781.475, 23.9512 }, { 0, 0, 0, 1 }, 0x7FCE215C, { 0x606B4F4B }, 0x4730BE59 },
{ { 6656.834, -2915.906, 22.49256 }, { 0, 0, 0, 1 }, 0x1952BEA6, { 0x7961AB64 }, 0x312D483E },
{ { 6523.773, -2833.227, 33.6155 }, { 0, 0, 0, 1 }, 0x04F4B247, { 0x7961AB64 }, 0x312D483E },
{ { 6523.1, -2900.546, 32.91179 }, { 0, 0, 0, 1 }, 0x8457BB1E, { 0x7961AB64 }, 0x312D483E },
{ { 6382.747, -2896.067, 40.15963 }, { 0, 0, 0, 1 }, 0xF16BFEE0, { 0x7961AB64 }, 0x312D483E },
{ { 5979.614, -2754.789, 0.2229364 }, { 0, 0, 0, 1 }, 0xFFE4073A, { 0x4EC3ABFC }, 0x110F5213 },
{ { 6200.284, -2725.617, 30.89707 }, { 0, 0, 0, 1 }, 0x545D7D15, { 0x7961AB64 }, 0x312D483E },
{ { 6171.468, -2632.077, 42.55495 }, { 0, 0, -0.7071066, 0.7071066 }, 0xD69F0480, { 0x124F4BEC }, 0xC7CE7360 },
{ { 5832.368, -2632.077, 24.94958 }, { 0, 0, -0.7071066, 0.7071066 }, 0x98612761, { 0x124F4BEC }, 0xC7CE7360 },
{ { 6418.474, -2634.272, 42.20729 }, { 0, 0, -0.2672384, 0.9636305 }, 0xD93DA8FD, { 0x124F4BEC }, 0xC7CE7360 },
{ { 6039.562, -2632.077, 42.55495 }, { 0, 0, -0.7071066, 0.7071066 }, 0x35C8E212, { 0x124F4BEC }, 0xC7CE7360 },
{ { 6370.775, -2720.209, 32.95122 }, { 0, 0, -0.2672384, 0.9636305 }, 0xCC770F70, { 0x124F4BEC }, 0xC7CE7360 },
{ { 6298.608, -2874.772, 33.76309 }, { 0, 0, -0.1305262, 0.9914448 }, 0x7DA071C4, { 0x124F4BEC }, 0xC7CE7360 },
{ { 6346.042, -2679.983, 37.32347 }, { 0, 0, 0, 1 }, 0xD8D156D3, { 0x7961AB64 }, 0x312D483E },
{ { 6314.992, -2802.993, 29.49007 }, { 0, 0, 0, 1 }, 0xC43D0DE8, { 0x7961AB64 }, 0x312D483E },
{ { 6232.423, -2767.586, 20.58415 }, { 0, 0, 0.2588191, 0.9659258 }, 0xCD9075A5, { 0x7961AB64 }, 0x312D483E },
{ { 6239.357, -2763.543, 20.54508 }, { 0, 0, 0, 1 }, 0x0402E28D, { 0x7961AB64 }, 0x312D483E },
{ { 6105.183, -2692.456, 23.42001 }, { 0, 0, 1, 8E-08 }, 0xCA383FCC, { 0x7961AB64 }, 0x312D483E },
{ { 6205.724, -2668.103, 31.50641 }, { 0, 0, 0, 1 }, 0xB4A0CD40, { 0x606B4F4B }, 0x4730BE59 },
{ { 6555.843, -2687.115, 41.46331 }, { 0, 0, -0.7071068, 0.7071066 }, 0x5743A0CA, { 0x5B782690 }, 0x4871302C },
{ { 6490.553, -2674.245, 40.09492 }, { 0, 0, 0.9238793, -0.3826839 }, 0x873E7E30, { 0x5B782690 }, 0x4871302C },
{ { 6416.84, -2708.992, 40.22583 }, { 0, 0, -0.7071068, 0.7071066 }, 0x4D018C46, { 0x5B782690 }, 0x4871302C },
{ { 6593.83, -2799.081, 32.93495 }, { 0, 0, 0, 1 }, 0xABD14755, { 0x5B782690 }, 0x4871302C },
{ { 6451.853, -2811.732, 31.7826 }, { 0, 0, 0, 1 }, 0x76ECDD8D, { 0x5B782690 }, 0x4871302C },
{ { 6334.348, -2799.912, 36.06788 }, { 0, 0, 1, 1.9E-07 }, 0xBE9A6CE7, { 0x5B782690 }, 0x4871302C },
{ { 6229.992, -2846.724, 33.04892 }, { 0, 0, 0, 1 }, 0xCE120BB2, { 0x5B782690 }, 0x4871302C },
{ { 6152.504, -2729.185, 28.93578 }, { 0, 0, 0, 1 }, 0x08D5815C, { 0x5B782690 }, 0x4871302C },
{ { 6030.294, -2701.71, 25.18422 }, { 0, 0, -1.7E-07, 1 }, 0x1E9FACF0, { 0x5B782690 }, 0x4871302C },
{ { 6408.466, -2933.756, 35.90915 }, { 0, 0, 0, 1 }, 0xF09ED0CB, { 0x5B782690 }, 0x4871302C },
{ { 6570.638, -2932.645, 29.71704 }, { 0, 0, 0, 1 }, 0xE250342E, { 0x5B782690 }, 0x4871302C },
{ { 6276.805, -2769.6, 31.44274 }, { 0, 0, 0, 1 }, 0xF150803A, { 0x7961AB64 }, 0x312D483E },
{ { 6282.628, -2724.941, 30.01627 }, { 0, 0, 0, 1 }, 0x9B2F53F9, { 0x7961AB64 }, 0x312D483E },
{ { 6278.614, -2826.916, 30.06211 }, { 0, 0, 0, 1 }, 0xD60B12E7, { 0x7961AB64 }, 0x312D483E },
{ { 6091.555, -2506.81, 14.52365 }, { 0, 0, 0, 1 }, 0x53AF6A50, { 0x9CD5D5E6 }, 0xD997F856 },
{ { 6335.646, -2356.051, 31.26772 }, { 0, 0, 0, 1 }, 0xFDC83E83, { 0x9CD5D5E6 }, 0xD997F856 },
{ { 6299.521, -2563.017, 35.71094 }, { 0, 0, 0, 1 }, 0x16B2F058, { 0x9CD5D5E6 }, 0xD997F856 },
{ { 6552.006, -2418.993, 20.22842 }, { 0, 0, 0, 1 }, 0xCD185D24, { 0x9CD5D5E6 }, 0xD997F856 },
{ { 6152.834, -2346.325, 14.33189 }, { 0, 0, 0, 1 }, 0x424C478A, { 0x9CD5D5E6 }, 0xD997F856 },
{ { 6341.332, -2147.547, -0.3205314 }, { 0, 0, 9E-08, 1 }, 0x400639BC, { 0xEAC3F7C7 }, 0x75246AAB },
{ { 6180.035, -2209.187, 1.13909 }, { 0, 0, 0, 1 }, 0x30309A11, { 0xEAC3F7C7 }, 0x75246AAB },
{ { 6020.108, -2314.786, 0.7849912 }, { 0, 0, 0, 1 }, 0x04F24371, { 0xEAC3F7C7 }, 0x75246AAB },
{ { 6005.088, -2502.552, -0.3704364 }, { 0, 0, 0, 1 }, 0x1707E79C, { 0xEAC3F7C7 }, 0x75246AAB },
{ { 6079.797, -2532.489, 14.71744 }, { 0, 0, 0, 1 }, 0x6EE2C8FB, { 0x8DFC3F0D }, 0x8AE01198 },
{ { 6195.938, -2573.947, 26.38102 }, { 0, 0, 0, 1 }, 0x42CC7899, { 0x8DFC3F0D }, 0x8AE01198 },
{ { 6155.165, -2535.631, 26.64074 }, { 0, 0, 0, 1 }, 0x34B4DC6A, { 0x8DFC3F0D }, 0x8AE01198 },
{ { 6339.362, -2494.077, 39.3843 }, { 0, 0, 0, 1 }, 0xE9D88642, { 0x0009237D }, 0x6741CA54 },
{ { 6444.835, -2494.288, 39.32734 }, { 0, 0, 0, 1 }, 0xF81622BD, { 0x0009237D }, 0x6741CA54 },
{ { 6585.93, -2527.576, 35.84781 }, { 0, 0, 0, 1 }, 0x0F74D17A, { 0x1314493F }, 0xC134FE39 },
{ { 6463.709, -2531.105, 40.53848 }, { 0, 0, -0.2503801, 0.9681478 }, 0xD804DFF8, { 0x131620E3 }, 0x775C04BD },
{ { 6506.61, -2451.395, 32.75299 }, { 0, 0, -0.2503801, 0.9681478 }, 0xE5BC7B67, { 0x131620E3 }, 0x775C04BD },
{ { 5990.707, -2484.239, 0.2003851 }, { 0, 0, 0.7071066, 0.7071066 }, 0x9F8C535E, { 0xF25687C4 }, 0x5DA6B71E },
{ { 6143.021, -2493.485, 25.87401 }, { 0, 0, 0, 1 }, 0x729BF03A, { 0xE2AC0BE3 }, 0x8F39517D },
{ { 6240.42, -2598.357, 37.06692 }, { 0, 0, 0, 1 }, 0x64D254A7, { 0xE2AC0BE3 }, 0x8F39517D },
{ { 6457.385, -2361.472, 30.50919 }, { 0, 0, 0, 1 }, 0x51E49BC6, { 0xE2AC0BE3 }, 0x8F39517D },
{ { 6020.381, -2307.752, -5.526952 }, { 0, 0, 0, 1 }, 0xD96BDEF3, { 0xF25687C4 }, 0x5DA6B71E },
{ { 6129.052, -2329.547, 23.92075 }, { 0, 0, 0, 1 }, 0x0DC43F81, { 0x209D6451 }, 0xB0D55D7A },
{ { 6454.386, -2610.229, 46.02366 }, { 0, 0, -0.7071068, 0.7071066 }, 0x2C68336D, { 0xE89F2C30 }, 0x713C8516 },
{ { 6380.477, -2419.735, 41.27345 }, { 0, 0, 0, 1 }, 0xFB81D2A1, { 0xE89F2C30 }, 0x713C8516 },
{ { 6557.692, -2426.862, 32.59162 }, { 0, 0, 1, 8E-08 }, 0x7D960E4B, { 0xE89F2C30 }, 0x713C8516 },
{ { 6476.167, -2434.341, 35.99958 }, { 0, 0, 0, 1 }, 0xDB4E3D21, { 0xE89F2C30 }, 0x713C8516 },
{ { 6463.069, -2335.521, 31.91557 }, { 0, 0, 0, 1 }, 0x5F0222F3, { 0xE89F2C30 }, 0x713C8516 },
{ { 6361.504, -2287.188, 31.0255 }, { 0, 0, 0, 1 }, 0x0CA474DA, { 0xE89F2C30 }, 0x713C8516 },
{ { 6468.756, -2196.708, 21.26085 }, { 0, 0, -0.7071068, 0.7071066 }, 0x8D6DADFA, { 0xE89F2C30 }, 0x713C8516 },
{ { 6243.021, -2281.954, 30.43363 }, { 0, 0, -0.7071068, 0.7071066 }, 0x447F649B, { 0xE89F2C30 }, 0x713C8516 },
{ { 6099.698, -2292.704, 25.50827 }, { 0, 0, -9E-08, 1 }, 0x1248802E, { 0xE89F2C30 }, 0x713C8516 },
{ { 6094.784, -2422.096, 24.66972 }, { 0, 0, -0.7071068, 0.7071066 }, 0x2911ADC0, { 0xE89F2C30 }, 0x713C8516 },
{ { 6078.931, -2579.91, 24.17352 }, { 0, 0, -0.7071068, 0.7071066 }, 0x7708C9A5, { 0xE89F2C30 }, 0x713C8516 },
{ { 6260.941, -2420.791, 39.99124 }, { 0, 0, -0.7071068, 0.7071066 }, 0x3565C668, { 0xE89F2C30 }, 0x713C8516 },
{ { 6258.892, -2561.367, 44.24664 }, { 0, 0, 0, 1 }, 0xE02E1BFA, { 0xE89F2C30 }, 0x713C8516 },
{ { 6374.943, -2553.891, 43.35051 }, { 0, 0, -0.7071068, 0.7071066 }, 0x10C7FD2D, { 0xE89F2C30 }, 0x713C8516 },
{ { 6559.793, -2550.997, 43.22389 }, { 0, 0, -9E-08, 1 }, 0x6EB6F07D, { 0xE89F2C30 }, 0x713C8516 },

};

static std::string packs[] = {
	"platform:/levels/gta5/_citye/downtown_01/dt1_05.rpf",
	"platform:/levels/gta5/_citye/downtown_01/dt1_21.rpf",
	"platform:/levels/gta5/generic/gtxd.rpf",
	"platform:/levels/gta5/_citye/downtown_01/fwy_02.rpf",
	"platform:/levels/gta5/_hills/cityhills_03/cityhills_03.rpf",
	"platform:/levels/gta5/_hills/cityhills_03/ch3_06.rpf",
	"platform:/levels/gta5/props/vegetation/v_bush.rpf",
	"platform:/levels/gta5/props/vegetation/v_trees.rpf",
	"platform:/levels/gta5/_hills/cityhills_03/ch3_10.rpf",
	"platform:/levels/gta5/props/industrial/v_industrial.rpf",
	"platform:/levels/gta5/_hills/cityhills_03/ch3_11.rpf",
	"platform:/levels/gta5/_hills/cityhills_03/ch3_rd1.rpf"
};

static hook::cdecl_stub<int*(streaming::strStreamingModule*, int* id, uint32_t* hash)> _getindexbykey([]()
{
	return (void*)0x14067C6A0;
});

#include <atPool.h>

void tempDrawEntity::Update()
{
	static auto strMgr = streaming::Manager::GetInstance();
	static auto txdStore = strMgr->moduleMgr.GetStreamingModule("ytd");
	static auto drbStore = strMgr->moduleMgr.GetStreamingModule("ydr");
	static auto dwdStore = strMgr->moduleMgr.GetStreamingModule("ydd");

	bool allLoaded = true;
	int mainTxdIdx = 0;

	for (auto txd : txds)
	{
		int idx = -1;
		_getindexbykey(txdStore, &idx, &txd);

		if (!mainTxdIdx)
		{
			mainTxdIdx = idx;
		}

		if (idx != -1)
		{
			if ((strMgr->Entries[txdStore->baseIdx + idx].flags & 3) != 1)
			{
				allLoaded = false;

				strMgr->RequestObject(txdStore->baseIdx + idx, 0);
			}
		}
	}

	if (!allLoaded)
	{
		return;
	}

	auto store = (dwdHash) ? dwdStore : drbStore;
	auto asset = (dwdHash) ? dwdHash : modelHash;

	int idx = -1;
	_getindexbykey(store, &idx, &asset);

	if (idx != -1)
	{
		if ((strMgr->Entries[store->baseIdx + idx].flags & 3) != 1)
		{
			auto pool = ((atPoolBase*)((char*)store + 56));

			if (dwdHash)
			{
				if (pool->GetAt<char>(idx))
				{
					*(uint16_t*)(pool->GetAt<char>(idx) + 16) = mainTxdIdx;
				}
			}
			else
			{
				if (pool->GetAt<char>(idx))
				{
					*(uint16_t*)(pool->GetAt<char>(idx) + 24) = mainTxdIdx;
				}
			}

			strMgr->RequestObject(store->baseIdx + idx, 0);
		}
		else
		{
			alignas(16) DirectX::XMFLOAT4X4 matrix;
			DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixAffineTransformation(
				DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f),
				DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
				DirectX::XMVectorSet(orientation.x, orientation.y, orientation.z, orientation.w),
				DirectX::XMVectorSet(position.x, position.y, position.z, 1.0f)
			));

			/*auto oldRasterizerState = GetRasterizerState();
			SetRasterizerState(GetStockStateIdentifier(RasterizerStateDefault));

			auto oldBlendState = GetBlendState();
			SetBlendState(GetStockStateIdentifier(BlendStateDefault));

			SetDepthStencilState(*(int*)0x142B08C94);*/

			rmcDrawable* drawable;

			if (dwdHash)
			{
				auto dict = (rage::five::pgDictionary<rmcDrawable>*)store->GetPtr(idx);
				drawable = dict->Get(modelHash);
			}
			else
			{
				drawable = (rmcDrawable*)store->GetPtr(idx);
			}

			ID3DUserDefinedAnnotation* pPerf;
			GetD3D11DeviceContext()->QueryInterface(__uuidof(pPerf), reinterpret_cast<void**>(&pPerf));

			pPerf->BeginEvent(fmt::sprintf(L"draw %s", ToWide(drawable->name ? drawable->name : fmt::sprintf("%08x", modelHash))).c_str());

			drawable->Draw(&matrix, 0xFFFF, 0);

			pPerf->EndEvent();
			pPerf->Release();
		}
	}
}

//extern fwEvent<> OnPostFrontendRender;

static InitFunction initFunctionSceneExperiment([]()
{
	// again it's fucked and won't ever work
	return;

	OnPostFrontendRender.Connect([]()
	{
		//GetD3D11DeviceContext()->ClearDepthStencilView(NULL, D3D11_CLEAR_DEPTH, 0.0f, 0);

		char viewport[1156];
		((void(*)(void*))0x1412E9644)((void*)viewport); // grcViewport::ctor

		((void(*)(void* viewport, float fov, float aspect, float near, float far))0x141304D7C)((void*)viewport, 45.f, 21.f/9.f, 0.1f, 1000.f); // grcViewport::Perspective

		alignas(16) DirectX::XMFLOAT4X4 matrix;
		//DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixLookToLH(DirectX::XMVectorSet(1387.26f, -1052.22f, 147.52f, 1.0f), /*DirectX::XMVectorSet(1442.44f, -1071.69f, 132.27f, 1.0f)*/ DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f)));
		DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixLookAtRH(DirectX::XMVectorSet(1387.26f, -1052.22f, 147.52f, 1.0f), DirectX::XMVectorSet(1442.44f, -1071.69f, 132.27f, 1.0f), DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))));
		//DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), -3.14159265f / 2.0f)) * DirectX::XMMatrixTranslation(-1387.26f, -(-1052.22f), -147.52f));

		((void(*)(void* viewport, void* camMatrix))0x1413098DC)((void*)viewport, &matrix); // grcViewport::SetCamMtx

		((void(*)(void* viewport, bool))0x141309CCC)((void*)viewport, true); // grcViewport::__activate

		static bool on = false;

		// no static bounds for test
		//hook::return_function(0x141590B3C);
		hook::jump(0x141590B3C, 0x141590918);

		if (GetAsyncKeyState(VK_F5))
		{
			on = true;

			for (auto& pack : packs)
			{
				struct DataFileEntry
				{
					char name[128];
					char pad[16]; // 128
					int32_t type; // 140
					int32_t index; // 148
					bool locked; // 152
					bool flag2; // 153
					bool flag3; // 154
					bool disabled; // 155
					bool persistent; // 156
					bool overlay;
					char pad2[10];
				};

				DataFileEntry entry = { 0 };
				strcpy(entry.name, pack.c_str());
				entry.type = 0;
				entry.disabled = true;
				entry.overlay = true;

				((void(*)(void*))0x1408DACEC)(&entry);
			}
		}

		if (!streaming::Manager::GetInstance() || !on)
		{
			return;
		}

		/*static uint32_t id;

		if (!id || id == -1)
		{
			streaming::RegisterRawStreamingFile(&id, "citizen:/web.ydr", true, "web.ydr", false);
			return;
		}

		streaming::Manager::GetInstance()->RequestObject(id, 0);

		if ((streaming::Manager::GetInstance()->Entries[id].flags & 3) == 1)
		{
			auto mod = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule(id);
			auto idx = id - mod->baseIdx;

			alignas(16) float matrix[] = {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};

			//SetWorldMatrix(matrix);

			auto oldRasterizerState = GetRasterizerState();
			SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

			auto oldBlendState = GetBlendState();
			SetBlendState(GetStockStateIdentifier(BlendStateDefault));

			ID3DUserDefinedAnnotation* pPerf;
			GetD3D11DeviceContext()->QueryInterface(__uuidof(pPerf), reinterpret_cast<void**>(&pPerf));

			pPerf->BeginEvent(L"DrawCRAP");

			auto drawable = (rmcDrawable*)mod->GetAssetPointer(idx);
			drawable->Draw(matrix, 0xFFFF, 0);

			pPerf->EndEvent();

			SetRasterizerState(oldRasterizerState);

			SetBlendState(oldBlendState);

			pPerf->Release();
		}*/

		// eh gbuf
		((void(*)())0x1405003BC)();

		// clear gbuf
		((void(*)(bool))0x1405001E4)(false);

		// more gbuf
		((void(*)())0x140542710)();

		// act deferred
		((void(*)())0x140547708)();

		ID3DUserDefinedAnnotation* pPerf;
		GetD3D11DeviceContext()->QueryInterface(__uuidof(pPerf), reinterpret_cast<void**>(&pPerf));

		pPerf->BeginEvent(L"DrawCRAP");

		for (auto& entity : entities)
		{
			entity.Update();
		}

		pPerf->EndEvent();

		pPerf->BeginEvent(L"light?");

		// set timecycle
		*(void**)0x14276EEA8 = (void*)0x142770C60;

		static bool tc;

		if (!tc)
		{
			// update timecycle(?)
			((void(*)(int))0x1405F85E8)(4);
			((void(*)(void*, int))0x1405FB59C)((void*)0x14247D790, 4);
			((void(*)(void*, int))0x140DF1188)((void*)0x14276EF00, 4);
			((void(*)(void*, int))0x140DF09AC)((void*)0x14276EF00, 4);

			// don't apply default mods
			hook::return_function(0x140DECE00);

			tc = true;
		}

		((void(*)())0x140027A78)();
		((void(*)())0x140587A34)();

		((void(*)(void* viewport, bool))0x141309CCC)(*(void**)0x142AD7B08, true); // grcViewport::__activate
		((void(*)())0x14050B5A4)();
		((void(*)(bool))0x1405425F8)(true);
		((void(*)(bool))0x1405775F8)(true);
		((void(*)())0x14057969C)();
		((void(*)(bool, bool))0x140519224)(true, false);

		pPerf->EndEvent();

		pPerf->Release();

		((void(*)(void* viewport, bool))0x141309CCC)(*(void**)0x142AD7B08, true); // grcViewport::__activate
		((void(*)())0x14054CB58)();
	}, 99999);
});
