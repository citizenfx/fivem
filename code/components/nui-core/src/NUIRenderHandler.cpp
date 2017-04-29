/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIRenderHandler.h"
#include "GfxUtil.h"

#include "memdbgon.h"

NUIRenderHandler::NUIRenderHandler(NUIClient* client)
	: m_paintingPopup(false), m_owner(client)
{

}

bool NUIRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
	NUIWindow* window = m_owner->GetWindow();
	rect.Set(0, 0, window->GetWidth(), window->GetHeight());

	return true;
}

void NUIRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
{
	// OnPaint shouldn't be reached anymore
	//assert(!"NUIRenderHandler::OnPaint");

	return;

	if (m_owner->GetWindowValid())
	{
		// lock the window lock
		auto& lock = m_owner->GetWindowLock();
		lock.lock();

		// paint the buffer
		if (type == PET_VIEW)
		{
			PaintView(dirtyRects, buffer, width, height);
		}
		else if (type == PET_POPUP)
		{
			PaintPopup(buffer, width, height);
		}

		// invalidate and paint the popup
		UpdatePopup();

		// mark the render buffer as dirty
		m_owner->GetWindow()->MarkRenderBufferDirty();

		// unlock the lock
		lock.unlock();
	}
}


void NUIRenderHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
	if (!m_owner->GetWindowValid())
	{
		return;
	}

	// if we're hiding the popup, redraw the view that the popup was drawn on
	if (!show)
	{
		m_popupRect.Set(0, 0, 0, 0);
		browser->GetHost()->Invalidate(PET_VIEW);
	}
}

void NUIRenderHandler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
	if (rect.width <= 0 || rect.height <= 0)
	{
		return;
	}

	NUIWindow* window = m_owner->GetWindow();

	m_popupRect = rect;

	if (m_popupRect.x < 0)
	{
		m_popupRect.x = 0;
	}

	if (m_popupRect.y < 0)
	{
		m_popupRect.y = 0;
	}

	if ((m_popupRect.x + m_popupRect.width) > window->GetWidth())
	{
		m_popupRect.x = window->GetWidth() - m_popupRect.width;
	}

	if ((m_popupRect.y + m_popupRect.height) > window->GetHeight())
	{
		m_popupRect.y = window->GetHeight() - m_popupRect.height;
	}

	if (m_popupRect.x < 0)
	{
		m_popupRect.x = 0;
	}

	if (m_popupRect.y < 0)
	{
		m_popupRect.y = 0;
	}
}

void NUIRenderHandler::UpdatePopup()
{
	if (!m_paintingPopup)
	{
		if (!m_popupRect.IsEmpty())
		{
			m_paintingPopup = true;

			m_owner->GetBrowser()->GetHost()->Invalidate(PET_POPUP);

			m_paintingPopup = false;
		}
	}
}

void NUIRenderHandler::PaintView(const RectList& dirtyRects, const void* buffer, int width, int height)
{
	NUIWindow* window = m_owner->GetWindow();
	void* renderBuffer = window->GetRenderBuffer();
	int roundedWidth = window->GetRoundedWidth();

	for (auto& rect : dirtyRects)
	{
		if (!rage::grcTexture::IsRenderSystemColorSwapped())
		{
			for (int y = rect.y; y < (rect.y + rect.height); y++)
			{
				int* src = &((int*)(buffer))[(y * width) + rect.x];
				int* dest = &((int*)(renderBuffer))[(y * roundedWidth) + rect.x];

				memcpy(dest, src, (rect.width * 4));
			}
		}
		else
		{
			ConvertImageDataRGBA_BGRA(rect.x, rect.y, rect.width, rect.height, width * 4, buffer, roundedWidth * 4, renderBuffer);
		}

		window->AddDirtyRect(rect);
	}
}

void NUIRenderHandler::PaintPopup(const void* buffer, int width, int height)
{
	NUIWindow* window = m_owner->GetWindow();

	// clean popup rectangle
	int skip_pixels = 0, x = m_popupRect.x;
	int skip_rows = 0, yy = m_popupRect.y;

	int w = width;
	int h = height;

	if (x < 0)
	{
		skip_pixels = -x;
		x = 0;
	}

	if (yy < 0)
	{
		skip_rows = -yy;
		yy = 0;
	}

	if ((x + w) > window->GetWidth())
	{
		w -= ((x + w) - window->GetWidth());
	}

	if ((yy + h) > window->GetHeight())
	{
		h -= ((yy + h) - window->GetHeight());
	}

	// do rendering
	void* renderBuffer = window->GetRenderBuffer();
	int roundedWidth = window->GetRoundedWidth();

	if (!rage::grcTexture::IsRenderSystemColorSwapped())
	{
		for (int y = yy; y < (yy + h); y++)
		{
			int* src = &((int*)(buffer))[((y - yy) * width)];
			int* dest = &((int*)(renderBuffer))[(y * roundedWidth) + x];

			memcpy(dest, src, (w * 4));
		}
	}
	else
	{
		ConvertImageDataRGBA_BGRA(x, yy, w, h, width * 4, buffer, roundedWidth * 4, renderBuffer);
	}

	// add dirty rect
	window->AddDirtyRect(m_popupRect);
}