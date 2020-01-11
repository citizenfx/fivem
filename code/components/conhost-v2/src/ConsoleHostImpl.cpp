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

#include <mutex>
#include <queue>

#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#ifndef IS_LAUNCHER
#include <DrawCommands.h>
#include <grcTexture.h>

#include <InputHook.h>
#endif

#include <mmsystem.h>

#include <imgui.h>

static bool g_conHostInitialized = false;
extern bool g_consoleFlag;
int g_scrollTop;
int g_bufferHeight;

#ifndef IS_LAUNCHER
static uint32_t g_pointSamplerState;
static rage::grcTexture* g_fontTexture;

struct DrawList
{
	ImVector<ImDrawVert> VtxBuffer;
	ImVector<ImDrawIdx> IdxBuffer;
	ImVector<ImDrawCmd> CmdBuffer;
};

static void RenderDrawListInternal(DrawList* drawList)
{
	auto oldRasterizerState = GetRasterizerState();
	SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

	auto oldBlendState = GetBlendState();
	SetBlendState(GetStockStateIdentifier(BlendStateDefault));

	auto oldDepthStencilState = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));

	size_t idxOff = 0;

	for (auto& cmd : drawList->CmdBuffer)
	{
		if (cmd.UserCallback != nullptr)
		{
			cmd.UserCallback(nullptr, &cmd);
		}
		else
		{
			SetTextureGtaIm((rage::grcTexture*)cmd.TextureId);

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

					rage::grcVertex(vertex.pos.x, vertex.pos.y, 0.0f, 0.0f, 0.0f, -1.0f, color, vertex.uv.x, vertex.uv.y);
				}

				idxOff += c;

#if defined(GTA_FIVE)
				// set scissor rects here, as they might be overwritten by a matrix push
				D3D11_RECT scissorRect;
				scissorRect.left = cmd.ClipRect.x;
				scissorRect.top = cmd.ClipRect.y;
				scissorRect.right = cmd.ClipRect.z;
				scissorRect.bottom = cmd.ClipRect.w;

				GetD3D11DeviceContext()->RSSetScissorRects(1, &scissorRect);
#else
				SetScissorRect(cmd.ClipRect.x, cmd.ClipRect.y, cmd.ClipRect.z, cmd.ClipRect.w);
#endif

				rage::grcEnd();
			}

			PopDrawBlitImShader();
		}
	}

	SetRasterizerState(oldRasterizerState);

	SetBlendState(oldBlendState);

	SetDepthStencilState(oldDepthStencilState);

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
		
		DrawList* grDrawList = new DrawList();
		grDrawList->CmdBuffer.swap(drawList->CmdBuffer);
		grDrawList->IdxBuffer.swap(drawList->IdxBuffer);
		grDrawList->VtxBuffer.swap(drawList->VtxBuffer);

		if (IsOnRenderThread())
		{
			RenderDrawListInternal(grDrawList);
		}
		else
		{
			uintptr_t argRef = (uintptr_t)grDrawList;

			EnqueueGenericDrawCommand([](uintptr_t a, uintptr_t b)
			{
				RenderDrawListInternal((DrawList*)a);
			}, &argRef, &argRef);
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
	reference.format = 11;
	reference.pixelData = (uint8_t*)pixels;

	rage::grcTexture* texture = rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
	g_fontTexture = texture;

	io.Fonts->TexID = (void *)g_fontTexture;
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

static std::mutex g_conHostMutex;

void DrawMiniConsole();

DLL_EXPORT void OnConsoleFrameDraw(int width, int height)
{
	if (!g_conHostMutex.try_lock())
	{
		return;
	}

#ifndef IS_LAUNCHER
	if (!g_fontTexture)
	{
		CreateFontTexture();
	}
#endif

	static uint32_t lastDrawTime = timeGetTime();

	bool shouldDrawGui = false;

	ConHost::OnShouldDrawGui(&shouldDrawGui);

	if (!g_consoleFlag && !shouldDrawGui)
	{
		lastDrawTime = timeGetTime();

		g_conHostMutex.unlock();
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = g_consoleFlag;

	{
		io.DisplaySize = ImVec2(width, height);

		io.DeltaTime = (timeGetTime() - lastDrawTime) / 1000.0f;
	}

	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	ImGui::NewFrame();

	if (g_consoleFlag)
	{
		DrawConsole();
	}

	DrawMiniConsole();

	ConHost::OnDrawGui();

	ImGui::Render();

	lastDrawTime = timeGetTime();

	g_conHostMutex.unlock();
}

static void OnConsoleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& result)
{
	bool shouldDrawGui = false;

	ConHost::OnShouldDrawGui(&shouldDrawGui);

	if (!g_consoleFlag || !pass)
	{
		return;
	}

	std::unique_lock<std::mutex> g_conHostMutex;

	ImGuiIO& io = ImGui::GetIO();
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		pass = false;
		break;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		pass = false;
		break;
	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		pass = false;
		break;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		pass = false;
		break;
	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		pass = false;
		break;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		pass = false;
		break;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		pass = false;
		break;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		pass = false;
		break;
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		pass = false;
		break;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		pass = false;
		break;
	case WM_CHAR:
		// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		pass = false;
		break;
	case WM_INPUT:
		pass = false;
		break;
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

ImFont* consoleFontSmall;
ImFont* consoleFontTiny;

static InitFunction initFunction([]()
{
	auto cxt = ImGui::CreateContext();
	ImGui::SetCurrentContext(cxt);

	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;
	io.ConfigWindowsResizeFromEdges = true;

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

	static std::string imguiIni = ToNarrow(MakeRelativeCitPath(L"citizen/imgui.ini"));
	io.IniFilename = const_cast<char*>(imguiIni.c_str());
	io.RenderDrawListsFn = RenderDrawLists;  // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
	//io.ImeWindowHandle = g_hWnd;

	ImGuiStyle& style = ImGui::GetStyle();

	ImColor hiliteBlue = ImColor(81, 179, 236);
	ImColor hiliteBlueTransparent = ImColor(81, 179, 236, 180);
	ImColor backgroundBlue = ImColor(22, 24, 28, 200);
	ImColor semiTransparentBg = ImColor(255, 255, 255, 0.6);
	ImColor semiTransparentBgHover = ImColor(255, 255, 255, 0.8);

	style.Colors[ImGuiCol_WindowBg] = backgroundBlue;
	style.Colors[ImGuiCol_TitleBg] = hiliteBlue;
	style.Colors[ImGuiCol_TitleBgActive] = hiliteBlue;
	style.Colors[ImGuiCol_TitleBgCollapsed] = hiliteBlue;
	style.Colors[ImGuiCol_Border] = hiliteBlue;
	style.Colors[ImGuiCol_FrameBg] = semiTransparentBg;
	style.Colors[ImGuiCol_FrameBgHovered] = semiTransparentBgHover;
	style.Colors[ImGuiCol_TextSelectedBg] = hiliteBlueTransparent;

	// fuck rounding
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	{
		FILE* font = _wfopen(MakeRelativeCitPath(L"citizen/mensch.ttf").c_str(), L"rb");

		if (font)
		{
			fseek(font, 0, SEEK_END);

			auto fontSize = ftell(font);

			uint8_t* fontData = new uint8_t[fontSize];

			fseek(font, 0, SEEK_SET);

			fread(&fontData[0], 1, fontSize, font);
			fclose(font);

			io.Fonts->AddFontFromMemoryTTF(fontData, fontSize, 22.0f);

			consoleFontSmall = io.Fonts->AddFontFromMemoryTTF(fontData, fontSize, 18.0f);
			consoleFontTiny = io.Fonts->AddFontFromMemoryTTF(fontData, fontSize, 14.0f);
		}
	}

#ifndef IS_LAUNCHER
	OnGrcCreateDevice.Connect([=]()
	{
#ifndef IS_RDR3
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.MaxAnisotropy = 0;

		g_pointSamplerState = CreateSamplerState(&samplerDesc);
#else
		g_pointSamplerState = GetStockStateIdentifier(SamplerStatePoint);
#endif
	});

	InputHook::QueryInputTarget.Connect([](std::vector<InputTarget*>& targets)
	{
		if (!g_consoleFlag)
		{
			return true;
		}

		static struct : InputTarget
		{
			virtual inline void KeyDown(UINT vKey, UINT scanCode) override
			{
				std::unique_lock<std::mutex> g_conHostMutex;

				ImGuiIO& io = ImGui::GetIO();

				if (vKey < 256)
				{
					io.KeysDown[vKey] = 1;
				}
			}

			virtual inline void KeyUp(UINT vKey, UINT scanCode) override
			{
				std::unique_lock<std::mutex> g_conHostMutex;

				ImGuiIO& io = ImGui::GetIO();

				if (vKey < 256)
				{
					io.KeysDown[vKey] = 0;
				}
			}

			virtual inline void MouseDown(int buttonIdx, int x, int y) override
			{
				std::unique_lock<std::mutex> g_conHostMutex;

				ImGuiIO& io = ImGui::GetIO();

				if (buttonIdx < std::size(io.MouseDown))
				{
					io.MouseDown[buttonIdx] = true;
				}
			}

			virtual inline void MouseUp(int buttonIdx, int x, int y) override
			{
				std::unique_lock<std::mutex> g_conHostMutex;

				ImGuiIO& io = ImGui::GetIO();

				if (buttonIdx < std::size(io.MouseDown))
				{
					io.MouseDown[buttonIdx] = false;
				}
			}

			virtual inline void MouseWheel(int delta) override
			{
				std::unique_lock<std::mutex> g_conHostMutex;

				ImGuiIO& io = ImGui::GetIO();
				io.MouseWheel += delta > 0 ? +1.0f : -1.0f;
			}

			virtual inline void MouseMove(int x, int y) override
			{
				std::unique_lock<std::mutex> g_conHostMutex;

				ImGuiIO& io = ImGui::GetIO();
				io.MousePos.x = (signed short)(x);
				io.MousePos.y = (signed short)(y);
			}
		} tgt;

		targets.push_back(&tgt);

		return false;
	});

	InputHook::DeprecatedOnWndProc.Connect([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult)
	{
		OnConsoleWndProc(hWnd, msg, wParam, lParam, pass, lresult);
	});

	OnPostFrontendRender.Connect([]()
	{
		int width, height;
		GetGameResolution(width, height);

		OnConsoleFrameDraw(width, height);
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
