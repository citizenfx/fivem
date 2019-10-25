#include <StdInc.h>
#include <GameWindow.h>

#include <CefOverlay.h>

#include <CoreConsole.h>

#include <Game.h>

#include <windows.h>
#include <windowsx.h>

#include <HostSharedData.h>
#include <ReverseGameData.h>

#include <d3d11.h>
#include <dcomp.h>

#include <wrl.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <BgfxPrepare.h>

#include <CefImeHandler.h>

#include <bx/uint32_t.h>

#include <imgui.h>

DLL_IMPORT void RunConsoleGameFrame();
DLL_IMPORT void OnConsoleFrameDraw(int width, int height);
DLL_IMPORT void RunConsoleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& result);

void EnsureImFontTexture();

namespace WRL = Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dcomp.lib")

#include <mmsystem.h>

nui::GameInterface* InitializeNui(citizen::GameWindow* window);

bgfx::VertexDecl DebugUvVertex::ms_decl;

static std::vector<std::string> ConvertUnderlinesTo(const std::vector<CefCompositionUnderline>& underlines)
{
	std::vector<std::string> rv;

	for (auto& u : underlines)
	{
		rv.push_back(std::string{ reinterpret_cast<const char*>(&u), sizeof(u) });
	}

	return rv;
}

namespace citizen
{
class Win32GameWindow : public GameWindow
{
  private:
	WRL::ComPtr<IDXGIDevice> dxgiDevice;
	WRL::ComPtr<ID3D11Device> d3d11Device;
	WRL::ComPtr<IDCompositionDesktopDevice> dcompDevice;
	WRL::ComPtr<IDCompositionVisual2> visual;
	WRL::ComPtr<IDCompositionTarget> compTarget;

	bgfx::TextureHandle m_textures[4];
	bgfx::TextureHandle m_backTexture;
	bgfx::ProgramHandle m_program;

	bgfx::VertexBufferHandle m_vbuf;
	bgfx::IndexBufferHandle m_ibuf;

	bgfx::UniformHandle s_texColor;
	bgfx::UniformHandle u_params;
	bgfx::UniformHandle u_modelViewProj;

	nui::GameInterface* m_gi;

	Game* m_game;
	DWORD m_windowThread;
	bool m_sizing;

  public:
	Win32GameWindow(const std::string& title, int defaultWidth, int defaultHeight, Game* game)
	    : m_widthVar("r_width", ConVar_Archive, defaultWidth, &m_targetWidth),
	      m_heightVar("r_height", ConVar_Archive, defaultHeight, &m_targetHeight),
	      m_fullscreenVar("r_fullscreen", ConVar_Archive, true, &m_fullscreen),
	      m_active(true),
		  m_msgTime(0),
		  m_gi(nullptr),
		  m_game(game),
		  m_sizing(false)
	{
		m_textures[0] = m_textures[1] = m_textures[2] = m_textures[3] = m_backTexture = bgfx::TextureHandle{ bgfx::kInvalidHandle };

		m_width = m_targetWidth;
		m_height = m_targetHeight;

		// prepare resolution
		HMONITOR hMonitor       = MonitorFromPoint(POINT{0, 0}, 0);
		MONITORINFO monitorInfo = {sizeof(MONITORINFO)};

		GetMonitorInfo(hMonitor, &monitorInfo);

		int x = 0;
		int y = 0;

		if (!m_fullscreen)
		{
			trace("Creating %dx%d game window...\n", m_width, m_height);

			x = ((monitorInfo.rcWork.right - m_width) / 2);
			y = ((monitorInfo.rcWork.bottom - m_height) / 2);
		}
		else
		{
			m_widthVar.GetHelper()->SetRawValue(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left);
			m_heightVar.GetHelper()->SetRawValue(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);

			m_width = m_targetWidth;
			m_height = m_targetHeight;

			trace("Creating fullscreen (%dx%d) game window...\n", m_width, m_height);
		}

		// create the actual game window - well, the class
		{
			// TODO: icon resource
			WNDCLASS windowClass      = {0};
			windowClass.lpfnWndProc   = WindowProcedureWrapper;
			windowClass.hInstance     = GetModuleHandle(nullptr);
			windowClass.hCursor       = LoadCursor(0, IDC_ARROW);
			windowClass.lpszClassName = L"CfxGameWindow";
			windowClass.style         = CS_HREDRAW | CS_VREDRAW;

			RegisterClass(&windowClass);
		}

		DWORD windowStyle = WS_VISIBLE;

		if (m_fullscreen)
		{
			windowStyle |= WS_POPUP;
		}
		else
		{
			windowStyle |= WS_OVERLAPPEDWINDOW; //WS_SYSMENU | WS_CAPTION | WS_SIZEBOX;
		}

		// adjust the rectangle to fit a possible client area
		RECT windowRect;
		windowRect.left   = x;
		windowRect.right  = m_width + x;
		windowRect.top    = y;
		windowRect.bottom = m_height + y;

		m_aspectRatio = float(windowRect.right - windowRect.left) / float(windowRect.bottom - windowRect.top);

		uint32_t prewidth = windowRect.right - windowRect.left;
		uint32_t preheight = windowRect.bottom - windowRect.top;

		AdjustWindowRect(&windowRect, windowStyle, FALSE);

		m_frameWidth = (windowRect.right - windowRect.left) - prewidth;
		m_frameHeight = (windowRect.bottom - windowRect.top) - preheight;

		// create the window
		m_windowThread = GetCurrentThreadId();

		m_windowHandle = CreateWindowEx(0, L"CfxGameWindow", ToWide(title).c_str(),
		                                windowStyle, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		                                nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

		RAWINPUTDEVICE rawInputDevices[2];
		rawInputDevices[0].hwndTarget = NULL;
		rawInputDevices[0].dwFlags = 0;
		rawInputDevices[0].usUsagePage = 1;
		rawInputDevices[0].usUsage = 2;

		rawInputDevices[1].hwndTarget = NULL;
		rawInputDevices[1].dwFlags = 0;
		rawInputDevices[1].usUsagePage = 1;
		rawInputDevices[1].usUsage = 6;

		RegisterRawInputDevices(rawInputDevices, std::size(rawInputDevices), sizeof(rawInputDevices[0]));

		assert(m_windowHandle != nullptr);

		ms_windowMapping.insert({m_windowHandle, this});

		// set the platform info
		bgfx::PlatformData pd;
		pd.nwh = m_windowHandle;

		bgfx::setPlatformData(pd);

		bgfx::Init init;
		init.type = bgfx::RendererType::Direct3D11;
		init.resolution.width = m_width;
		init.resolution.height = m_height;
		bgfx::init(init);

		bgfx::reset(m_width, m_height, /*BGFX_RESET_VSYNC | */BGFX_RESET_FLIP_AFTER_RENDER);

		//bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);

		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xffffff00, 1.0f, 0);

		m_program = bgfx::createProgram(bgfx::createShader(bgfx::makeRef(vs_debugdraw_fill_texture_dx11, sizeof(vs_debugdraw_fill_texture_dx11))),
			bgfx::createShader(bgfx::makeRef(fs_debugdraw_fill_texture_dx11, sizeof(fs_debugdraw_fill_texture_dx11))));

		DebugUvVertex::init();

		u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, 4);
		u_modelViewProj = bgfx::createUniform("u_modelViewProj", bgfx::UniformType::Vec4, 4);
		s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

		m_vbuf = bgfx::createVertexBuffer(bgfx::makeRef(s_quadVertices, sizeof(s_quadVertices)), DebugUvVertex::ms_decl);
		m_ibuf = bgfx::createIndexBuffer(bgfx::makeRef(s_quadIndices, sizeof(s_quadIndices)));

		CreateTextures(init.resolution.width, init.resolution.height);

		m_gi = InitializeNui(this);
	}

	void CreateTextures(int width, int height)
	{
		std::vector<uint32_t> mem(width * height);

		for (size_t i = 0; i < mem.size(); i++)
		{
			mem[i] = 0xFF0000FF;
		}

		for (int i = 0; i < std::size(m_textures); i++)
		{
			if (bgfx::isValid(m_textures[i]))
			{
				bgfx::destroy(m_textures[i]);
			}

			m_textures[i] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8);

			bgfx::updateTexture2D(m_textures[i], 0, 0, 0, 0, width, height, bgfx::copy(mem.data(), mem.size() * 4));
		}

		if (bgfx::isValid(m_backTexture))
		{
			bgfx::destroy(m_backTexture);
		}

		m_backTexture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8);
	}

	virtual ~Win32GameWindow() override
	{
		Close();
	}

	virtual void* GetNativeHandle() override
	{
		return m_windowHandle;
	}

	virtual void Close() override
	{
		DestroyWindow(m_windowHandle);
		UnregisterClass(L"Win32GameWindow", GetModuleHandle(nullptr));
	}

	virtual void ProcessEvents() override
	{
		MSG msg;

		while (GetMessage(&msg, nullptr, 0, 0/*, PM_REMOVE*/) != 0)
		{
			if (m_active)
			{
				if (!nui::HasMainUI())
				{
					POINT centerPoint = { m_width / 2, m_height / 2 };
					ClientToScreen(m_windowHandle, &centerPoint);

					RECT windowRect;
					GetWindowRect(m_windowHandle, &windowRect);

					RECT clientRect;
					GetClientRect(m_windowHandle, &clientRect);

					POINT topLeft = { clientRect.left, clientRect.top };
					ClientToScreen(m_windowHandle, &topLeft);

					POINT bottomRight = { clientRect.right, clientRect.bottom };
					ClientToScreen(m_windowHandle, &bottomRight);
					
					clientRect.left = topLeft.x;
					clientRect.top = topLeft.y;

					clientRect.right = bottomRight.x;
					clientRect.bottom = bottomRight.y;

					POINT cp;
					GetCursorPos(&cp);

					if (PtInRect(&clientRect, cp))
					{
						SetCursorPos(centerPoint.x, centerPoint.y);

						SetCapture(m_windowHandle);
						ClipCursor(&windowRect);
					}
				}

				while (ShowCursor(FALSE) >= 0)
				{
					// do nothing
				}
			}

			m_msgTime = msg.time;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	virtual void Render() override
	{
		static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
		static bool inited;
		static WRL::ComPtr<IDXGIKeyedMutex> mutexes[4];

		if (rgd->editWidth && (rgd->twidth != m_width || rgd->theight != m_height))
		{
			m_targetWidth = rgd->twidth;
			m_targetHeight = rgd->theight;

			HandleSizeEvent(m_targetWidth, m_targetHeight);

			// move one pixel down to work around D3D/DWM flickering when presenting (that's solved when the user moves the window)
			RECT rect;
			GetWindowRect(m_windowHandle, &rect);
			SetWindowPos(m_windowHandle, NULL, rect.left, rect.top, m_targetWidth + m_frameWidth, m_targetHeight + m_frameHeight, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_ASYNCWINDOWPOS);

			rgd->editWidth = false;
		}

		rgd->width = m_width;
		rgd->height = m_height;

		if (!inited && GetCurrentThreadId() != m_windowThread)
		{
			Sleep(5);
		}

		// if sizing, the window thread is responsible for us
		if (m_sizing && GetCurrentThreadId() != m_windowThread)
		{
			return;
		}

		std::unique_lock<std::mutex> lock(m_renderMutex);

		RunConsoleGameFrame();

		if (!rgd->mainWindowHandle)
		{
			rgd->mainWindowHandle = m_windowHandle;
		}

		if ((rgd->inited && !inited) || rgd->createHandles)
		{
			inited = true;

			// recreate in case we resized before
			CreateTextures(rgd->width, rgd->height);

			// overriding is only allowed a frame later
			bgfx::frame();

			for (int i = 0; i < rgd->surfaceLimit; i++)
			{
				WRL::ComPtr<ID3D11Texture2D> sharedTexture;

				HRESULT hr = ((ID3D11Device*)bgfx::getInternalData()->context)->OpenSharedResource(rgd->surfaces[i], __uuidof(ID3D11Texture2D), (void**)sharedTexture.GetAddressOf());

				sharedTexture.As(&mutexes[i]);

				if (SUCCEEDED(hr))
				{
					D3D11_TEXTURE2D_DESC d;
					sharedTexture->GetDesc(&d);
					bgfx::overrideInternal(m_textures[i], (uintptr_t)sharedTexture.Get());
				}
			}

			rgd->createHandles = false;
		}

		bool blitted = false;

		if (inited)
		{
			int texIdx = -1;

			lock.unlock();

			do 
			{
				texIdx = rgd->FetchSurface(500);

				if (!rgd->inited)
				{
					inited = false;
					break;
				}
			} while (texIdx == -1 && !nui::HasMainUI());

			lock.lock();

			if (texIdx >= 0)
			{
				bgfx::blit(0, m_backTexture, 0, 0, m_textures[texIdx]);
			}
		}

		if (!m_sizing)
		{
			bgfx::touch(0);

			bgfx::setState(BGFX_STATE_WRITE_RGB);
			bgfx::setVertexBuffer(0, m_vbuf);
			bgfx::setIndexBuffer(m_ibuf);
			bgfx::setViewRect(0, 0, 0, m_width, m_height);

			float mtx[4][4] = { 0 };
			mtx[0][0] = 1.0f;
			mtx[1][1] = 1.0f;
			mtx[2][2] = 1.0f;
			mtx[3][3] = 1.0f;

			bgfx::setTransform(&mtx);

			bgfx::setTexture(0, s_texColor, m_backTexture);

			bgfx::submit(0, m_program);

			if (!(GetAsyncKeyState(VK_F8) & 0x8000))
			{
				m_gi->OnRender();
			}

			EnsureImFontTexture();
			OnConsoleFrameDraw(m_width, m_height);
		}

		bgfx::frame();

		if (inited)
		{
			rgd->ReleaseSurface();
		}
	}

	virtual void ProcessEventsOnce() override
	{
		if (m_active)
		{
			ProcessMouse();
		}
	}

	virtual void GetMetrics(int& width, int& height) override
	{
		width = m_width;
		height = m_height;
	}

  private:
	void ProcessMouse()
	{
		// no-op
	}

	LRESULT WindowProcedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int msgTimeDiff  = (GetTickCount() - m_msgTime);
		uint64_t msgTime = 0;

		static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

		bool pass = true;
		LRESULT r;

		RunConsoleWndProc(window, msg, wParam, lParam, pass, r);

		if (!pass)
		{
			return r;
		}

		m_gi->OnWndProc(window, msg, wParam, lParam, pass, r);

		if (!pass)
		{
			return r;
		}

		if (window == m_windowHandle)
		{
			static OsrImeHandlerWin* g_imeHandler;

			if (!g_imeHandler)
			{
				g_imeHandler = new OsrImeHandlerWin(m_windowHandle);
			}

			switch (msg)
			{
			case WM_ERASEBKGND:
				return 1;

			case WM_CHAR:
			{
				using namespace std::string_literals;
				wchar_t charParam = static_cast<wchar_t>(wParam);

				m_game->GetImpl()->GetIPC().Call("charInput"s, (uint32_t)charParam);
				break;
			}

			case WM_PAINT:
			{
				break;
			}

			case WM_SIZING:
			{
				if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000))
				{
					RECT& rect = *(RECT*)lParam;
					uint32_t width = rect.right - rect.left - m_frameWidth;
					uint32_t height = rect.bottom - rect.top - m_frameHeight;

					// Recalculate size according to aspect ratio
					switch (wParam)
					{
					case WMSZ_LEFT:
					case WMSZ_RIGHT:
					{
						float aspectRatio = 1.0f / m_aspectRatio;
						width = bx::uint32_max(800, width);
						height = uint32_t(float(width) * aspectRatio);
					}
					break;

					default:
					{
						float aspectRatio = m_aspectRatio;
						height = bx::uint32_max(600, height);
						width = uint32_t(float(height) * aspectRatio);
					}
					break;
					}

					// Recalculate position using different anchor points
					switch (wParam)
					{
					case WMSZ_TOPLEFT:
						rect.left = rect.right - width - m_frameWidth;
						rect.top = rect.bottom - height - m_frameHeight;
						break;

					case WMSZ_TOP:
					case WMSZ_TOPRIGHT:
						rect.right = rect.left + width + m_frameWidth;
						rect.top = rect.bottom - height - m_frameHeight;
						break;

					case WMSZ_LEFT:
					case WMSZ_BOTTOMLEFT:
						rect.left = rect.right - width - m_frameWidth;
						rect.bottom = rect.top + height + m_frameHeight;
						break;

					default:
						rect.right = rect.left + width + m_frameWidth;
						rect.bottom = rect.top + height + m_frameHeight;
						break;
					}
				}
			}
			return 0;

			case WM_ENTERSIZEMOVE:
			{
				if (!m_sizing)
				{
					SetTimer(m_windowHandle, 1, 10, NULL);

					m_sizing = true;
				}
				break;
			}

			case WM_EXITSIZEMOVE:
			{
				if (m_sizing)
				{
					m_sizing = false;

					KillTimer(m_windowHandle, 1);

					HandleSizeEvent(m_targetWidth, m_targetHeight);

					// move one pixel down to work around D3D/DWM flickering when presenting (that's solved when the user moves the window)
					RECT rect;
					GetWindowRect(m_windowHandle, &rect);
					SetWindowPos(m_windowHandle, NULL, rect.left, rect.top + 1, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED | SWP_NOZORDER);
				}

				break;
			}

			case WM_TIMER:
				if (m_sizing)
				{
					Render();
				}

				break;

			case WM_SIZE:
			{
				uint32_t width = GET_X_LPARAM(lParam);
				uint32_t height = GET_Y_LPARAM(lParam);

				m_targetWidth = width;
				m_targetHeight = height;

				if (!m_sizing)
				{
					HandleSizeEvent(width, height);
				}
			}
			break;

			case WM_INPUT:
			{
				UINT size = 0;
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 0, &size, sizeof(RAWINPUTHEADER));

				void* data = malloc(size);
				if (data && GetRawInputData((HRAWINPUT)lParam, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER)) == size)
				{
					RAWINPUT* input = (RAWINPUT*)data;

					WaitForSingleObject(rgd->inputMutex, INFINITE);

					if (input->header.dwType == RIM_TYPEKEYBOARD)
					{
						if (input->data.keyboard.VKey != 0xFF)
						{
							auto mapExtCode = [](UINT uCode)
							{
								int flag = 0;

								if ((uCode >= VK_NONCONVERT && uCode <= VK_HOME) || (uCode == VK_SELECT || uCode == VK_PRINT) ||
									(uCode == VK_NUMLOCK || uCode == VK_RCONTROL || uCode == VK_RMENU) || uCode == VK_PA1)
								{
									flag = 0x100;

									if (uCode == VK_PA1)
									{
										uCode = VK_RETURN;
									}
								}

								// TODO: load right layout
								int scanCode = (uCode == VK_PAUSE) ? 69 : MapVirtualKeyW(uCode, MAPVK_VK_TO_VSC);

								return flag | scanCode;
							};

							auto mapKeyCode = [](USHORT vKey, UINT uCode, bool ext) -> UINT
							{
								if (vKey == VK_PAUSE)
								{
									return VK_PAUSE;
								}

								auto newVk = MapVirtualKeyW(uCode, MAPVK_VSC_TO_VK);

								switch (newVk)
								{
								// numpad distinction
								case VK_HOME:
									if (!ext)
									{
										newVk = VK_NUMPAD7;
									}
									break;
								case VK_PRIOR:
									if (!ext)
									{
										newVk = VK_NUMPAD9;
									}
									break;
								case VK_CLEAR:
									if (!ext)
									{
										newVk = VK_NUMPAD5;
									}
									break;
								case VK_NEXT:
									if (!ext)
									{
										newVk = VK_NUMPAD3;
									}
									break;
								case VK_END:
									if (!ext)
									{
										newVk = VK_NUMPAD1;
									}
									break;
								case VK_LEFT:
									if (!ext)
									{
										newVk = VK_NUMPAD4;
									}
									break;
								case VK_RIGHT:
									if (!ext)
									{
										newVk = VK_NUMPAD6;
									}
									break;
								case VK_UP:
									if (!ext)
									{
										newVk = VK_NUMPAD8;
									}
									break;
								case VK_DOWN:
									if (!ext)
									{
										newVk = VK_NUMPAD2;
									}
									break;
								case VK_INSERT:
									if (!ext)
									{
										newVk = VK_NUMPAD0;
									}
									break;
								case VK_DELETE:
									if (!ext)
									{
										newVk = VK_DECIMAL;
									}
									break;
								case VK_MULTIPLY:
									if (ext)
									{
										newVk = VK_SNAPSHOT;
									}
									break;
								case VK_NUMLOCK:
									if (!ext)
									{
										newVk = VK_PAUSE;
									}
									break;
								case VK_RETURN:
									newVk = (ext) ? VK_PA1 : VK_RETURN;
									break;
								// left/right keys
								case VK_SHIFT:
									newVk = MapVirtualKeyW(uCode, MAPVK_VSC_TO_VK_EX);
									break;
								case VK_CONTROL:
									newVk = (ext) ? VK_RCONTROL : VK_LCONTROL;
									break;
								case VK_MENU:
									newVk = (ext) ? VK_RMENU : VK_LMENU;
									break;
								}

								return newVk;
							};

							auto code = input->data.keyboard.MakeCode;

							if (input->data.keyboard.Flags & RI_KEY_E1)
							{
								code = mapExtCode(input->data.keyboard.VKey);
							}

							rgd->keyboardState[mapKeyCode(input->data.keyboard.VKey, code, (input->data.keyboard.Flags & RI_KEY_E0) ? true : false)] = (input->data.keyboard.Flags & RI_KEY_BREAK) ? 0 : 0x80;
						}
					}
					else if (input->header.dwType == RIM_TYPEMOUSE)
					{
						rgd->mouseX += input->data.mouse.lLastX;
						rgd->mouseY += input->data.mouse.lLastY;

						if (input->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
						{
							rgd->mouseWheel = int16_t(input->data.mouse.usButtonData) / 120;
						}

						if (input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)
						{
							rgd->mouseButtons &= ~1;
						}
						else if (input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN)
						{
							rgd->mouseButtons |= 1;
						}
						else if (input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)
						{
							rgd->mouseButtons &= ~2;
						}
						else if (input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN)
						{
							rgd->mouseButtons |= 2;
						}
						else if (input->data.mouse.usButtonFlags& RI_MOUSE_BUTTON_3_UP)
						{
							rgd->mouseButtons &= ~4;
						}
						else if (input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN)
						{
							rgd->mouseButtons |= 4;
						}
						else if (input->data.mouse.usButtonFlags& RI_MOUSE_BUTTON_4_UP)
						{
							rgd->mouseButtons &= ~8;
						}
						else if (input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
						{
							rgd->mouseButtons |= 8;
						}
						else if (input->data.mouse.usButtonFlags& RI_MOUSE_BUTTON_5_UP)
						{
							rgd->mouseButtons &= ~16;
						}
						else if (input->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
						{
							rgd->mouseButtons |= 16;
						}
					}

					ReleaseMutex(rgd->inputMutex);
				}

				free(data);
				break;
			}
			case WM_IME_STARTCOMPOSITION:
			{
				if (g_imeHandler)
				{
					g_imeHandler->CreateImeWindow();
					g_imeHandler->MoveImeWindow();
					g_imeHandler->ResetComposition();
				}

				return false;
			}
			case WM_IME_SETCONTEXT:
			{
				// We handle the IME Composition Window ourselves (but let the IME Candidates
				// Window be handled by IME through DefWindowProc()), so clear the
				// ISC_SHOWUICOMPOSITIONWINDOW flag:
				lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
				::DefWindowProc(window, msg, wParam, lParam);

				// Create Caret Window if required
				if (g_imeHandler) {
					g_imeHandler->CreateImeWindow();
					g_imeHandler->MoveImeWindow();
				}

				return false;
			}
			case WM_IME_COMPOSITION:
			{
				if (g_imeHandler) {
					CefString cTextStr;
					if (g_imeHandler->GetResult(lParam, cTextStr)) {
						// Send the text to the browser. The |replacement_range| and
						// |relative_cursor_pos| params are not used on Windows, so provide
						// default invalid values.
						m_game->GetImpl()->GetIPC().Call("imeCommitText", ToNarrow(cTextStr.ToWString()), -1, -1, 0);
						//browser->GetHost()->ImeCommitText(cTextStr,
						//	CefRange(UINT32_MAX, UINT32_MAX), 0);
						g_imeHandler->ResetComposition();
						// Continue reading the composition string - Japanese IMEs send both
						// GCS_RESULTSTR and GCS_COMPSTR.
					}

					std::vector<CefCompositionUnderline> underlines;
					int composition_start = 0;

					if (g_imeHandler->GetComposition(lParam, cTextStr, underlines,
						composition_start)) {
						// Send the composition string to the browser. The |replacement_range|
						// param is not used on Windows, so provide a default invalid value.
						/*browser->GetHost()->ImeSetComposition(
							cTextStr, underlines, CefRange(UINT32_MAX, UINT32_MAX),
							CefRange(composition_start,
								static_cast<int>(composition_start + cTextStr.length())));*/

						m_game->GetImpl()->GetIPC().Call("imeSetComposition", ToNarrow(cTextStr.ToWString()), ConvertUnderlinesTo(underlines), -1, -1, composition_start, static_cast<int>(composition_start + cTextStr.length()));

						// Update the Candidate Window position. The cursor is at the end so
						// subtract 1. This is safe because IMM32 does not support non-zero-width
						// in a composition. Also,  negative values are safely ignored in
						// MoveImeWindow
						g_imeHandler->UpdateCaretPosition(composition_start - 1);
					}
					else {
						//browser->GetHost()->ImeCancelComposition();
						m_game->GetImpl()->GetIPC().Call("imeCancelComposition");
						g_imeHandler->ResetComposition();
						g_imeHandler->DestroyImeWindow();
					}
				}

				return false;
			}
			case WM_IME_ENDCOMPOSITION:
			{
				m_game->GetImpl()->GetIPC().Call("imeCancelComposition");
				g_imeHandler->ResetComposition();
				g_imeHandler->DestroyImeWindow();

				return false;
			}
			case WM_IME_KEYDOWN:
			case WM_IME_KEYUP:
				return false;

			case WM_ACTIVATEAPP:
			{
				m_active = !!wParam;

				if (!m_active)
				{
					ClipCursor(nullptr);
					ReleaseCapture();

					while (ShowCursor(TRUE) < 0)
					{
						// do nothing
					}
				}

				break;
			}

			case WM_CLOSE:
				PostQuitMessage(0);
				break;
			}
		}

		return DefWindowProc(window, msg, wParam, lParam);
	}

  private:
	void HandleSizeEvent(int w, int h)
	{
		std::unique_lock<std::mutex> lock(m_renderMutex);

		if (m_width != w || m_height != h)
		{
			m_width = w;
			m_height = h;

			bgfx::reset(w, h, /*BGFX_RESET_VSYNC | */BGFX_RESET_FLIP_AFTER_RENDER);

			UpdateWindow(m_windowHandle);

			m_game->GetImpl()->GetIPC().Call("resizeWindow", w, h);
		}
	}

	static std::map<HWND, Win32GameWindow*> ms_windowMapping;

	static LRESULT CALLBACK WindowProcedureWrapper(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		auto it = ms_windowMapping.find(window);

		if (it != ms_windowMapping.end())
		{
			return it->second->WindowProcedure(window, msg, wParam, lParam);
		}

		return DefWindowProc(window, msg, wParam, lParam);
	}

  private:
	ConVar<int> m_widthVar;
	ConVar<int> m_heightVar;

	ConVar<bool> m_fullscreenVar;

  private:
	HWND m_windowHandle;

	int m_targetWidth;
	int m_targetHeight;

	int m_width;
	int m_height;

	bool m_fullscreen;

	uint32_t m_msgTime;
	bool m_active;

	uint32_t m_frameWidth;
	uint32_t m_frameHeight;
	float m_aspectRatio;

	std::mutex m_renderMutex;
};

std::map<HWND, Win32GameWindow*> Win32GameWindow::ms_windowMapping;

std::unique_ptr<GameWindow> GameWindow::Create(const std::string& title, int defaultWidth, int defaultHeight, Game* game)
{
	return std::make_unique<Win32GameWindow>(title, defaultWidth, defaultHeight, game);
}
}
