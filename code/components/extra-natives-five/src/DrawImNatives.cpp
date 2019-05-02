#include <StdInc.h>

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

	BeginImVertices(3, positions.size());

	for (int i = 0; i < positions.size(); i++)
	{
		AddImVertex(positions[i].x, positions[i].y, 0.0f, 0.0f, 0.0f, -1.0f, colors.empty() ? 0xFFFFFFFF : colors[i], uvs.empty() ? 0.0f : uvs[i].x, uvs.empty() ? 0.0f : uvs[i].y);
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

	DrawImVertices();

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
