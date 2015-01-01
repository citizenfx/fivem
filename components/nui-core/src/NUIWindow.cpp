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

#include "memdbgon.h"

NUIWindow::NUIWindow(bool primary, int width, int height)
	: m_primary(primary), m_width(width), m_height(height), m_renderBuffer(nullptr), m_renderBufferDirty(false), m_onClientCreated(nullptr), m_nuiTexture(nullptr)
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

	m_nuiTexture = rage::grcTextureFactory::getInstance()->createManualTexture(m_width, m_height, FORMAT_A8R8G8B8, true, &textureDef);

	// create the client/browser instance
	m_client = new NUIClient(this);

	CefWindowInfo info;
	info.SetAsWindowless(GetDesktopWindow(), true);

	CefBrowserSettings settings;
	settings.javascript_close_windows = STATE_DISABLED;

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

	if (m_renderBufferDirty)
	{
		//int timeBegin = timeGetTime();

		void* pBits = nullptr;
		int pitch;
		bool discarded = false;

#ifndef _HAS_GRCTEXTURE_MAP
		D3DLOCKED_RECT lockedRect;
		m_nuiTexture->m_pITexture->LockRect(0, &lockedRect, NULL, 0);

		pBits = lockedRect.pBits;
		pitch = lockedRect.Pitch;
#else
		rage::grcLockedTexture lockedTexture;

		if (m_nuiTexture->Map(0, 0, &lockedTexture, rage::grcLockFlags::Write))
		{
			pBits = lockedTexture.pBits;
			pitch = lockedTexture.pitch;
		}
		else if (m_nuiTexture->Map(0, 0, &lockedTexture, rage::grcLockFlags::WriteDiscard))
		{
			pBits = lockedTexture.pBits;
			pitch = lockedTexture.pitch;

			discarded = true;
		}
#endif

		if (pBits)
		{
			if (!discarded)
			{
				while (!m_dirtyRects.empty())
				{
					EnterCriticalSection(&m_renderBufferLock);
					CefRect rect = m_dirtyRects.front();
					m_dirtyRects.pop();
					LeaveCriticalSection(&m_renderBufferLock);

					for (int y = rect.y; y < (rect.y + rect.height); y++)
					{
						int* src = &((int*)(m_renderBuffer))[(y * m_roundedWidth) + rect.x];
						int* dest = &((int*)(pBits))[(y * (pitch / 4)) + rect.x];

						memcpy(dest, src, (rect.width * 4));
					}
				}
			}
			else
			{
				EnterCriticalSection(&m_renderBufferLock);
				m_dirtyRects = std::queue<CefRect>();
				LeaveCriticalSection(&m_renderBufferLock);

				memcpy(pBits, m_renderBuffer, m_height * pitch);
			}
		}

#ifndef _HAS_GRCTEXTURE_MAP
		m_nuiTexture->m_pITexture->UnlockRect(0);
#else
		if (pBits)
		{
			m_nuiTexture->Unmap(&lockedTexture);
		}
#endif

		//int duration = timeGetTime() - timeBegin;

		m_renderBufferDirty = false;
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
	CefRect fullRect(0, 0, m_width, m_height);
	((NUIClient*)m_client.get())->GetBrowser()->GetHost()->Invalidate(fullRect, PET_VIEW);
}