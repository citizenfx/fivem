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

#include <DrawCommands.h>
#include <grcTexture.h>

#include <InputHook.h>

#include <mmsystem.h>

#include <imgui.h>

static bool g_conHostInitialized = false;
extern bool g_consoleFlag;
int g_scrollTop;
int g_bufferHeight;

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
			// TODO: cliprect
			SetTextureGtaIm((rage::grcTexture*)cmd.TextureId);

			PushDrawBlitImShader();

			BeginImVertices(3, cmd.ElemCount);

			for (int i = 0; i < cmd.ElemCount; i++)
			{
				auto& vertex = drawList->VtxBuffer.Data[drawList->IdxBuffer.Data[i + idxOff]];
				auto color = vertex.col;

				AddImVertex(vertex.pos.x, vertex.pos.y, 0.0f, 0.0f, 0.0f, -1.0f, color, vertex.uv.x, vertex.uv.y);
			}

			idxOff += cmd.ElemCount;

			DrawImVertices();

			PopDrawBlitImShader();
		}
	}

	SetRasterizerState(oldRasterizerState);

	SetBlendState(oldBlendState);

	SetDepthStencilState(oldDepthStencilState);

	delete drawList;
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

void DrawConsole();

static InitFunction initFunction([] ()
{
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

			io.Fonts->AddFontFromMemoryTTF(fontData, fontSize, 20.0f);
		}
	}

	static std::mutex g_conHostMutex;

	OnGrcCreateDevice.Connect([=]()
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.MaxAnisotropy = 0;

		g_pointSamplerState = CreateSamplerState(&samplerDesc);
	});

	InputHook::OnWndProc.Connect([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& result)
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
		}
		
		if (!pass)
		{
			result = true;
		}
	});

	OnPostFrontendRender.Connect([]()
	{
		if (!g_conHostMutex.try_lock())
		{
			return;
		}

		if (!g_fontTexture)
		{
			CreateFontTexture();
		}

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
			int width, height;
			GetGameResolution(width, height);

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

		ConHost::OnDrawGui();

		ImGui::Render();

		lastDrawTime = timeGetTime();

		g_conHostMutex.unlock();
	});
});

struct ConsoleKeyEvent
{
	uint32_t vKey;
	wchar_t character;
	ConsoleModifiers modifiers;
};

void SendPrintMessage(const std::string& message);

void ConHost::Print(int channel, const std::string& message)
{
	SendPrintMessage(message);
}

fwEvent<const char*, const char*> ConHost::OnInvokeNative;
fwEvent<> ConHost::OnDrawGui;
fwEvent<bool*> ConHost::OnShouldDrawGui;