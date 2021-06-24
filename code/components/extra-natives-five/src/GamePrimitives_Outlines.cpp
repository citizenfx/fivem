#include <StdInc.h>
#include <GamePrimitives.h>

#include <fiAssetManager.h>
#include <CfxRGBA.h>

#include <Hooking.h>

#include <CoreConsole.h>
#include <imgui.h>

#include <ScriptEngine.h>

// fwRegdRef
namespace rage
{
template<typename T>
class fwRegdRef
{
public:
	inline fwRegdRef()
	{
	}

	inline fwRegdRef(T* value)
	{
		Reset(value);
	}

	inline fwRegdRef(const fwRegdRef& right)
	{
		Reset(right.ref);
	}

	inline fwRegdRef(fwRegdRef&& right)
	{
		auto ref = right.ref;
		right.Reset(nullptr);
		Reset(ref);
	}

	~fwRegdRef()
	{
		Reset(nullptr);
	}

	void Reset(T* ref)
	{
		if (ref != this->ref)
		{
			if (this->ref)
			{
				this->ref->RemoveKnownRef((void**)&this->ref);
			}

			this->ref = ref;

			if (ref)
			{
				ref->AddKnownRef((void**)&this->ref);
			}
		}
	}

	inline auto& operator=(const fwRegdRef& right)
	{
		Reset(right.ref);
		return *this;
	}

	inline auto& operator=(T* right)
	{
		Reset(right);
		return *this;
	}

	inline bool operator<(const fwRegdRef& right) const
	{
		return ref < right.ref;
	}

	inline bool operator==(const fwRegdRef& right) const
	{
		return ref == right.ref;
	}

	inline bool operator!=(const fwRegdRef& right) const
	{
		return ref != right.ref;
	}

	inline T* operator->() const
	{
		return ref;
	}

	inline T& operator*() const
	{
		return *ref;
	}

	inline operator bool() const
	{
		return (ref != nullptr);
	}

	template<typename TOther>
	inline operator TOther*() const
	{
		return (TOther*)ref;
	}

private:
	T* ref = nullptr;
};
}
// end

struct CEntityDrawHandler
{
	virtual ~CEntityDrawHandler() = 0;
	virtual void m_8() = 0;
	virtual void Draw(fwEntity* entity, void* a3) = 0;
};

static rage::grcRenderTarget *maskRenderTarget, *tempRenderTarget;

#define M_PI 3.14159265358979323846
#define M_E 2.71828182845904523536

static float Gauss(float x, float stdDev)
{
	auto stdDev2 = stdDev * stdDev * 2;
	auto a = 1 / sqrtf(M_PI * stdDev2);
	auto gauss = a * powf(M_E, -x * x / stdDev2);

	return gauss;
}

// for entity prototypes
static int* currentShader;
static int* g_currentDrawBucket;

static std::vector<rage::fwRegdRef<fwEntity>> outlineEntities;
static CRGBA outlineColor{255, 0, 255, 255};

static int screenSize, intensity, width, color, gaussSamples;
static int mainTex, maskTex;
static int h, v;
static rage::grmShaderFx* shader;

static void DrawScreenSpace()
{
	float screenSizeF[2] = { 1.0f / (float)GetViewportW(), 1.0f / (float)GetViewportH() };
	shader->SetParameter(screenSize, screenSizeF, 8, 1);

	int widthF = 30;
	shader->SetParameter(width, &widthF, 4, 1);

	float colorF[4] = {
		outlineColor.red / 255.0f,
		outlineColor.green / 255.0f,
		outlineColor.blue / 255.0f,
		1.0f
	};
	shader->SetParameter(color, colorF, 16, 1);

	static float gaussSamplesF[32] = { 0.f };
	static int lastGaussWidth;

	if (lastGaussWidth != widthF)
	{
		for (int i = 0; i < widthF; i++)
		{
			gaussSamplesF[i] = Gauss((float)i, widthF * 0.5f);
		}

		lastGaussWidth = widthF;
	}

	shader->SetParameter(gaussSamples, gaussSamplesF, 16, (32 * 4) / 16);

	float intensityF = 55.f;
	shader->SetParameter(intensity, &intensityF, 4, 1);

	shader->SetSampler(maskTex, maskRenderTarget);
	shader->SetSampler(mainTex, maskRenderTarget);

	rage::grcTextureFactory::getInstance()->PushRenderTarget(nullptr, tempRenderTarget, nullptr, 0, true, 0);
	ClearRenderTarget(true, 0, false, 0.0f, false, 0);

	auto lastZ = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

	auto lastBlend = GetBlendState();
	SetBlendState(GetStockStateIdentifier(BlendStateDefault));

	auto drawQuad = []()
	{
		rage::grcBegin(4, 4);

		uint32_t color = 0xffffffff;

		rage::grcVertex(-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 1.0f);
		rage::grcVertex(1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 1.0f, 1.0f);
		rage::grcVertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		rage::grcVertex(1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 1.0f, 0.0f);

		rage::grcEnd();
	};

	shader->PushTechnique(h, true, 0);
	shader->PushPass(0);
	drawQuad();
	shader->PopPass();
	shader->PopTechnique();

	rage::grcTextureFactory::getInstance()->PopRenderTarget(nullptr, nullptr);

	shader->SetSampler(mainTex, tempRenderTarget);

	shader->PushTechnique(v, true, 0);
	shader->PushPass(0);
	drawQuad();
	shader->PopPass();
	shader->PopTechnique();

	SetDepthStencilState(lastZ);
	SetBlendState(lastBlend);
}

static hook::cdecl_stub<int(const char*)> _getTechniqueDrawName([]()
{
	return hook::get_pattern("E8 ? ? ? ? 33 DB 48 8D 4C 24 20 44 8D", -0x11);
});

static InitFunction initFunctionBuffers([]()
{
	OnSetUpRenderBuffers.Connect([](int w, int h)
	{
		maskRenderTarget = CreateRenderTarget(0, "outlineMaskRT", 3, w, h, 32, nullptr, true, maskRenderTarget);
		tempRenderTarget = CreateRenderTarget(0, "outlineTempRT", 3, w, h, 32, nullptr, true, tempRenderTarget);
	});

	OnDrawSceneEnd.Connect([]()
	{
		static int last;
		static int lastZ;
		static int lastBlend;
		uintptr_t a = 0, b = 0;

		if (outlineEntities.empty())
		{
			return;
		}

		static auto init = ([]()
		{
			rage::fiAssetManager::GetInstance()->PushFolder("citizen:/shaderz/");
			shader = rage::grmShaderFactory::GetInstance()->Create();

			if (!shader->LoadTechnique("outlinez", nullptr, false))
			{
				shader = nullptr;
			}

			rage::fiAssetManager::GetInstance()->PopFolder();

			if (!shader)
			{
				return false;
			}

			screenSize = shader->GetParameter("ScreenSize");
			intensity = shader->GetParameter("Intensity");
			width = shader->GetParameter("Width");
			color = shader->GetParameter("Color");
			gaussSamples = shader->GetParameter("GaussSamples");

			mainTex = shader->GetParameter("MainTexSampler");
			maskTex = shader->GetParameter("MaskTexSampler");

			h = shader->GetTechnique("h");
			v = shader->GetTechnique("v");

			return true;
		})();

		if (!init)
		{
			return;
		}

		EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
		{
			lastZ = GetDepthStencilState();
			SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

			lastBlend = GetBlendState();
			SetBlendState(GetStockStateIdentifier(BlendStateDefault));

			last = *currentShader;
			*currentShader = _getTechniqueDrawName("unlit");

			// draw bucket 0 pls, not 1
			// #TODO: set via DC?
			*g_currentDrawBucket = 0;

			rage::grcTextureFactory::getInstance()->PushRenderTarget(nullptr, maskRenderTarget, nullptr, 0, true, 0);
			ClearRenderTarget(true, 0, false, 0.0f, false, 0);
		},
		&a, &b);

		for (auto& ent : outlineEntities)
		{
			if (ent)
			{
				auto drawHandler = *(CEntityDrawHandler**)((char*)ent + 72);

				if (drawHandler)
				{
					uint8_t meh[64] = { 0, 1, 0 };
					drawHandler->Draw(ent, &meh);
				}
			}
		}

		EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
		{
			rage::grcTextureFactory::getInstance()->PopRenderTarget(nullptr, nullptr);

			*currentShader = last;

			SetDepthStencilState(lastZ);
			SetBlendState(lastBlend);

			DrawScreenSpace();
		},
		&a, &b);
	});
});

static HookFunction hookFunction([]()
{
	// 1604: 0x141D8C6BC
	currentShader = hook::get_address<int*>(hook::get_pattern("85 C9 75 13 8B 05 ? ? ? ? C6", 6));

	// 1604: 0x142DD9C6C
	g_currentDrawBucket = hook::get_address<int*>(hook::get_pattern("4C 8B E9 8B 0D ? ? ? ? 89 44 24 74 B8 01", 5));
});

static InitFunction initFunctionScriptBind([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_DRAW_OUTLINE", [](fx::ScriptContext& context)
	{
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		if (!entity)
		{
			return;
		}

		bool outline = context.GetArgument<bool>(1);
		auto it = std::find_if(outlineEntities.begin(), outlineEntities.end(), [entity](const decltype(outlineEntities)::value_type& find)
		{
			return &(*find) == entity;
		});

		if (outline)
		{
			if (it == outlineEntities.end())
			{
				outlineEntities.push_back(entity);
			}
		}
		else
		{
			if (it != outlineEntities.end())
			{
				outlineEntities.erase(it);
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_MATRIX", [](fx::ScriptContext& context)
	{
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		if (!entity)
		{
			return;
		}

		int i = 1;

		auto addColumn = [&context, &i](Matrix4x4& m, int out)
		{
			for (int idx = 0; idx < 3; idx++)
			{
				((float*)&m)[out + idx] = context.GetArgument<float>(i++);
			}
		};

		Matrix4x4 newMat;
		newMat._44 = 1.0f;
		addColumn(newMat, 4);
		addColumn(newMat, 0);
		addColumn(newMat, 8);
		addColumn(newMat, 12);

		entity->UpdateTransform(newMat, true);
	});
});

#ifdef _DEBUG
static InitFunction initFunction([]()
{
	static ConsoleCommand castProbe("castSelectionProbe", []()
	{
		if (!*g_viewportGame)
		{
			return;
		}

		const auto& io = ImGui::GetIO();

		float xp = io.MousePos.x - ImGui::GetMainViewport()->Pos.x;
		float yp = io.MousePos.y - ImGui::GetMainViewport()->Pos.y;

		auto entity = DoMouseHitTest(xp, yp, HitFlags{});
		if (entity)
		{
			outlineEntities = { entity };
		}
	});

	static ConsoleCommand castProbe2("castSelectionProbe", [](int entMask)
	{
		if (!*g_viewportGame)
		{
			return;
		}

		const auto& io = ImGui::GetIO();

		float xp = io.MousePos.x - ImGui::GetMainViewport()->Pos.x;
		float yp = io.MousePos.y - ImGui::GetMainViewport()->Pos.y;

		HitFlags flags;
		flags.entityTypeMask = entMask;

		auto entity = DoMouseHitTest(xp, yp, flags);
		if (entity)
		{
			outlineEntities = { entity };
		}
	});
});
#endif
