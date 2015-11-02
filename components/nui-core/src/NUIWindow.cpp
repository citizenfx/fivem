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

#include "memdbgon.h"

namespace egl
{
	unsigned int DLL_IMPORT SetSwapFrameHandler(void(*handler)(void*));

	unsigned int DLL_IMPORT GetMainWindowSharedHandle(HANDLE* shared_handle);
}

NUIWindow::NUIWindow(bool primary, int width, int height)
	: m_primary(primary), m_width(width), m_height(height), m_renderBuffer(nullptr), m_dirtyFlag(0), m_onClientCreated(nullptr), m_nuiTexture(nullptr)
{
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
	info.SetAsWindowless(NULL, true);

	CefBrowserSettings settings;
	settings.javascript_close_windows = STATE_DISABLED;
	settings.windowless_frame_rate = 60;

	CefRefPtr<CefRequestContext> rc = CefRequestContext::GetGlobalContext();
	CefBrowserHost::CreateBrowser(info, m_client, url, settings, rc);

	static NUIWindow* window = this;
	
	egl::SetSwapFrameHandler([] (void*)
	{
		window->UpdateSharedResource();
	});

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

void NUIWindow::UpdateSharedResource()
{
	static bool createdClient;

	static HANDLE lastParentHandle;

	HANDLE parentHandle;
	if (egl::GetMainWindowSharedHandle(&parentHandle))
	{
		if (lastParentHandle != parentHandle)
		{
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

	MarkRenderBufferDirty();
}

void NUIWindow::UpdateFrame()
{
	if (!m_nuiTexture)
	{
		return;
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

	NUIWindowManager* wm = Instance<NUIWindowManager>::Get();
	ID3D11Texture2D* texture = wm->GetParentTexture();

	if (texture)
	{
		IDXGIKeyedMutex* keyedMutex;
		texture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&keyedMutex);

		if (InterlockedExchange(&m_dirtyFlag, 0) > 0)
		{
			if (keyedMutex->AcquireSync(1, 5) == S_OK)
			{
				ID3D11DeviceContext* deviceContext = GetD3D11DeviceContext();
				assert(deviceContext);

				deviceContext->CopyResource(m_nuiTexture->texture, texture);
			}
			else
			{
				MarkRenderBufferDirty();
			}
		}

		keyedMutex->ReleaseSync(0);
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