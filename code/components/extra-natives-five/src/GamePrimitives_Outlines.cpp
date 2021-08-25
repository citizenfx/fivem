#include <StdInc.h>
#include <GamePrimitives.h>

#include <fiAssetManager.h>
#include <CfxRGBA.h>

#include <Hooking.h>

#include <CoreConsole.h>
#include <imgui.h>

#include <ScriptEngine.h>

#include "EntityExtensions.h"

static std::vector<fwEntity*> outlineEntities;

inline static decltype(outlineEntities)::iterator FindOutlineEntityByEntity(const fwEntity* entity)
{
	return std::find(outlineEntities.begin(), outlineEntities.end(), entity);
}

struct CEntityDrawHandler
{
	virtual ~CEntityDrawHandler() = 0;
	virtual void m_8() = 0;
	virtual void Draw(fwEntity* entity, void* a3) = 0;
};

static rage::grcRenderTarget *maskRenderTarget, *tempRenderTarget;

// for entity prototypes
static int* currentShader;
static int* g_currentDrawBucket;

static CRGBA outlineColor{ 255, 0, 255, 255 };

class OutlineRenderer
{
public:
	bool Initialized()
	{
		return initialized;
	}

	virtual void Setup() = 0;
	virtual void DrawScreenSpace() = 0;

	void Init()
	{
		if (LoadShader(GetShaderName()))
		{
			Setup();
		}
	}

protected:
	virtual const char* GetShaderName() = 0;

	inline void DrawQuad()
	{
		rage::grcBegin(4, 4);

		uint32_t color = 0xffffffff;

		rage::grcVertex(-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 1.0f);
		rage::grcVertex(1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 1.0f, 1.0f);
		rage::grcVertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		rage::grcVertex(1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 1.0f, 0.0f);

		rage::grcEnd();
	}

	inline void DrawQuadUsingTechnique(int technique)
	{
		shader->PushTechnique(technique, true, 0);
		shader->PushPass(0);

		DrawQuad();

		shader->PopPass();
		shader->PopTechnique();
	}

	inline void SetScreenSize(int parameter)
	{
		float screenSizeF[2] = { 1.0f / (float)GetViewportW(), 1.0f / (float)GetViewportH() };
		shader->SetParameter(parameter, screenSizeF, 8, 1);
	}

	inline void StoreState()
	{
		storedStencilState = GetDepthStencilState();
		SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

		storedBlendState = GetBlendState();
		SetBlendState(GetStockStateIdentifier(BlendStateDefault));
	}

	inline void RestoreState()
	{
		SetDepthStencilState(storedStencilState);
		SetBlendState(storedBlendState);
	}

	inline void PushRenderTarget(rage::grcRenderTarget* renderTarget)
	{
		rage::grcTextureFactory::getInstance()->PushRenderTarget(nullptr, renderTarget, nullptr, 0, true, 0);
	}

	inline void PopRenderTarget()
	{
		rage::grcTextureFactory::getInstance()->PopRenderTarget(nullptr, nullptr);
	}

	inline void SetColorNoAlpha(int parameter)
	{
		float colorF[4] = {
			outlineColor.red / 255.0f,
			outlineColor.green / 255.0f,
			outlineColor.blue / 255.0f,
			1.0f
		};
		
		SetColor(parameter, colorF);
	}

	inline void SetColor(int parameter)
	{
		float colorF[4] = {
			outlineColor.red / 255.0f,
			outlineColor.green / 255.0f,
			outlineColor.blue / 255.0f,
			outlineColor.alpha / 255.0f,
		};
		
		SetColor(parameter, colorF);
	}

	inline void SetColor(int parameter, const float* colorF)
	{
		shader->SetParameter(parameter, colorF, 4 * sizeof(float), 1);
	}

	bool LoadShader(const char* name)
	{
		rage::fiAssetManager::GetInstance()->PushFolder("citizen:/shaderz/");
		shader = rage::grmShaderFactory::GetInstance()->Create();

		if (!shader->LoadTechnique(name, nullptr, false))
		{
			shader = nullptr;
		}

		rage::fiAssetManager::GetInstance()->PopFolder();

		initialized = shader != nullptr;

		return initialized;
	}

protected:
	bool initialized = false;
	rage::grmShaderFx* shader;

private:
	int storedStencilState, storedBlendState;
};

class GaussOutlineRenderer : public OutlineRenderer
{
protected:
	virtual const char* GetShaderName() override
	{
		return "outlinez";
	}

public:
	virtual void Setup() override
	{
		screenSize = shader->GetParameter("ScreenSize");
		intensity = shader->GetParameter("Intensity");
		width = shader->GetParameter("Width");
		color = shader->GetParameter("Color");
		gaussSamples = shader->GetParameter("GaussSamples");

		mainTex = shader->GetParameter("MainTexSampler");
		maskTex = shader->GetParameter("MaskTexSampler");

		h = shader->GetTechnique("h");
		v = shader->GetTechnique("v");

		for (int i = 0; i < m_width; i++)
		{
			m_gaussSamples[i] = Gauss((float)i, m_width * 0.5f);
		}
	}

	virtual void DrawScreenSpace() override
	{
		SetScreenSize(screenSize);
		SetColorNoAlpha(color);

		shader->SetParameter(width, &m_width, sizeof(m_width), 1);
		shader->SetParameter(intensity, &m_intensity, sizeof(m_intensity), 1);
		shader->SetParameter(gaussSamples, m_gaussSamples, sizeof(m_gaussSamples), 1);

		shader->SetSampler(maskTex, maskRenderTarget);
		shader->SetSampler(mainTex, maskRenderTarget);

		StoreState();

		PushRenderTarget(tempRenderTarget);
		ClearRenderTarget(true, 0, false, 0.0f, false, 0);

		DrawQuadUsingTechnique(h);

		PopRenderTarget();


		shader->SetSampler(mainTex, tempRenderTarget);

		DrawQuadUsingTechnique(v);


		RestoreState();
	}

	static float Gauss(float x, float stdDev)
	{
		auto stdDev2 = stdDev * stdDev * 2;
		auto a = 1 / sqrtf(3.14159265358979323846 * stdDev2);
		auto gauss = a * powf(2.71828182845904523536, -x * x / stdDev2);

		return gauss;
	}

private:
	float m_intensity = 55.f;
	float m_gaussSamples[32] = { .0f };
	int m_width = 30;

	int screenSize, intensity, width, color, gaussSamples;

	int mainTex, maskTex;

	int h, v;
};

class FirmOutlineRenderer : public OutlineRenderer
{
protected:
	virtual const char* GetShaderName() override
	{
		return "outlinez_firm";
	}

public:
	virtual void Setup() override
	{
		screenSize = shader->GetParameter("ScreenSize");
		width = shader->GetParameter("Width");
		color = shader->GetParameter("Color");

		mainTex = shader->GetParameter("MainTexSampler");
		maskTex = shader->GetParameter("MaskTexSampler");

		h = shader->GetTechnique("h");
		v = shader->GetTechnique("v");
	}

	virtual void DrawScreenSpace() override
	{
		SetScreenSize(screenSize);
		SetColor(color);

		shader->SetParameter(width, &m_width, sizeof(m_width), 1);

		shader->SetSampler(maskTex, maskRenderTarget);
		shader->SetSampler(mainTex, maskRenderTarget);

		StoreState();

		PushRenderTarget(tempRenderTarget);
		ClearRenderTarget(true, 0, false, 0.0f, false, 0);

		DrawQuadUsingTechnique(h);

		PopRenderTarget();


		shader->SetSampler(mainTex, tempRenderTarget);

		DrawQuadUsingTechnique(v);


		RestoreState();
	}

private:
	int m_width = 2;

	int screenSize, width, color;

	int mainTex, maskTex;

	int h, v;
};

class MaskRenderer : public OutlineRenderer
{
protected:
	virtual const char* GetShaderName() override
	{
		return "outlinez_mask";
	}

public:
	virtual void Setup() override
	{
		screenSize = shader->GetParameter("ScreenSize");
		color = shader->GetParameter("Color");

		maskTex = shader->GetParameter("MaskTexSampler");

		v = shader->GetTechnique("v");
	}

	virtual void DrawScreenSpace() override
	{
		SetScreenSize(screenSize);
		SetColor(color);

		shader->SetSampler(maskTex, maskRenderTarget);

		StoreState();

		DrawQuadUsingTechnique(v);

		RestoreState();
	}

private:
	int screenSize, color;

	int maskTex;

	int v;
};

static int g_renderer = 0;
static std::vector<OutlineRenderer*> g_renderers = {
	new GaussOutlineRenderer(),
	new FirmOutlineRenderer(),
	new MaskRenderer(),
};

inline static OutlineRenderer* GetRenderer()
{
	auto renderer = g_renderers[g_renderer];

	if (renderer->Initialized()) {
		return renderer;
	}

	for (auto rendererVariant : g_renderers) {
		if (rendererVariant->Initialized()) {
			return rendererVariant;
		}
	}

	return nullptr;
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
			for (auto renderer : g_renderers)
			{
				renderer->Init();
			}

			return true;
		})();

		if (!GetRenderer())
		{
			return;
		}

		EnqueueGenericDrawCommand([](uintptr_t bucket, uintptr_t)
		{
			rage::grcTextureFactory::getInstance()->PushRenderTarget(nullptr, maskRenderTarget, nullptr, 0, true, 0);
			ClearRenderTarget(true, 0, false, 0.0f, false, 0);
		},
		&a, &b);

		for (int bucket = 0; bucket < 4; bucket++)
		{
			a = bucket;

			EnqueueGenericDrawCommand([](uintptr_t bucket, uintptr_t)
			{
				lastZ = GetDepthStencilState();
				SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

				lastBlend = GetBlendState();
				SetBlendState(GetStockStateIdentifier(BlendStateDefault));

				last = *currentShader;
				*currentShader = _getTechniqueDrawName("unlit");

				// draw bucket 0 pls, not 1
				// #TODO: set via DC?
				*g_currentDrawBucket = bucket;
			},
			&a, &b);

			for (auto ent : outlineEntities)
			{
				if (ent)
				{
					auto drawHandler = *(CEntityDrawHandler**)((char*)ent + 72);

					// support smooth transition of outlined dummy to instantiated
					if (auto ext = ent->GetExtension<InstantiatedObjectRefExtension>())
					{
						if (auto instantiatedEntity = ext->GetObjectRef())
						{
							if (auto newDrawHandler = *(CEntityDrawHandler**)((char*)instantiatedEntity + 72))
							{
								drawHandler = newDrawHandler;
								ent = instantiatedEntity;
							}
						}
					}

					if (drawHandler)
					{
						uint8_t meh[64] = { 0, 1, 0 };
						drawHandler->Draw(ent, &meh);
					}
				}
			}

			EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
			{
				*currentShader = last;

				SetDepthStencilState(lastZ);
				SetBlendState(lastBlend);
			},
			&a, &b);
		}

		EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
		{
			rage::grcTextureFactory::getInstance()->PopRenderTarget(nullptr, nullptr);

			auto renderer = GetRenderer();
			if (renderer)
			{
				renderer->DrawScreenSpace();
			}
		},
		&a, &b);
	});
});


// this extension will remove it's entity from the outlineEntities list if entity gets removed
class OutlineSentinelExtension : public rage::fwExtension
{
public:
	OutlineSentinelExtension()
	{
	}
	virtual ~OutlineSentinelExtension()
	{
		if (ref)
		{
			if (auto it = FindOutlineEntityByEntity(ref); it != outlineEntities.end())
			{
				outlineEntities.erase(it);
			}
		}
	}

	virtual int GetExtensionId() const override
	{
		return GetClassId();
	}

	static int GetClassId()
	{
		return (int)EntityExtensionClassId::OutlineSentinel;
	}

	void SetEntityRef(fwEntity* entity)
	{
		ref = entity;
	}

	void RemoveEntityRef()
	{
		ref = nullptr;
	}

private:
	fwEntity* ref = nullptr;
};

static HookFunction hookFunction([]()
{
	// 1604: 0x141D8C6BC
	currentShader = hook::get_address<int*>(hook::get_pattern("85 C9 75 13 8B 05 ? ? ? ? C6", 6));

	// 1604: 0x142DD9C6C
	g_currentDrawBucket = hook::get_address<int*>(hook::get_pattern("4C 8B E9 8B 0D ? ? ? ? 89 44 24 74 B8 01", 5));
});

static InitFunction initFunctionScriptBind([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_DRAW_OUTLINE_COLOR", [](fx::ScriptContext& context)
	{
		outlineColor.red = std::clamp(context.GetArgument<int>(0), 0, 255);
		outlineColor.green = std::clamp(context.GetArgument<int>(1), 0, 255);
		outlineColor.blue = std::clamp(context.GetArgument<int>(2), 0, 255);
		outlineColor.alpha = std::clamp(context.GetArgument<int>(3), 0, 255);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_DRAW_OUTLINE_SHADER", [](fx::ScriptContext& context)
	{
		int shader = std::clamp(context.GetArgument<int>(0), 0, (int)(g_renderers.size() - 1));

		g_renderer = shader;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_DRAW_OUTLINE", [](fx::ScriptContext& context)
	{
		int entityHandle = context.GetArgument<int>(0);
		bool outline = context.GetArgument<bool>(1);

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(entityHandle);
		if (!entity)
		{
			return;
		}

		auto it = FindOutlineEntityByEntity(entity);

		if (outline)
		{
			if (it == outlineEntities.end())
			{
				auto outlineSentinel = entity->GetExtension<OutlineSentinelExtension>();

				if (!outlineSentinel)
				{
					outlineSentinel = new OutlineSentinelExtension();
					entity->AddExtension(outlineSentinel);
				}

				outlineSentinel->SetEntityRef(entity);

				outlineEntities.push_back(entity);
			}
		}
		else
		{
			if (it != outlineEntities.end())
			{
				if (auto ext = entity->GetExtension<OutlineSentinelExtension>())
				{
					ext->RemoveEntityRef();
				}

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

		// If dummy - update its real object, if available
		if (auto ext = entity->GetExtension<InstantiatedObjectRefExtension>())
		{
			if (auto instantiatedEntity = ext->GetObjectRef())
			{
				instantiatedEntity->UpdateTransform(newMat, true, true);
			}
		}
	});
});

#ifdef _DEBUG

#include <imgui.h>

#include <CoreConsole.h>
#include <ConsoleHost.h>
#include <Streaming.h>

static InitFunction initFunction([]()
{
	static bool cl_drawOutlinesDebugEnabled = false;

	static ConVar<bool> drawFps("cl_drawOutlinesDebug", ConVar_Archive, false, &cl_drawOutlinesDebugEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || cl_drawOutlinesDebugEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!cl_drawOutlinesDebugEnabled)
		{
			return;
		}

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x + 10, ImGui::GetMainViewport()->Pos.y + 150), 0, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("DrawOutlinesDebug", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("%d outline entities", outlineEntities.size());
						
			for (auto ent : outlineEntities)
			{
				if (ent)
				{
					std::string name = fmt::sprintf("%x", ent->GetArchetype()->hash);

					ImGui::Text("0x%lx (%s):", (uint64_t)ent, name);

					if (auto ext = ent->GetExtension<InstantiatedObjectRefExtension>())
					{
						ImGui::Text("--> InstatiatedObjectRefExtension, ref (0x%lx) is valid: %d", (uint64_t)ext->GetObjectRef(), ext->GetObjectRef() != nullptr);
					}
					if (auto ext = ent->GetExtension<DummyObjectRefExtension>())
					{
						ImGui::Text("--> DummyObjectRefExtension, ref (0x%lx) is valid: %d", (uint64_t)ext->GetObjectRef(), ext->GetObjectRef() != nullptr);
					}
				}
				else
				{
					ImGui::Text("%lx (null): ref is null", (uint64_t)ent);
				}
			}
		}

		ImGui::PopStyleVar();
		ImGui::End();
	});

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
