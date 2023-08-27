/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <Error.h>

#include "ConsoleHost.h"
#include "ConsoleHostImpl.h"
#include "CoreConsole.h"

#include <d3d11.h>

#include <mutex>
#include <queue>

#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#if __has_include("CefOverlay.h")
#include "CefOverlay.h"

#define WITH_NUI 1
#endif

#ifndef IS_LAUNCHER
#include <DrawCommands.h>
#include <grcTexture.h>

#include <InputHook.h>
#endif

#include <mmsystem.h>

#include <imgui.h>

#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

#include <ShlObj.h>

#include <HostSharedData.h>
#include <ReverseGameData.h>

static void BuildFont(float scale);

static ImGuiStyle InitStyle()
{
	ImGuiStyle style;

	ImColor hiliteBlue = ImColor(81, 179, 236);
	ImColor hiliteBlueTransparent = ImColor(81, 179, 236, 180);
	ImColor backgroundBlue = ImColor(22, 24, 28, 200);
	ImColor semiTransparentBg = ImColor(50, 50, 50, 0.6 * 255);
	ImColor semiTransparentBgHover = ImColor(80, 80, 80, 0.6 * 255);

	style.Colors[ImGuiCol_WindowBg] = backgroundBlue;
	style.Colors[ImGuiCol_TitleBg] = hiliteBlue;
	style.Colors[ImGuiCol_TitleBgActive] = hiliteBlue;
	style.Colors[ImGuiCol_TitleBgCollapsed] = hiliteBlue;
	style.Colors[ImGuiCol_Border] = hiliteBlue;
	style.Colors[ImGuiCol_FrameBg] = semiTransparentBg;
	style.Colors[ImGuiCol_FrameBgHovered] = semiTransparentBgHover;
	style.Colors[ImGuiCol_TextSelectedBg] = hiliteBlueTransparent;

	return style;
}

static bool g_conHostInitialized = false;
extern bool g_consoleFlag;
extern bool g_cursorFlag;
int g_scrollTop;
int g_bufferHeight;

bool ConsoleHasAnything()
{
	return g_consoleFlag || g_cursorFlag;
}

bool ConsoleHasMouse()
{
	if (ConsoleHasAnything())
	{
#if WITH_NUI
		if (nui::HasCursor())
		{
			return ImGui::GetIO().WantCaptureMouse;
		}
#endif

		return true;
	}

	return false;
}

bool ConsoleHasKeyboard()
{
	// g_cursorFlag is cursor-only (for when we want to use the dear ImGui cursor for other purposes)
	if (g_consoleFlag)
	{
#if WITH_NUI
		if (nui::HasFocus())
		{
			return ImGui::GetIO().WantCaptureKeyboard;
		}
#endif

		return true;
	}

	return false;
}

#ifndef IS_LAUNCHER
static uint32_t g_pointSamplerState;
static rage::grcTexture* g_fontTexture;

static void RenderDrawListInternal(ImDrawList* drawList, ImDrawData* refData)
{
#ifndef _HAVE_GRCORE_NEWSTATES
	SetRenderState(0, grcCullModeNone);
	SetRenderState(2, 0); // alpha blending m8

	GetD3D9Device()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	GetD3D9Device()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	GetD3D9Device()->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
#else
	auto oldRasterizerState = GetRasterizerState();
	SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

	auto oldBlendState = GetBlendState();
	SetBlendState(GetStockStateIdentifier(BlendStateDefault));

	auto oldDepthStencilState = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));
#endif

	size_t idxOff = 0;

	for (auto& cmd : drawList->CmdBuffer)
	{
		if (cmd.UserCallback != nullptr)
		{
			cmd.UserCallback(nullptr, &cmd);
		}
		else
		{
			if (cmd.TextureId)
			{
				SetTextureGtaIm(*(rage::grcTexture**)cmd.TextureId);
			}
			else
			{
				SetTextureGtaIm(rage::grcTextureFactory::GetNoneTexture());
			}

			PushDrawBlitImShader();

			for (int s = 0; s < cmd.ElemCount; s += 2046)
			{
				int c = std::min(cmd.ElemCount - s, uint32_t(2046));
				rage::grcBegin(3, c);

				//trace("imgui draw %d tris\n", cmd.ElemCount);

				for (int i = 0; i < c; i++)
				{
					auto& vertex = drawList->VtxBuffer.Data[drawList->IdxBuffer.Data[i + idxOff]];
					auto color = vertex.col;
					
					if (!rage::grcTexture::IsRenderSystemColorSwapped())
					{
						color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);
					}

					rage::grcVertex(vertex.pos.x - refData->DisplayPos.x, vertex.pos.y - refData->DisplayPos.y, 0.0f, 0.0f, 0.0f, -1.0f, color, vertex.uv.x, vertex.uv.y);
				}

				idxOff += c;

#if defined(GTA_FIVE)
				// set scissor rects here, as they might be overwritten by a matrix push
				D3D11_RECT scissorRect;
				scissorRect.left = cmd.ClipRect.x - refData->DisplayPos.x;
				scissorRect.top = cmd.ClipRect.y - refData->DisplayPos.y;
				scissorRect.right = cmd.ClipRect.z - refData->DisplayPos.x;
				scissorRect.bottom = cmd.ClipRect.w - refData->DisplayPos.y;

				GetD3D11DeviceContext()->RSSetScissorRects(1, &scissorRect);
#else
				SetScissorRect(cmd.ClipRect.x - refData->DisplayPos.x, cmd.ClipRect.y - refData->DisplayPos.y, cmd.ClipRect.z - refData->DisplayPos.x, cmd.ClipRect.w - refData->DisplayPos.y);
#endif

				rage::grcEnd();
			}

			PopDrawBlitImShader();
		}
	}

#ifdef _HAVE_GRCORE_NEWSTATES
	SetRasterizerState(oldRasterizerState);

	SetBlendState(oldBlendState);

	SetDepthStencilState(oldDepthStencilState);
#else
	GetD3D9Device()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	GetD3D9Device()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	GetD3D9Device()->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
#endif

	delete drawList;

#if defined(GTA_FIVE)
	{
		D3D11_RECT scissorRect;
		scissorRect.left = 0.0f;
		scissorRect.top = 0.0f;
		scissorRect.right = 1.0f;
		scissorRect.bottom = 1.0f;

		GetD3D11DeviceContext()->RSSetScissorRects(1, &scissorRect);
	}
#else
	SetScissorRect(0, 0, 0x1FFF, 0x1FFF);
#endif
}

static void RenderDrawLists(ImDrawData* drawData)
{
	if (!drawData->Valid)
	{
		return;
	}

	for (int i = 0; i < drawData->CmdListsCount; i++)
	{
		ImDrawList* drawList = drawData->CmdLists[i];
		ImDrawList* grDrawList = drawList->CloneOutput();

		if (IsOnRenderThread())
		{
			RenderDrawListInternal(grDrawList, drawData);
		}
		else
		{
			uintptr_t argRef = (uintptr_t)grDrawList;
			uintptr_t argRefB = (uintptr_t)drawData;

			EnqueueGenericDrawCommand([](uintptr_t a, uintptr_t b)
			{
				RenderDrawListInternal((ImDrawList*)a, (ImDrawData*)b);
			},
			&argRef, &argRefB);
		}
	}
}

static void CreateFontTexture()
{
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	rage::grcTextureReference reference;
	memset(&reference, 0, sizeof(reference));
	reference.width = width;
	reference.height = height;
	reference.depth = 1;
	reference.stride = width * 4;
#ifdef GTA_NY
	reference.format = 1;
#else
	reference.format = 11;
#endif
	reference.pixelData = (uint8_t*)pixels;

	rage::grcTexture* texture = rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
	g_fontTexture = texture;

	if (!io.Fonts->TexID)
	{
		void** texId = new void*[2];
		io.Fonts->TexID = (ImTextureID)texId;
	}

	((void**)io.Fonts->TexID)[0] = (void*)g_fontTexture;
}
#else
DLL_EXPORT fwEvent<ImDrawData*> OnRenderImDrawData;

static void RenderDrawLists(ImDrawData* drawData)
{
	if (!drawData->Valid)
	{
		return;
	}

	OnRenderImDrawData(drawData);
}
#endif

void DrawConsole();
void DrawDevGui();

static std::recursive_mutex g_conHostMutex;
ImFont* consoleFontSmall;

void DrawMiniConsole();
void DrawWinConsole(bool* pOpen);

static void HandleFxDKInput(ImGuiIO& io)
{
	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
	static uint16_t lastInputChar = 0;
	static uint32_t inputCharChangedAt = timeGetTime();

	auto currentInputChar = rgd->inputChar;
	auto inputCharChanged = currentInputChar != lastInputChar;

	lastInputChar = currentInputChar;

	if (currentInputChar > 0 && currentInputChar < 0x10000)
	{
		if (inputCharChanged)
		{
			inputCharChangedAt = timeGetTime();
			io.AddInputCharacterUTF16(currentInputChar);
		}
		else if (timeGetTime() - inputCharChangedAt > 300)
		{
			io.AddInputCharacterUTF16(currentInputChar);
		}
	}
}

void OnConsoleFrameDraw(int width, int height, bool usedSharedD3D11);

DLL_EXPORT void OnConsoleFrameDraw(int width, int height)
{
	return OnConsoleFrameDraw(width, height, false);
}

extern ID3D11DeviceContext* g_pd3dDeviceContext;

extern float ImGui_ImplWin32_GetWindowDpiScale(ImGuiViewport* viewport);
extern void ImGui_ImplDX11_RecreateFontsTexture();

static bool g_winConsole;

void OnConsoleFrameDraw(int width, int height, bool usedSharedD3D11)
{
	std::unique_lock lock(g_conHostMutex, std::defer_lock);
	if (!lock.try_lock())
	{
		return;
	}

#ifndef IS_FXSERVER
	static ConVar<bool> winConsoleVar("con_winconsole", ConVar_Archive | ConVar_UserPref, false, &g_winConsole);
#endif

#ifndef IS_LAUNCHER
	static float lastScale = 1.0f;
	float scale = ImGui_ImplWin32_GetWindowDpiScale(ImGui::GetMainViewport());

	if (scale > 2.0f)
	{
		scale = 2.0f;
	}

	if (scale != lastScale)
	{
		ImGui::GetStyle() = InitStyle();
		ImGui::GetStyle().ScaleAllSizes(scale);

		BuildFont(scale);
		CreateFontTexture();

		ImGui_ImplDX11_RecreateFontsTexture();

		lastScale = scale;
	}

	if (!g_fontTexture)
	{
		CreateFontTexture();
	}
#endif

	static uint32_t lastDrawTime = timeGetTime();

	bool shouldDrawGui = false;

	ConHost::OnShouldDrawGui(&shouldDrawGui);

	if (!g_cursorFlag && !g_consoleFlag && !shouldDrawGui && !g_winConsole)
	{
		// if not drawing the gui, we're also not owning the cursor
#ifdef WITH_NUI
		nui::SetHideCursor(false);
#endif

		lastDrawTime = timeGetTime();

		return;
	}

	ImGuiIO& io = ImGui::GetIO();

	HandleFxDKInput(io);

	io.MouseDrawCursor = ConsoleHasMouse();

#ifdef WITH_NUI
	nui::SetHideCursor(io.MouseDrawCursor);
#endif

	{
		io.DisplaySize = ImVec2(width, height);
		io.DeltaTime = (timeGetTime() - lastDrawTime) / 1000.0f;

		if (io.DeltaTime <= 0.0f)
		{
			io.DeltaTime = 1.0f / 60.0f;
		}
	}

	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	if (ImGui::GetPlatformIO().Viewports.Size > 1)
	{
		MSG msg = { 0 };

		// run a separate filtered message loop *per window* as we don't want to include the game window here
		for (size_t viewport = 1; viewport < ImGui::GetPlatformIO().Viewports.Size; viewport++)
		{
			auto hWnd = (HWND)ImGui::GetPlatformIO().Viewports[viewport]->PlatformHandleRaw;

			while (PeekMessageW(&msg, hWnd, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	bool wasSmallFont = false;

	{
		int x, y;
		GetGameResolution(x, y);

		if (x <= 1920 && y <= 1080)
		{
			ImGui::PushFont(consoleFontSmall);
			wasSmallFont = true;
		}
	}

	if (g_consoleFlag)
	{
		DrawDevGui();
		DrawConsole();
	}

	DrawMiniConsole();

	if (g_winConsole)
	{
		DrawWinConsole(&g_winConsole);
	}

	ConHost::OnDrawGui();

	if (wasSmallFont)
	{
		ImGui::PopFont();
	}

	ImGui::Render();
	RenderDrawLists(ImGui::GetDrawData());

	ID3D11RenderTargetView* oldRTV = nullptr;
	ID3D11DepthStencilView* oldDSV = nullptr;

	if (usedSharedD3D11)
	{
		g_pd3dDeviceContext->OMGetRenderTargets(1, &oldRTV, &oldDSV);
	}

    ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();

	if (usedSharedD3D11)
	{
		g_pd3dDeviceContext->OMSetRenderTargets(1, &oldRTV, oldDSV);

		if (oldRTV)
		{
			oldRTV->Release();
			oldRTV = nullptr;
		}

		if (oldDSV)
		{
			oldDSV->Release();
			oldDSV = nullptr;
		}
	}

	lastDrawTime = timeGetTime();
}

static void OnConsoleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& result)
{
	bool shouldDrawGui = false;

	ConHost::OnShouldDrawGui(&shouldDrawGui);

	if (!pass)
	{
		return;
	}

	if (!ConsoleHasAnything())
	{
		return;
	}

	std::unique_lock _(g_conHostMutex);
	ImGuiIO& io = ImGui::GetIO();

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) == TRUE)
	{
		pass = false;
		result = true;
		return;
	}
	
	if (msg == WM_INPUT || (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST))
	{
		if (ConsoleHasMouse())
		{
			pass = false;
		}
	}

	if ((msg >= WM_KEYFIRST && msg <= WM_KEYLAST) || msg == WM_CHAR)
	{
		if (ConsoleHasKeyboard())
		{
			pass = false;
		}
	}

	if (!pass)
	{
		result = true;
	}
}

void ProcessWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult);

DLL_EXPORT void RunConsoleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& result)
{
	ProcessWndProc(hWnd, msg, wParam, lParam, pass, result);
	OnConsoleWndProc(hWnd, msg, wParam, lParam, pass, result);
}

ImFont* consoleFontTiny;

static void BuildFont(float scale)
{
	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->SetTexID(nullptr);

	FILE* font = _wfopen(MakeRelativeCitPath(L"citizen/consolefont.ttf").c_str(), L"rb");

	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder builder;

	static const ImWchar extra_ranges[] = {
		0x0100,
		0x017F, // Latin Extended-A
		0x0180,
		0x024F, // Latin Extended-B
		0x0370,
		0x03FF, // Greek and Coptic
		0x10A0,
		0x10FF, // Georgian
		0x1E00,
		0x1EFF, // Latin Extended Additional
		0,
	};

	builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
	builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
	builder.AddRanges(&extra_ranges[0]);
	builder.BuildRanges(&ranges);

	if (font)
	{
		fseek(font, 0, SEEK_END);

		auto fontSize = ftell(font);

		std::unique_ptr<uint8_t[]> fontData(new uint8_t[fontSize]);

		fseek(font, 0, SEEK_SET);

		fread(&fontData[0], 1, fontSize, font);
		fclose(font);

		ImFontConfig fontCfg;
		fontCfg.FontDataOwnedByAtlas = false;

		io.Fonts->AddFontFromMemoryTTF(fontData.get(), fontSize, 22.0f * scale, &fontCfg, ranges.Data);
		consoleFontSmall = io.Fonts->AddFontFromMemoryTTF(fontData.get(), fontSize, 18.0f * scale, &fontCfg, ranges.Data);
		consoleFontTiny = io.Fonts->AddFontFromMemoryTTF(fontData.get(), fontSize, 14.0f * scale, &fontCfg, ranges.Data);

		io.Fonts->Build();
	}
}

#pragma comment(lib, "d3d11.lib")

void ImGui_ImplWin32_InitPlatformInterface();

extern ImGuiKey ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam);

static HookFunction initFunction([]()
{
	auto cxt = ImGui::CreateContext();
	ImGui::SetCurrentContext(cxt);

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;
	io.ConfigWindowsResizeFromEdges = true;

	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests (optional, rarely used)
	io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports; // We can create multi-viewports on the Platform side (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)

	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

	ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	main_viewport->PlatformHandle = main_viewport->PlatformHandleRaw = nullptr;

	ID3D11Device* device;
	ID3D11DeviceContext* immcon;

	auto setupDevice = [](ID3D11Device* device, ID3D11DeviceContext* immcon)
	{
		if (device && immcon)
		{
			auto& io = ImGui::GetIO();

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui_ImplWin32_InitPlatformInterface();
			}

			ImGui_ImplDX11_Init(device, immcon);
		}
	};

	bool usedSharedD3D11 = false;

	// 'GTA_FIVE' is a D3D11 game so we can use the native D3D11 device
#ifdef GTA_FIVE
	// #TODO: if this turns out breaking, check for 'bad' graphics mods and only run this code if none is present
	usedSharedD3D11 = true;

	OnGrcCreateDevice.Connect([setupDevice]()
	{
		struct
		{
			void* vtbl;
			ID3D11Device* rawDevice;
		}* deviceStuff = (decltype(deviceStuff))GetD3D11Device();

		setupDevice(deviceStuff->rawDevice, GetD3D11DeviceContext());
	});
#endif

	if (!usedSharedD3D11)
	{
		// use the system function as many proxy DLLs don't like multiple devices being made in the game process
		// and they're 'closed source' and 'undocumented' so we can't reimplement the same functionality natively

		// also, create device here and not after the game's or nui:core hacks will mismatch with proxy DLLs
		wchar_t systemD3D11Name[512];
		GetSystemDirectoryW(systemD3D11Name, std::size(systemD3D11Name));
		wcscat(systemD3D11Name, L"\\d3d11.dll");

		auto systemD3D11 = LoadLibraryW(systemD3D11Name);
		assert(systemD3D11);

		auto systemD3D11CreateDevice = (decltype(&D3D11CreateDevice))GetProcAddress(systemD3D11, "D3D11CreateDevice");

		systemD3D11CreateDevice(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&device,
		nullptr,
		&immcon);

		OnGrcCreateDevice.Connect([device, immcon, setupDevice]()
		{
			if (device)
			{
				setupDevice(device, immcon);
			}
		});
	}

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

	static std::string imguiIni = ([]
	{
		std::wstring outPath = MakeRelativeCitPath(L"citizen/imgui.ini");

		PWSTR appDataPath;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
		{
			// create the directory if not existent
			std::wstring cfxPath = std::wstring(appDataPath) + L"\\CitizenFX";
			CreateDirectory(cfxPath.c_str(), nullptr);

			std::wstring profilePath = cfxPath + L"\\imgui.ini";

			if (GetFileAttributes(profilePath.c_str()) == INVALID_FILE_ATTRIBUTES && GetFileAttributes(outPath.c_str()) != INVALID_FILE_ATTRIBUTES)
			{
				CopyFile(outPath.c_str(), profilePath.c_str(), FALSE);
			}

			CoTaskMemFree(appDataPath);

			outPath = profilePath;
		}

		return ToNarrow(outPath);
	})();

	io.IniFilename = const_cast<char*>(imguiIni.c_str());
	//io.ImeWindowHandle = g_hWnd;

	ImGui::GetStyle() = InitStyle();

	// fuck rounding
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	BuildFont(1.0f);

#ifndef IS_LAUNCHER
	OnGrcCreateDevice.Connect([=]()
	{
#ifdef _HAVE_GRCORE_NEWSTATES
#ifndef IS_RDR3
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.MaxAnisotropy = 0;

		g_pointSamplerState = CreateSamplerState(&samplerDesc);
#else
		g_pointSamplerState = GetStockStateIdentifier(SamplerStatePoint);
#endif
#endif
	});

	InputHook::QueryInputTarget.Connect([](std::vector<InputTarget*>& targets)
	{
		if (!ConsoleHasAnything())
		{
			return true;
		}

		static struct : InputTarget
		{
			virtual inline void KeyDown(UINT vKey, UINT scanCode) override
			{
				std::unique_lock _(g_conHostMutex);
				
				ImGuiIO& io = ImGui::GetIO();

				if (vKey < 256)
				{
					io.AddKeyEvent(ImGui_ImplWin32_VirtualKeyToImGuiKey(vKey), true);
				}
			}

			virtual inline void KeyUp(UINT vKey, UINT scanCode) override
			{
				std::unique_lock _(g_conHostMutex);

				ImGuiIO& io = ImGui::GetIO();

				if (vKey < 256)
				{
					io.AddKeyEvent(ImGui_ImplWin32_VirtualKeyToImGuiKey(vKey), false);
				}
			}

			virtual inline void MouseDown(int buttonIdx, int x, int y) override
			{
				std::unique_lock _(g_conHostMutex);

				ImGuiIO& io = ImGui::GetIO();

				if (buttonIdx < std::size(io.MouseDown))
				{
					io.AddMouseButtonEvent(buttonIdx, true);
				}
			}

			virtual inline void MouseUp(int buttonIdx, int x, int y) override
			{
				std::unique_lock _(g_conHostMutex);

				ImGuiIO& io = ImGui::GetIO();

				if (buttonIdx < std::size(io.MouseDown))
				{
					io.AddMouseButtonEvent(buttonIdx, false);
				}
			}

			virtual inline void MouseWheel(int delta) override
			{
				if (delta == 0)
				{
					return;
				}

				std::unique_lock _(g_conHostMutex);

				ImGuiIO& io = ImGui::GetIO();
				io.AddMouseWheelEvent(0.0f, delta > 0 ? +1.0f : -1.0f);
			}

			virtual inline void MouseMove(int x, int y) override
			{
				std::unique_lock _(g_conHostMutex);

				ImGuiIO& io = ImGui::GetIO();
				io.AddMousePosEvent((signed short)(x), (signed short)(y));
			}
		} tgt;

		targets.push_back(&tgt);

		return false;
	});

	InputHook::DeprecatedOnWndProc.Connect([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult)
	{
		OnConsoleWndProc(hWnd, msg, wParam, lParam, pass, lresult);
	});

	InputHook::QueryMayLockCursor.Connect([](int& may)
	{
		if (ConsoleHasAnything())
		{
			may = 0;
		}
	});

	OnPostFrontendRender.Connect([usedSharedD3D11]()
	{
		int width, height;
		GetGameResolution(width, height);

		OnConsoleFrameDraw(width, height, usedSharedD3D11);
	});
#endif
});

struct ConsoleKeyEvent
{
	uint32_t vKey;
	wchar_t character;
	ConsoleModifiers modifiers;
};

void SendPrintMessage(const std::string& channel, const std::string& message);

bool ConHost::IsConsoleOpen()
{
	return g_consoleFlag;
}

void ConHost::Print(const std::string& channel, const std::string& message)
{
	SendPrintMessage(channel, message);
}

fwEvent<const char*, const char*> ConHost::OnInvokeNative;
fwEvent<> ConHost::OnDrawGui;
fwEvent<bool*> ConHost::OnShouldDrawGui;

DLL_EXPORT ImFont* GetConsoleFontSmall()
{
	return consoleFontSmall;
}

DLL_EXPORT ImFont* GetConsoleFontTiny()
{
	return consoleFontTiny;
}

// GTA-specific
#include <Hooking.h>

static decltype(&ReleaseCapture) g_origReleaseCapture;

static void WINAPI ReleaseCaptureStub()
{
	if (ConsoleHasAnything())
	{
		return;
	}

	g_origReleaseCapture();
}

static HookFunction hookFunction([]()
{
	g_origReleaseCapture = (decltype(g_origReleaseCapture))hook::iat("user32.dll", ReleaseCaptureStub, "ReleaseCapture");
});
