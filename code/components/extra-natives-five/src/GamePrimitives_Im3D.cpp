#include <StdInc.h>

#include <CoreConsole.h>
#include <GamePrimitives.h>
#include <CL2LaunchMode.h>
#include <HostSharedData.h>
#include <WorldEditorControls.h>

#include <DrawCommands.h>
#include <fiAssetManager.h>

#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <im3d.h>
#include <imgui.h>

#include <ConsoleHost.h>
#include <ScriptEngine.h>

// copy draw lists for use off-thread
struct RemoteDrawList : Im3d::DrawList
{
	RemoteDrawList(const Im3d::DrawList& other)
		: Im3d::DrawList(other)
	{
	}

	RemoteDrawList(RemoteDrawList&& other) noexcept
	{
		m_vertexCount = other.m_vertexCount;
		m_primType = other.m_primType;
		m_layerId = other.m_layerId;

		m_vertexData = other.m_vertexData;
		other.m_vertexData = nullptr;
	}

	RemoteDrawList(const RemoteDrawList& other) = delete;

	~RemoteDrawList()
	{
		delete[] m_vertexData;
		m_vertexData = nullptr;
	}
};

struct RemoteDrawLists
{
	size_t count;
	std::vector<RemoteDrawList> lists;
};

struct gz
{
	rage::grmShaderFx* shader;
	int uViewport, uViewProjMatrix;
	int lines, triangles, points;

	bool inited;

	void Init();
	void Begin();
	void End();
	void Draw(const RemoteDrawLists* lists);
} gizmo;

void gz::Init()
{
	if (inited)
	{
		return;
	}

	inited = true;

	rage::fiAssetManager::GetInstance()->PushFolder("citizen:/shaderz/");
	auto shader = rage::grmShaderFactory::GetInstance()->Create();
	bool success = shader->LoadTechnique("im3d", nullptr, false);
	rage::fiAssetManager::GetInstance()->PopFolder();

	if (!success)
	{
		return;
	}

	gizmo.lines = shader->GetTechnique("lines");
	gizmo.triangles = shader->GetTechnique("triangles");
	gizmo.points = shader->GetTechnique("points");

	gizmo.uViewport = shader->GetParameter("uViewport");
	gizmo.uViewProjMatrix = shader->GetParameter("uViewProjMatrix");

	gizmo.shader = shader;
}

struct RegisteredControl
{
	RegisteredControl(const std::string& name)
		: downCmd("+" + name, [this]()
		{
			down = true;
		}),
		  upCmd("-" + name, [this]()
		  {
			  down = false;
		  })
	{
	}

	bool IsDown() const
	{
		return down;
	}

private:
	bool down = false;

	ConsoleCommand downCmd;
	ConsoleCommand upCmd;
};

void gz::Begin()
{
	Im3d::NewFrame();

	auto& ad = Im3d::GetAppData();
	auto& ctx = Im3d::GetContext();

	const auto& viewport = (*g_viewportGame)->viewport;

	DirectX::XMVECTOR sc, qu, tr;
	DirectX::XMMatrixDecompose(&sc, &qu, &tr, DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)viewport.m_inverseView));

	auto rot = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f), qu);

	static auto lastTime = std::chrono::high_resolution_clock::now().time_since_epoch();
	auto time = std::chrono::high_resolution_clock::now().time_since_epoch();

	ad.m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(time - lastTime).count() / 1000.0f;
	ad.m_viewportSize = Im3d::Vec2(GetViewportW(), GetViewportH());
	ad.m_viewOrigin = Im3d::Vec3(viewport.m_inverseView[12], viewport.m_inverseView[13], viewport.m_inverseView[14]);
	ad.m_viewDirection = Im3d::Vec3{
		DirectX::XMVectorGetX(rot),
		DirectX::XMVectorGetY(rot),
		DirectX::XMVectorGetZ(rot)
	};
	ad.m_worldUp = Im3d::Vec3(0.0f, 0.0f, 1.0f);
	ad.m_projOrtho = false;
	ad.m_projScaleY = viewport.m_projection[5] / 2.0f;
	ad.m_flipGizmoWhenBehind = false;

	lastTime = time;

	float x;
	float y;

	if (launch::IsSDKGuest())
	{
		static HostSharedData<WorldEditorControls> wec("CfxWorldEditorControls");

		x = wec->mouseX;
		y = wec->mouseY;

		ad.m_keyDown[Im3d::Action_Select] = wec->gizmoSelect;

		ctx.m_gizmoMode = (Im3d::GizmoMode)wec->gizmoMode;
		ctx.m_gizmoLocal = wec->gizmoLocal;
	}
	else
	{
		const auto& io = ImGui::GetIO();

		x = (io.MousePos.x - ImGui::GetMainViewport()->Pos.x) / (float)GetViewportW();
		y = (io.MousePos.y - ImGui::GetMainViewport()->Pos.y) / (float)GetViewportH();

		static RegisteredControl gizmoSelect("gizmoSelect");
		static RegisteredControl gizmoLocal("gizmoLocal");
		static RegisteredControl gizmoTranslation("gizmoTranslation");
		static RegisteredControl gizmoRotation("gizmoRotation");
		static RegisteredControl gizmoScale("gizmoScale");

		ad.m_keyDown[Im3d::Action_Select] = gizmoSelect.IsDown();
		ad.m_keyDown[Im3d::Action_GizmoLocal] = gizmoLocal.IsDown();
		ad.m_keyDown[Im3d::Action_GizmoTranslation] = gizmoTranslation.IsDown();
		ad.m_keyDown[Im3d::Action_GizmoRotation] = gizmoRotation.IsDown();
		ad.m_keyDown[Im3d::Action_GizmoScale] = gizmoScale.IsDown();
	}

	auto rayStart = Unproject(viewport, rage::Vec3V{ x, y, 0.0f });
	auto rayEnd = Unproject(viewport, rage::Vec3V{ x, y, 1.0f });

	ad.m_cursorRayOrigin = Im3d::Vec3{ rayStart.x, rayStart.y, rayStart.z };

	auto s = DirectX::XMVectorSet(rayStart.x, rayStart.y, rayStart.z, 1.0f);
	auto e = DirectX::XMVectorSet(rayEnd.x, rayEnd.y, rayEnd.z, 1.0f);

	auto d = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(e, s));
	ad.m_cursorRayDirection = Im3d::Vec3{
		DirectX::XMVectorGetX(d),
		DirectX::XMVectorGetY(d),
		DirectX::XMVectorGetZ(d)
	};
}

void gz::Draw(const RemoteDrawLists* lists)
{
	const auto& viewport = *rage::spdViewport::GetCurrent();

	float viewportSize[2] = {
		(float)GetViewportW(),
		(float)GetViewportH()
	};

	float ident[16] =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	SetWorldMatrix(ident);

	for (size_t i = 0, n = lists->count; i < n; ++i)
	{
		const auto& drawList = lists->lists[i];

		auto lastZ = GetDepthStencilState();
		SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

		auto lastBlend = GetBlendState();
		SetBlendState(GetStockStateIdentifier(BlendStateDefault));

		shader->SetParameter(uViewport, viewportSize, 8, 1);
		shader->SetParameter(uViewProjMatrix, viewport.m_viewProjection, 16, 4);

		rage::grcDrawMode mode;

		switch (drawList.m_primType)
		{
			case Im3d::DrawPrimitive_Points:
				shader->PushTechnique(points, true, 0);
				mode = rage::grcPointList;
				break;
			case Im3d::DrawPrimitive_Lines:
				shader->PushTechnique(lines, true, 0);
				mode = rage::grcLineList;
				break;
			case Im3d::DrawPrimitive_Triangles:
				shader->PushTechnique(triangles, true, 0);
				mode = rage::grcTriangleList;
				break;
			default:
				continue; // wtf
		}

		shader->PushPass(0);

		rage::grcBegin(mode, drawList.m_vertexCount);
		for (size_t v = 0; v < drawList.m_vertexCount; v++)
		{
			const auto& vtx = drawList.m_vertexData[v];
			auto c = vtx.m_color.v;

			rage::grcVertex(vtx.m_positionSize.x, vtx.m_positionSize.y, vtx.m_positionSize.z, vtx.m_positionSize.w, 0.0f, 0.0f, c, 0.0f, 0.0f);
		}
		rage::grcEnd();

		shader->PopPass();
		shader->PopTechnique();

		SetDepthStencilState(lastZ);
		SetBlendState(lastBlend);
	}
}

void gz::End()
{
	/*for (auto entity : fakeHitEntities)
	{
		auto transform = entity->GetTransform();
		auto m03 = transform.m[0][3];
		auto m13 = transform.m[1][3];
		auto m23 = transform.m[2][3];
		transform.m[0][3] = 0.0f;
		transform.m[1][3] = 0.0f;
		transform.m[2][3] = 0.0f;
		if (Im3d::Gizmo(Im3d::MakeId(entity), (float*)&transform))
		{
			transform.m[0][3] = m03;
			transform.m[1][3] = m13;
			transform.m[2][3] = m23;
			entity->UpdateTransform(transform, true);
		}
	}*/

	Im3d::EndFrame();

	if (!shader)
	{
		return;
	}

	auto rdl = new RemoteDrawLists();
	rdl->count = Im3d::GetDrawListCount();
	rdl->lists.reserve(rdl->count);

	for (size_t i = 0, n = Im3d::GetDrawListCount(); i < n; ++i)
	{
		RemoteDrawList drawList = Im3d::GetDrawLists()[i];
		auto newVertexData = new Im3d::VertexData[drawList.m_vertexCount];
		memcpy(newVertexData, drawList.m_vertexData, drawList.m_vertexCount * sizeof(Im3d::VertexData));

		drawList.m_vertexData = newVertexData;
		rdl->lists.push_back(std::move(drawList));
	}

	uintptr_t a = (uintptr_t)rdl;
	uintptr_t b = 0;

	EnqueueGenericDrawCommand([](uintptr_t a, uintptr_t b)
	{
		auto drawLists = (RemoteDrawLists*)a;
		gizmo.Draw(drawLists);

		delete drawLists;
	},
	&a, &b);
}

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		if (!Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			return;
		}

		gizmo.Init();
		gizmo.Begin();
	}, INT32_MIN);

	OnDrawSceneEnd.Connect([]()
	{
		gizmo.End();
	}, INT32_MAX);
});

#include <InputHook.h>

static int cursorModeRefCount;

static InitFunction initFunctionScriptBind([]()
{
	fx::ScriptEngine::RegisterNativeHandler("DRAW_GIZMO", [](fx::ScriptContext& context)
	{
		context.SetResult(Im3d::Gizmo(context.CheckArgument<const char*>(1), (float*)context.CheckArgument<float*>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("ENTER_CURSOR_MODE", [](fx::ScriptContext& context)
	{
		if (cursorModeRefCount++ == 0)
		{
			ConHost::SetCursorMode(true);
			InputHook::SetControlBypasses(0x3D, { InputHook::ControlBypass{ true, 0 }, InputHook::ControlBypass{ true, 1 }, InputHook::ControlBypass{ true, 2 }, InputHook::ControlBypass{ true, 3 }, InputHook::ControlBypass{ true, 4 } });
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("LEAVE_CURSOR_MODE", [](fx::ScriptContext& context)
	{
		if (cursorModeRefCount-- == 1)
		{
			ConHost::SetCursorMode(false);
			InputHook::SetControlBypasses(0x3D, {});
		}
	});
});
