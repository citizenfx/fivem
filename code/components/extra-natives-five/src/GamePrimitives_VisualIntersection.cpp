#include <StdInc.h>
#include <Hooking.h>
#include <CoreConsole.h>

#include <DrawCommands.h>

#include <EntitySystem.h>

// WIP
#ifdef _DEBUG
namespace rage
{
struct alignas(16) Vec3V
{
	float x;
	float y;
	float z;
	float pad;

	Vec3V()
		: x(0), y(0), z(0), pad(0)
	{
	
	}

	Vec3V(float x, float y, float z)
		: x(x), y(y), z(z), pad(NAN)
	{
	
	}
};

struct alignas(16) Vec4V
{
	float x;
	float y;
	float z;
	float w;
};

struct spdAABB
{
	rage::Vec3V mins;
	rage::Vec3V maxs;
};

struct spdSphere
{
	// xyz = center
	// w   = radius
	Vec4V sphere;
};

// fake name
struct spdRay
{
	rage::Vec3V start;
	rage::Vec3V end;
};

enum class eSearchVolumeType : int
{
	SphereContains,
	SphereIntersect,
	SphereIntersectPrecise,
	AabbContainsAabb,
	AabbContainsSphere,
	AabbIntersectsSphere,
	AabbIntersectsAabb,
	RayIntersectsAabb
};

struct fwSearchVolume
{
	spdAABB aabb;
	spdSphere sphere;
	spdRay ray;
	eSearchVolumeType type;
};
}

static hook::cdecl_stub<void(rage::fwSearchVolume* volume, bool (*callback)(fwEntity*, void*), void* context, int a4, int a5, int a6, int a7, int a8)> _scene_forAllEntitiesIntersecting([]()
{
	return hook::get_pattern("41 F7 C1 C3 FF FF FF 75 04", -12);
});

class CScene
{
public:
	// entityTypes mask: (1 << [fwEntity+40 type])
	static void ForAllEntitiesIntersecting(rage::fwSearchVolume* volume, bool(*callback)(fwEntity*, void*), void* context, int entityTypes, int a5, int a6, int a7, int a8)
	{
		return _scene_forAllEntitiesIntersecting(volume, callback, context, entityTypes, a5, a6, a7, a8);
	}
};

static std::vector<fwEntity*> fakeHitEntities;

static fwEntity* RunSelectionProbe(const rage::spdRay& ray)
{
	rage::fwSearchVolume volume;
	volume.ray = ray;
	volume.sphere.sphere.x = ray.start.x;
	volume.sphere.sphere.y = ray.start.y;
	volume.sphere.sphere.z = ray.start.z;
	volume.sphere.sphere.w = 100.f;
	volume.aabb.mins.x = ray.start.x - 150.f;
	volume.aabb.mins.y = ray.start.y - 150.f;
	volume.aabb.mins.z = ray.start.z - 150.f;
	volume.aabb.maxs.x = ray.start.x + 150.f;
	volume.aabb.maxs.y = ray.start.y + 150.f;
	volume.aabb.maxs.z = ray.start.z + 150.f;
	volume.type = rage::eSearchVolumeType::RayIntersectsAabb;

	// fake list of entities to highlight or so
	fakeHitEntities.clear();

	CScene::ForAllEntitiesIntersecting(
	&volume, [](fwEntity* entity, void* cxt)
	{
		trace("entity %p -> %08x\n", (void*)entity, entity->GetArchetype()->hash);
		// this doesn't add ref, bad
		fakeHitEntities.push_back(entity);
		return true;
	}, nullptr, 2, 3, 1, 8, 1);

	// #TODO: real ray->poly hit testing (on visual part) of referenced drawables

	return nullptr;
}

struct grcViewport
{
	float m_mat1[16];
	float m_mat2[16];
	float m_viewProjection[16];
	float m_inverseView[16];
	char m_pad[64];
	float m_projection[16];
};

struct CViewportGame
{
public:
	virtual ~CViewportGame() = 0;

private:
	char m_pad[8];

public:
	grcViewport viewport;
};

static CViewportGame** g_viewportGame;

static rage::Vec3V Unproject(const grcViewport& viewport, const rage::Vec3V& viewPos)
{
	using namespace DirectX;
	
	auto invVP = XMMatrixInverse(NULL, XMLoadFloat4x4((const XMFLOAT4X4*)viewport.m_viewProjection));
	auto inVec = XMVectorSet((viewPos.x * 2.0f) - 1.0f, ((1.0 - viewPos.y) * 2.0f) - 1.0f, viewPos.z, 1.0f);
	auto outCoord = XMVector3TransformCoord(inVec, invVP);

	return {
		XMVectorGetX(outCoord),
		XMVectorGetY(outCoord),
		XMVectorGetZ(outCoord)
	};
}

static InitFunction initFunction([]()
{
	static ConsoleCommand castProbe("spdRayProbe", []()
	{
		if (!*g_viewportGame)
		{
			return;
		}

		// #TODO: actual game res, real probe positions
		POINT pt;
		GetCursorPos(&pt);
		float x = pt.x / 5120.0f;
		float y = pt.y / 1440.0f;

		auto rayStart = Unproject((*g_viewportGame)->viewport, rage::Vec3V{ x, y, 0.0f });
		auto rayEnd = Unproject((*g_viewportGame)->viewport, rage::Vec3V{ x, y, 1.0f });

		rage::spdRay ray;
		ray.start = rayStart;
		ray.end = rayEnd;

		auto entity = RunSelectionProbe(ray);
		trace("hit: %p\n", (void*)entity);
	});
});

static void (*g_origDrawSceneEnd)(int);

struct CEntityDrawHandler
{
	virtual ~CEntityDrawHandler() = 0;
	virtual void m_8() = 0;
	virtual void Draw(fwEntity* entity, void* a3) = 0;
};

namespace rage
{
struct fiAssetManager
{
	static fiAssetManager* GetInstance();

	void PushFolder(const char* folder);

	void PopFolder();
};

fiAssetManager* fiAssetManager::GetInstance()
{
	static auto instance = hook::get_address<fiAssetManager*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 8B DF 8D 77 01", 3));
	return instance;
}

static hook::thiscall_stub<void(fiAssetManager*, const char*)> _pushFolder([]()
{
	return hook::get_call(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 8B DF 8D 77 01", 7));
});

static hook::thiscall_stub<void(fiAssetManager*)> _popFolder([]()
{
	return hook::get_call(hook::get_pattern("EB 0A C7 83 ? 00 00 00 05 00 00 00", 0x13));
});

void fiAssetManager::PushFolder(const char* folder)
{
	return _pushFolder(this, folder);
}

void fiAssetManager::PopFolder()
{
	return _popFolder(this);
}
}

static rage::grcRenderTarget* maskRenderTarget;

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

static void Draw()
{
	static int screenSize, intensity, width, color, gaussSamples;
	static int mainTex, maskTex;
	static int h, v;
	static rage::grmShaderFx* shader;

	static auto init = ([]()
	{
		rage::fiAssetManager::GetInstance()->PushFolder("citizen:/shaderz/");
		shader = rage::grmShaderFactory::GetInstance()->Create();
		assert(shader->LoadTechnique("outlinez", nullptr, false));
		rage::fiAssetManager::GetInstance()->PopFolder();

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

	if (!fakeHitEntities.empty())
	{
		// #TODO: actual game res
		float screenSizeF[2] = { 1.0f / 5120.0f, 1.0f / 1440.0f };
		shader->SetParameter(screenSize, screenSizeF, 8, 1);

		int widthF = 30;
		shader->SetParameter(width, &widthF, 4, 1);

		float colorF[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
		shader->SetParameter(color, colorF, 16, 1);

		// #TODO: don't recompute all the time
		float gaussSamplesF[32] = { 0.f };

		for (int i = 0; i < widthF; i++)
		{
			gaussSamplesF[i] = Gauss((float)i, widthF * 0.5f);
		}

		shader->SetParameter(gaussSamples, gaussSamplesF, 16, (32 * 4) / 16);

		float intensityF = 55.f;
		shader->SetParameter(intensity, &intensityF, 4, 1);

		shader->SetSampler(maskTex, maskRenderTarget);
		shader->SetSampler(mainTex, maskRenderTarget);

		static rage::grcRenderTarget* tempRenderTarget;
		if (!tempRenderTarget)
		{
			// #TODO: actual game res
			tempRenderTarget = CreateRenderTarget(0, "outlineTempRT", 3, 5120, 1440, 32, nullptr, true, tempRenderTarget);
		}

		rage::grcTextureFactory::getInstance()->PushRenderTarget(nullptr, tempRenderTarget, nullptr, 0, true, 0);
		
		float nah[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		ID3D11RenderTargetView* rt;
		GetD3D11DeviceContext()->OMGetRenderTargets(1, &rt, nullptr);
		GetD3D11DeviceContext()->ClearRenderTargetView(rt, nah);

		if (rt)
		{
			rt->Release();
		}

		auto lastZ = GetDepthStencilState();
		SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

		auto lastBlend = GetBlendState();
		SetBlendState(GetStockStateIdentifier(BlendStateDefault));

		auto draw = []()
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
		draw();
		shader->PopPass();
		shader->PopTechnique();

		rage::grcTextureFactory::getInstance()->PopRenderTarget(nullptr, nullptr);

		shader->SetSampler(mainTex, tempRenderTarget);

		shader->PushTechnique(v, true, 0);
		shader->PushPass(0);
		draw();
		shader->PopPass();
		shader->PopTechnique();

		SetDepthStencilState(lastZ);
		SetBlendState(lastBlend);
	}
}

static hook::cdecl_stub<int(const char*)> _getTechniqueDrawName([]()
{
	return hook::get_pattern("E8 ? ? ? ? 33 DB 48 8D 4C 24 20 44 8D", -0x11);
});

static void DrawSceneEnd(int a1)
{
	g_origDrawSceneEnd(a1);

	static int last;
	static int lastZ;
	static int lastBlend;
	uintptr_t a = 0, b = 0;

	if (fakeHitEntities.empty())
	{
		return;
	}

	EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
	{
		// #TODO: should *actually* set these up in the usual buffer (re)create function for resizing
		if (!maskRenderTarget)
		{
			// #TODO: actual game res
			maskRenderTarget = CreateRenderTarget(0, "outlineMaskRT", 3, 5120, 1440, 32, nullptr, true, maskRenderTarget);
		}

		lastZ = GetDepthStencilState();
		SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

		lastBlend = GetBlendState();
		SetBlendState(GetStockStateIdentifier(BlendStateNoBlend));

		last = *currentShader;
		*currentShader = _getTechniqueDrawName("unlit");

		// draw bucket 0 pls, not 1
		// #TODO: set via DC?
		*g_currentDrawBucket = 0;

		rage::grcTextureFactory::getInstance()->PushRenderTarget(nullptr, maskRenderTarget, nullptr, 0, true, 0);
		ClearRenderTarget(true, 0, true, 0.0f, true, 0);

		// ?? as ClearRenderTarget is no-op in V
		// -> 0x1412EE818 (1604)?
		float nah[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		ID3D11RenderTargetView* rt;
		GetD3D11DeviceContext()->OMGetRenderTargets(1, &rt, nullptr);
		GetD3D11DeviceContext()->ClearRenderTargetView(rt, nah);

		if (rt)
		{
			rt->Release();
		}
	},
	&a, &b);

	// storing like this is very unsafe (entity deletion)
	for (auto& ent : fakeHitEntities)
	{
		auto drawHandler = *(CEntityDrawHandler**)((char*)ent + 72);

		if (drawHandler)
		{
			uint8_t meh[64] = { 0, 1, 0 };
			drawHandler->Draw(ent, &meh);
		}
	}

	EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
	{
		rage::grcTextureFactory::getInstance()->PopRenderTarget(nullptr, nullptr);

		*currentShader = last;

		SetDepthStencilState(lastZ);
		SetBlendState(lastBlend);

		Draw();
	},
	&a, &b);
}

static HookFunction hookFunction([]()
{
	// 1604: 0x141D8C6BC
	currentShader = hook::get_address<int*>(hook::get_pattern("85 C9 75 13 8B 05 ? ? ? ? C6", 6));

	// 1604: 0x142DD9C6C
	g_currentDrawBucket = hook::get_address<int*>(hook::get_pattern("4C 8B E9 8B 0D ? ? ? ? 89 44 24 74 B8 01", 5));

	g_viewportGame = hook::get_address<CViewportGame**>(hook::get_pattern("33 C0 48 39 05 ? ? ? ? 74 2E 48 8B 0D ? ? ? ? 48 85 C9 74 22", 5));

	// 1604: 0x1404F42AC
	// call to draw script 3D stuff
	{
		auto location = hook::get_pattern("81 08 08 E0 01 00 B9 FF 00 00 00 E8", 11);
		hook::set_call(&g_origDrawSceneEnd, location);
		hook::call(location, DrawSceneEnd);
	}
});
#endif
