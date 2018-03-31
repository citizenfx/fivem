/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIWindow.h"

#include "NUIClient.h"
#include "NUIWindowManager.h"

#include <DrawCommands.h>

#include <Error.h>

#include "memdbgon.h"

NUIWindow::NUIWindow(bool primary, int width, int height)
	: m_primary(primary), m_width(width), m_height(height), m_renderBuffer(nullptr), m_dirtyFlag(0), m_onClientCreated(nullptr), m_nuiTexture(nullptr)
{
	memset(&m_lastDirtyRect, 0, sizeof(m_lastDirtyRect));

	Instance<NUIWindowManager>::Get()->AddWindow(this);
}

NUIWindow::~NUIWindow()
{
	auto nuiClient = ((NUIClient*)m_client.get());
	auto& mutex = nuiClient->GetWindowLock();

	mutex.lock();
	nuiClient->SetWindowValid(false);
	nuiClient->GetBrowser()->GetHost()->CloseBrowser(true);
	mutex.unlock();

	Instance<NUIWindowManager>::Get()->RemoveWindow(this);
}

fwRefContainer<NUIWindow> NUIWindow::Create(bool primary, int width, int height, CefString url)
{
	auto window = new NUIWindow(primary, width, height);
	window->Initialize(url);

	return window;
}

void NUIWindow::Initialize(CefString url)
{
	InitializeCriticalSection(&m_renderBufferLock);

	if (m_renderBuffer)
	{
		delete[] m_renderBuffer;
	}

	// rounding function
	auto roundUp = [] (int x, int y)
	{
		return x + (y - (x % y));
	};

	// create the temporary backing store
	m_roundedHeight = roundUp(m_height, 16);
	m_roundedWidth = roundUp(m_width, 16);

	m_renderBuffer = new char[4 * m_roundedWidth * m_roundedHeight];

	// create the backing texture
	rage::grcManualTextureDef textureDef;
	memset(&textureDef, 0, sizeof(textureDef));
	textureDef.isStaging = 0;
	textureDef.arraySize = 1;

	m_nuiTexture = rage::grcTextureFactory::getInstance()->createManualTexture(m_width, m_height, 2 /* maps to BGRA DXGI format */, nullptr, true, &textureDef);

	// create the client/browser instance
	m_client = new NUIClient(this);

	CefWindowInfo info;
	info.SetAsWindowless(FindWindow(L"grcWindow", nullptr));
	info.shared_texture_enabled = true;
	info.external_begin_frame_enabled = true;

	CefBrowserSettings settings;
	settings.javascript_close_windows = STATE_DISABLED;
	settings.web_security = STATE_DISABLED;
	settings.windowless_frame_rate = 240;
	CefString(&settings.default_encoding).FromString("utf-8");

	CefRefPtr<CefRequestContext> rc = CefRequestContext::GetGlobalContext();
	CefBrowserHost::CreateBrowser(info, m_client, url, settings, rc);
}

void NUIWindow::AddDirtyRect(const CefRect& rect)
{
	EnterCriticalSection(&m_renderBufferLock);
	m_dirtyRects.push(rect);
	LeaveCriticalSection(&m_renderBufferLock);
}

CefBrowser* NUIWindow::GetBrowser()
{
	return ((NUIClient*)m_client.get())->GetBrowser();
}

static HANDLE g_resetEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

void NUIWindow::UpdateSharedResource(void* sharedHandle, uint64_t syncKey, const CefRenderHandler::RectList& rects)
{
	static bool createdClient;

	static HANDLE lastParentHandle;

	HANDLE parentHandle = (void*)sharedHandle;
	m_syncKey = syncKey;
	
	{
		if (lastParentHandle != parentHandle)
		{
			trace("Changing NUI shared resource...\n");

			lastParentHandle = parentHandle;

			ID3D11Device* device = GetD3D11Device();

			ID3D11Resource* resource = nullptr;
			if (SUCCEEDED(device->OpenSharedResource(parentHandle, __uuidof(IDXGIResource), (void**)&resource)))
			{
				ID3D11Texture2D* texture;
				assert(SUCCEEDED(resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture)));

				NUIWindowManager* wm = Instance<NUIWindowManager>::Get();
				ID3D11Texture2D* oldTexture = nullptr;

				if (wm->GetParentTexture())
				{
					oldTexture = wm->GetParentTexture();
				}

				wm->SetParentTexture(texture);

				// only release afterward to prevent the parent texture being invalid
				if (oldTexture)
				{
					oldTexture->Release();
				}

				createdClient = true;
			}
		}
	}

	for (auto& rect : rects)
	{
		int x = rect.x;
		int y = rect.y;
		int width = rect.width;
		int height = rect.height;

		RECT newRect;
		newRect.left = x;
		newRect.right = x + width;
		newRect.top = GetHeight() - y - height;
		newRect.bottom = GetHeight() - y;
		//newRect.top = y;
		//newRect.bottom = y + height;
		
		RECT oldRect = m_lastDirtyRect;

		UnionRect(&m_lastDirtyRect, &newRect, &oldRect);
	}

	MarkRenderBufferDirty();

	WaitForSingleObject(g_resetEvent, INFINITE);
}

void NUIWindow::UpdateFrame()
{
	if (m_client)
	{
		((NUIClient*)m_client.get())->GetBrowser()->GetHost()->SendExternalBeginFrame(0, 0, 0);
	}

	if (!m_nuiTexture)
	{
		return;
	}

	int resX, resY;
	GetGameResolution(resX, resY);

	if (m_width != resX || m_height != resY)
	{
		m_width = resX;
		m_height = resY;

		((NUIClient*)m_client.get())->GetBrowser()->GetHost()->WasResized();

		// make a new texture
		delete m_nuiTexture;

		rage::grcManualTextureDef textureDef;
		memset(&textureDef, 0, sizeof(textureDef));
		textureDef.isStaging = 0;
		textureDef.arraySize = 1;

		// create the new texture
		m_nuiTexture = rage::grcTextureFactory::getInstance()->createManualTexture(m_width, m_height, 2 /* maps to BGRA DXGI format */, nullptr, true, &textureDef);
	}

	for (auto& item : m_pollQueue)
	{
		NUIClient* client = static_cast<NUIClient*>(m_client.get());
		auto browser = client->GetBrowser();

		auto message = CefProcessMessage::Create("doPoll");
		auto argList = message->GetArgumentList();

		argList->SetSize(1);
		argList->SetString(0, item);

		browser->SendProcessMessage(PID_RENDERER, message);
	}

	m_pollQueue.clear();

	static std::once_flag of;

	std::call_once(of, [&] ()
	{
		static NUIWindow* window = this;

		SetEvent(g_resetEvent);
	});

	NUIWindowManager* wm = Instance<NUIWindowManager>::Get();
	ID3D11Texture2D* texture = wm->GetParentTexture();

	if (texture)
	{
		IDXGIKeyedMutex* keyedMutex = nullptr;
		texture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&keyedMutex);

		if (!keyedMutex)
		{
			FatalError(__FUNCTION__ ": ID3D11Texture2D::QueryInterface(IDXGIKeyedMutex) failed - your GPU driver likely does not support Direct3D shared resources correctly. Please update to the latest version of Windows and your GPU driver to resolve this problem.");
		}

		//
		// dirty flag checking and CopySubresourceRegion are disabled here due to some issue
		// with scheduling frame copies from the OnAcceleratedPaint handler.
		//
		// this'll use a bunch of additional GPU memory bandwidth, but this should not be an
		// issue on any modern GPU.
		//
		if (InterlockedExchange(&m_dirtyFlag, 0) > 0)
		{
			HRESULT hr = keyedMutex->AcquireSync(m_syncKey, 0);

			if (hr == S_OK)
			{
				ID3D11Device* device = GetD3D11Device();

				if (device)
				{
					ID3D11DeviceContext* deviceContext = GetD3D11DeviceContext();
					assert(deviceContext);

					D3D11_BOX box = CD3D11_BOX(m_lastDirtyRect.left,
											   m_lastDirtyRect.top,
											   0,
											   m_lastDirtyRect.right,
											   m_lastDirtyRect.bottom,
											   1);

					//deviceContext->CopySubresourceRegion(m_nuiTexture->texture, 0, m_lastDirtyRect.left, m_lastDirtyRect.top, 0, texture, 0, &box);
					deviceContext->CopyResource(m_nuiTexture->texture, texture);

					memset(&m_lastDirtyRect, 0, sizeof(m_lastDirtyRect));
				}

				SetEvent(g_resetEvent);

				keyedMutex->ReleaseSync(m_syncKey);
			}
			else
			{
				MarkRenderBufferDirty();
			}
		}
		
		keyedMutex->Release();
	}
}

void NUIWindow::SignalPoll(std::string& argument)
{
	if (m_pollQueue.find(argument) == m_pollQueue.end())
	{
		m_pollQueue.insert(argument);
	}
}

void NUIWindow::SetPaintType(NUIPaintType type)
{
	m_paintType = type;
}

void NUIWindow::Invalidate()
{
	((NUIClient*)m_client.get())->GetBrowser()->GetHost()->Invalidate(PET_VIEW);
}
