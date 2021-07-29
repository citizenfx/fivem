#include <StdInc.h>

#include <GamePrimitives.h>

#include <Hooking.h>

#include <DrawCommands.h>

#include <fiAssetManager.h>
#include <EntitySystem.h>
#include <MinHook.h>

#include <CL2LaunchMode.h>
#include <HostSharedData.h>
#include <WorldEditorControls.h>

#include <ScriptEngine.h>

#include <im3d.h>

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#define RAGE_FORMATS_IN_GAME

#include <rmcDrawable.h>

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

namespace rage
{
using five::rmcDrawable;
}

static hook::cdecl_stub<rage::rmcDrawable*(fwArchetype*)> _getDrawable([]()
{
	return hook::get_call(hook::get_pattern("66 C1 E8 0F 40 84 C5 0F 84 ? ? 00 00 E8", 13));
});

static fwEntity* RunSelectionProbe(const rage::spdRay& ray, int nth, const HitFlags& flags)
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
	static std::vector<fwEntity*> hitEntities;
	hitEntities.clear();

	CScene::ForAllEntitiesIntersecting(
	&volume, [](fwEntity* entity, void* cxt)
	{
		//trace("entity %p -> %08x\n", (void*)entity, entity->GetArchetype()->hash);
		hitEntities.push_back(entity);
		return true;
	}, nullptr, flags.entityTypeMask, 3, 1, 8, 1);

	std::vector<std::tuple<fwEntity*, float>> hitPassedEntities;
	
	using namespace DirectX;
	auto rayStart = XMLoadFloat3((XMFLOAT3*)&ray.start);
	auto rayEnd = XMLoadFloat3((XMFLOAT3*)&ray.end);

	// test all the drawables
	for (auto& entity : hitEntities)
	{
		auto archetype = entity->GetArchetype();
		if (!archetype)
		{
			continue;
		}

		if (!archetype->HasEmbeddedCollision())
		{
			continue;
		}

		if (!flags.preciseTesting)
		{
			auto pos = entity->GetPosition();
			auto v3 = XMLoadFloat3((XMFLOAT3*)&pos);
			float fraction = XMVectorGetX(XMVector3Length(v3 - rayStart));

			hitPassedEntities.push_back({ entity, fraction });
			continue;
		}

		auto drawable = _getDrawable(archetype);
		if (!drawable)
		{
			continue;
		}

		float fraction = FLT_MAX;

		auto cleanXform = entity->GetTransform();
		cleanXform.m[0][3] = 0.0f;
		cleanXform.m[1][3] = 0.0f;
		cleanXform.m[2][3] = 0.0f;
		cleanXform.m[3][3] = 1.0f;

		auto lm = DirectX::XMLoadFloat4x4(&cleanXform);
		auto tm = DirectX::XMMatrixInverse(NULL, lm);
		auto localRayStart = DirectX::XMVector3Transform(rayStart, tm);
		auto localRayEnd = DirectX::XMVector3Transform(rayEnd, tm);
		auto localRayDir = XMVector3Normalize(localRayEnd - localRayStart);
		auto localRayDirInv = XMVectorSplatOne() / XMVector3Normalize(localRayEnd - localRayStart);

		rage::five::grmLodGroup& lodGroup = drawable->GetLodGroup();
		auto models = lodGroup.GetPrimaryModel();
		for (size_t m = 0; m < models->GetCount(); m++)
		{
			auto model = models->Get(m);
			
			auto& geoms = model->GetGeometries();
			auto gbs = model->GetGeometryBounds();

			for (size_t g = 0; g < geoms.GetCount(); g++)
			{
				auto geom = geoms.Get(g);
				auto geomBound = gbs[(geoms.GetCount() == 1) ? 0 : g + 1];
				
				// ray/bound intersection test
				// from https://tavianator.com/2015/ray_box_nan.html
				{
					float t1 = (geomBound.aabbMin.x - XMVectorGetX(localRayStart)) * XMVectorGetX(localRayDirInv);
					float t2 = (geomBound.aabbMax.x - XMVectorGetX(localRayStart)) * XMVectorGetX(localRayDirInv);

					float tmin = std::min(t1, t2);
					float tmax = std::max(t1, t2);

					t1 = (geomBound.aabbMin.y - XMVectorGetY(localRayStart)) * XMVectorGetY(localRayDirInv);
					t2 = (geomBound.aabbMax.y - XMVectorGetY(localRayStart)) * XMVectorGetY(localRayDirInv);

					tmin = std::max(tmin, std::min(t1, t2));
					tmax = std::min(tmax, std::max(t1, t2));

					t1 = (geomBound.aabbMin.z - XMVectorGetZ(localRayStart)) * XMVectorGetZ(localRayDirInv);
					t2 = (geomBound.aabbMax.z - XMVectorGetZ(localRayStart)) * XMVectorGetZ(localRayDirInv);

					tmin = std::max(tmin, std::min(t1, t2));
					tmax = std::min(tmax, std::max(t1, t2));

					if (tmax <= std::max(tmin, 0.0f))
					{
						continue;
					}
				}

				// onwards!
				auto vb = geom->GetVertexBuffer(0);
				auto ib = geom->GetIndexBuffer(0);

				auto vl = vb->GetVertices();
				auto vs = vb->GetStride();

				// in grcore FVFs we can assume vertex offset will be 0
				size_t vo = 0;

				auto getVert = [vl, vs, vo](size_t i)
				{
					return XMLoadFloat3((XMFLOAT3*)((char*)vl + (i * vs) + vo));
				};

				const uint16_t* idxs = ib->GetIndexData();
				size_t prims = ib->GetIndexCount();

				XMVECTOR e1, e2, h, s, q;
				XMVECTOR a, f, u, v;
				XMVECTOR e = DirectX::XMVectorSet(0.00001f, 0.00001f, 0.00001f, 0.00001f);
				XMVECTOR ne = DirectX::XMVectorNegate(e);

				for (size_t t = 0; t < prims; t += 3)
				{
					auto v0 = getVert(idxs[t + 0]);
					auto v1 = getVert(idxs[t + 1]);
					auto v2 = getVert(idxs[t + 2]);

					e1 = v1 - v0;
					e2 = v2 - v0;

					h = XMVector3Cross(localRayDir, e2);
					a = XMVector3Dot(e1, h);

					if (XMVector4EqualInt(XMVectorAndInt(XMVectorGreater(a, ne), XMVectorLess(a, e)), DirectX::XMVectorTrueInt()))
					{
						continue;
					}

					f = DirectX::XMVectorSplatOne() / a;
					s = localRayStart - v0;
					u = f * XMVector3Dot(s, h);

					if (XMVector4EqualInt(XMVectorOrInt(XMVectorLess(u, DirectX::XMVectorZero()), XMVectorGreater(u, DirectX::XMVectorSplatOne())), DirectX::XMVectorTrueInt()))
					{
						continue;
					}

					q = XMVector3Cross(s, e1);
					v = f * XMVector3Dot(localRayDir, q);

					if (XMVector4EqualInt(XMVectorOrInt(XMVectorLess(v, DirectX::XMVectorZero()), XMVectorGreater(u + v, DirectX::XMVectorSplatOne())), DirectX::XMVectorTrueInt()))
					{
						continue;
					}

					auto frac = f * XMVector3Dot(e2, q);

					if (XMVector4EqualInt(XMVectorGreater(frac, e), DirectX::XMVectorTrueInt()))
					{
						fraction = XMVectorGetX(frac);
						break;
					}
				}

				if (fraction < FLT_MAX)
				{
					break;
				}
			}

			if (fraction < FLT_MAX)
			{
				break;
			}
		}

		if (fraction < FLT_MAX)
		{
			hitPassedEntities.push_back({ entity, fraction });
		}
	}

	// sort the hits
	std::sort(hitPassedEntities.begin(), hitPassedEntities.end(), [](const auto& l, const auto& r)
	{
		return std::get<1>(l) < std::get<1>(r);
	});

	if (nth >= 0 && nth < hitPassedEntities.size())
	{
		return std::get<0>(hitPassedEntities[nth]);
	}

	return nullptr;
}

fwEntity* DoMouseHitTest(int mX, int mY, const HitFlags& flags)
{
	static float lx;
	static float ly;

	auto xp = mX;
	auto yp = mY;

	float x = xp / (float)GetViewportW();
	float y = yp / (float)GetViewportH();

	auto rayStart = Unproject((*g_viewportGame)->viewport, rage::Vec3V{ x, y, 0.0f });
	auto rayEnd = Unproject((*g_viewportGame)->viewport, rage::Vec3V{ x, y, 1.0f });

	rage::spdRay ray;
	ray.start = rayStart;
	ray.end = rayEnd;

	auto ptD = ((lx - xp) * (lx - xp)) + ((ly - yp) * (ly - yp));
	static int nth = 0;
	if (ptD > (8 * 8))
	{
		nth = 0;
	}

	fwEntity* hitEnt;

	for (int attempt = 0; attempt < 2; attempt++)
	{
		auto entity = RunSelectionProbe(ray, nth, flags);

		if (entity)
		{
			nth++;
			hitEnt = entity;

			break;
		}

		nth = 0;
		hitEnt = nullptr;
	}

	lx = xp;
	ly = yp;

	return hitEnt;
}

static void (*g_origDrawSceneEnd)(int);
static void DrawSceneEnd(int a1)
{
	g_origDrawSceneEnd(a1);

	OnDrawSceneEnd();
}

static void (*g_origPostFxSetupHook)(int w, int h);

void PostFxSetupHook(int w, int h)
{
	// width and height are *probably* scaled to frame scaling size
	g_origPostFxSetupHook(w, h);

	OnSetUpRenderBuffers(w, h);
}

static HookFunction hookFunction([]()
{
	g_viewportGame = hook::get_address<CViewportGame**>(hook::get_pattern("33 C0 48 39 05 ? ? ? ? 74 2E 48 8B 0D ? ? ? ? 48 85 C9 74 22", 5));

	// 1604: 0x1404F42AC
	// call to draw script 3D stuff
	{
		auto location = hook::get_pattern("81 08 08 E0 01 00 B9 FF 00 00 00 E8", 11);
		hook::set_call(&g_origDrawSceneEnd, location);
		hook::call(location, DrawSceneEnd);
	}

	// post fx setup hijack
	// (we do this as it's called both on init as well as on render restart)
	{
		MH_Initialize();

		auto location = hook::get_pattern("41 BF 01 00 00 00 BA FF FF 00 00", -0x35);
		MH_CreateHook(location, PostFxSetupHook, (void**)&g_origPostFxSetupHook);
		MH_EnableHook(location);
	}
});

fwEvent<int, int> OnSetUpRenderBuffers;
fwEvent<> OnDrawSceneEnd;
CViewportGame** g_viewportGame;

static rage::spdViewport** rage__spdViewport__sm_Current;

rage::spdViewport* rage::spdViewport::GetCurrent()
{
	return *rage__spdViewport__sm_Current;
}

static HookFunction hookFunctionSafe([]()
{
	rage__spdViewport__sm_Current = hook::get_address<rage::spdViewport**>(hook::get_pattern("48 8B 3D ? ? ? ? 40 8A F2 48 8B D9 75 14", 3));
});

#include <imgui.h>

static hook::cdecl_stub<uint32_t(fwEntity*)> getScriptGuidForEntity([]()
{
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
});

static InitFunction initFunctionScript([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SELECT_ENTITY_AT_CURSOR", [](fx::ScriptContext& context)
	{
		float xp;
		float yp;

		if (launch::IsSDKGuest())
		{
			static HostSharedData<WorldEditorControls> wec("CfxWorldEditorControls");

			xp = wec->mouseX * GetViewportW();
			yp = wec->mouseY * GetViewportH();
		}
		else
		{
			const auto& io = ImGui::GetIO();

			xp = io.MousePos.x - ImGui::GetMainViewport()->Pos.x;
			yp = io.MousePos.y - ImGui::GetMainViewport()->Pos.y;
		}

		HitFlags hf;
		hf.entityTypeMask = context.GetArgument<int>(0);
		hf.preciseTesting = context.GetArgument<bool>(1);

		auto entity = DoMouseHitTest(xp, yp, hf);
		if (entity)
		{
			context.SetResult(getScriptGuidForEntity(entity));
			return;
		}

		context.SetResult(0);
	});

	fx::ScriptEngine::RegisterNativeHandler("SELECT_ENTITY_AT_POS", [](fx::ScriptContext& context)
	{
		float xp = context.GetArgument<float>(0) * GetViewportW();
		float yp = context.GetArgument<float>(1) * GetViewportH();

		HitFlags hf;
		hf.entityTypeMask = context.GetArgument<int>(2);
		hf.preciseTesting = context.GetArgument<bool>(3);

		auto entity = DoMouseHitTest(xp, yp, hf);
		if (entity)
		{
			context.SetResult(getScriptGuidForEntity(entity));
			return;
		}

		context.SetResult(0);
	});
});
