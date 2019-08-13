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

static float tx, ty, tz;

static HookFunction shaderHookFunction([]()
{
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
