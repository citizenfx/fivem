#include <StdInc.h>

#include <atArray.h>

#ifdef CreateDirectory
#undef CreateDirectory
#define CreateDirectory CreateDirectoryW

#undef RemoveDirectory
#define RemoveDirectory RemoveDirectoryW
#endif

#include <DrawCommands.h>
#include <grcTexture.h>
#include <VFSManager.h>
#include <VFSRagePackfile7.h>

#include <CefOverlay.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include "common.h"
#include "Frontend.h"
#include "Hud.h"
#include "Clock.h"
#include "Streaming.h"

DLL_IMPORT ID3D11Device* GetRawD3D11Device();

struct Re3InitStruct
{
	void* context;
	void* backBuffer;
	void* backBufferDS;
	bool* inited;
};

struct grcRenderTargetDX11
{
	void* vtbl;
	char pad[48]; // +8
	ID3D11Resource* resource; // +56
	char pad2[32];
	ID3D11Resource* resource2; // +96
	char pad3[32];
	ID3D11ShaderResourceView* srv; // +136
	char pad4[8];
	ID3D11UnorderedAccessView* uav; // +152
	atArray<ID3D11View*> views; // +160
};

int Re3Main(Re3InitStruct* data);

void hackSetVideoMode(int w, int h);

extern fwEvent<> OnScript;

#include "Camera.h"
#include "Ped.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "World.h"

bool bWantsGameplay;

static InitFunction initFunction([]()
{
	OnScript.Connect([]()
	{
		static bool inited;
		static bool initedGameplay;

		if (!nui::HasMainUI())
		{
			inited = false;
			bWantsGameplay = false;
			initedGameplay = false;
			nui::SetHideCursor(false);
		}
		else
		{
			static CVector source;
			static CVector front;

			if (!inited && !bWantsGameplay)
			{
				CVector sources[] = {
					// tsq
					{ 120.029823f, -1097.096680f, 31.810095f },
					
					// park
					{ 81.171021f, -605.553040f, 50.588715f },

					// cxn
					{ 281.311951f, -318.440887f, 42.782726f },

					// air
					{ -683.817810f, -490.591125f, 45.710773f },

					// dam
					{ -927.047913f, 514.431152f, 100.942825f },

					// dock
					{ 1443.914185f, -824.889954f, 62.230587f },
				};

				CVector fronts[] = {
					{ -0.867821f, 0.490475f, -0.079503f },
					{ -0.461511f, -0.847191f, -0.263201f },
					{ 0.792552f, -0.510112f, -0.334136f },
					{ -0.800066f, -0.500873f, -0.330184f },
					{ -0.936004f, -0.215049f, -0.278659f },
					{ 0.507365f, -0.758158f, -0.409607f },
				};

				srand(time(NULL));
				auto idx = rand() % std::size(sources);

				source = sources[idx];
				front = fronts[idx];

				CPlayerPed* ped = CWorld::Players[CWorld::PlayerInFocus].m_pPed;
				
				ped->SetPosition(source);
				ped->bUsesCollision = false;
				ped->bCollisionProof = true;
				ped->bIsVisible = false;

				SYSTEMTIME t;
				GetLocalTime(&t);

				CHud::m_Wants_To_Draw_Hud = false;
				CClock::SetGameClock((t.wHour * 4) % 24, t.wMinute);

				if (initedGameplay)
				{
					nui::PostFrameMessage("mpMenu", R"({ "type": "exitGameplay" })");
					nui::SetHideCursor(false);
				}

				inited = true;
				initedGameplay = false;
			}

			if (!bWantsGameplay)
			{
				TheCamera.ActiveCam = 2;

				DebugCamMode = 999;
				TheCamera.Cams[TheCamera.ActiveCam].Mode = 999;

				TheCamera.Cams[TheCamera.ActiveCam].Source = source;
				TheCamera.Cams[TheCamera.ActiveCam].Front = front;
				TheCamera.Cams[TheCamera.ActiveCam].Up = CVector(0.0f, 0.0f, 1.0f);
			}

			if (bWantsGameplay && !initedGameplay)
			{
				TheCamera.ActiveCam = 0;
				DebugCamMode = 0;
				TheCamera.Cams[TheCamera.ActiveCam].Mode = CCam::MODE_FOLLOWPED;

				CPlayerPed* ped = CWorld::Players[CWorld::PlayerInFocus].m_pPed;

				CTimer::Stop();
				CStreaming::LoadScene({ 500.9f, -389.2f, 21.2f });
				CTimer::Update();

				ped->SetPosition({ 500.9f, -389.2f, 21.2f });
				ped->bUsesCollision = true;
				ped->bCollisionProof = true;
				ped->bIsVisible = true;

				CHud::m_Wants_To_Draw_Hud = true;

				inited = false;
				initedGameplay = true;
			}
		}
	});

	nui::OnInvokeNative.Connect([](const wchar_t* type, const wchar_t* arg)
	{
		if (wcscmp(type, L"enterGameplay") == 0)
		{
			nui::SetHideCursor(true);
			bWantsGameplay = true;
		}
	});

	OnPostFrontendRender.Connect([]()
	{
		static rage::grcTexture* gTex = NULL;

		static bool inited = false;

		static auto initOnce = ([]()
		{
			fwRefContainer<vfs::RagePackfile7> pack = new vfs::RagePackfile7();
			if (!pack->OpenArchive("citizen:/re3.rpf"))
			{
				return 0;
			}

			vfs::Mount(pack, "re3:/");

			bgfx::renderFrame();

			rage::grcManualTextureDef textureDef;
			memset(&textureDef, 0, sizeof(textureDef));
			textureDef.isStaging = 0;
			textureDef.arraySize = 1;
			textureDef.isRenderTarget = 1;

			int w, h;
			GetGameResolution(w, h);

			hackSetVideoMode(w, h);

			auto tf = rage::grcTextureFactory::getInstance();
			auto tex = tf->createManualTexture(w, h, 2, NULL, true, &textureDef);
			auto rt = (grcRenderTargetDX11*)tf->createFromNativeTexture("", tex->texture, NULL);

			gTex = tex;

			ID3D11Texture2D* depthTexture;
			ID3D11DepthStencilView* ds;

			D3D11_TEXTURE2D_DESC tgtDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D24_UNORM_S8_UINT, w, h, 1, 1, D3D11_BIND_DEPTH_STENCIL);

			auto d3d = GetRawD3D11Device();

			if (!d3d)
			{
				d3d = GetD3D11Device();
			}

			d3d->CreateTexture2D(&tgtDesc, nullptr, &depthTexture);

			D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc = CD3D11_DEPTH_STENCIL_VIEW_DESC(depthTexture, D3D11_DSV_DIMENSION_TEXTURE2D);

			d3d->CreateDepthStencilView(depthTexture, &dsDesc, &ds);

			auto bb = rt->views[0];
			//auto ds = rt2->views[0];

			std::thread([bb, ds, d3d]()
			{
				Re3InitStruct data;
				data.backBuffer = bb;
				data.backBufferDS = ds;
				data.context = d3d;
				data.inited = &inited;

				SetThreadName(-1, "zRE3");
				Re3Main(&data);
			})
			.detach();

			return 0;
		})();

		if (inited && nui::HasMainUI())
		{
			int w, h;
			GetGameResolution(w, h);

			SetTextureGtaIm(gTex);

			auto oldRasterizerState = GetRasterizerState();
			SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

			auto oldBlendState = GetBlendState();
			SetBlendState(GetStockStateIdentifier(BlendStateNoBlend));

			auto oldDepthStencilState = GetDepthStencilState();
			SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

			PushDrawBlitImShader();

			rage::grcBegin(4, 4);

			uint32_t color = 0xFFFFFFFF;

			rage::grcVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
			rage::grcVertex(w, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, color, 1.0f, 0.0f);
			rage::grcVertex(0.0f, h, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 1.0f);
			rage::grcVertex(w, h, 0.0f, 0.0f, 0.0f, -1.0f, color, 1.0f, 1.0f);

			rage::grcEnd();

			PopDrawBlitImShader();

			SetRasterizerState(oldRasterizerState);

			SetBlendState(oldBlendState);

			SetDepthStencilState(oldDepthStencilState);
		}
		else if (!nui::HasMainUI())
		{
			FrontEndMenuManager.m_bWantToRestart = true;
		}
	}, INT32_MIN);

	OnPostFrontendRender.Connect([]()
	{
		auto dev = GetD3D11Device();
		ID3D11Device1* dev1;
		if (FAILED(dev->QueryInterface(&dev1)))
		{
			return;
		}

		D3D_FEATURE_LEVEL fls[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		D3D_FEATURE_LEVEL fl;
		static ID3DDeviceContextState* state;

		if (!state)
		{
			assert(SUCCEEDED(dev1->CreateDeviceContextState(0, fls, std::size(fls), D3D11_SDK_VERSION, __uuidof(ID3D11Device1), &fl, &state)));
		}

		auto cxt = GetD3D11DeviceContext();

		ID3D11DeviceContext1* cxt1;
		if (FAILED(cxt->QueryInterface(&cxt1)))
		{
			return;
		}

		ID3DDeviceContextState* lastState;
		cxt1->SwapDeviceContextState(state, &lastState);

		bgfx::renderFrame(0);

		cxt1->SwapDeviceContextState(lastState, NULL);

		lastState->Release();
		cxt1->Release();
		dev1->Release();
	},
	-10);
});

