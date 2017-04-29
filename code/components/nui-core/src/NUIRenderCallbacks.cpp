#include "StdInc.h"
#include "CefOverlay.h"
#include "NUIWindowManager.h"

#include <DrawCommands.h>

#include "memdbgon.h"

extern POINT g_cursorPos;

extern rage::grcTexture* g_cursorTexture;

static InitFunction initFunction([] ()
{
	OnD3DPostReset.Connect([] ()
	{
		Instance<NUIWindowManager>::Get()->ForAllWindows([=] (fwRefContainer<NUIWindow> window)
		{
			window->Invalidate();
		});
	});

#if defined(GTA_NY)
	DoWeIgnoreTheFrontend.Connect([] (bool& dw)
	{
		if (nui::HasMainUI())
		{
			dw = true;
		}
	});
#endif

	OnPostFrontendRender.Connect([] ()
	{
		if (nui::HasMainUI())
		{
			nui::GiveFocus(true);
		}

#if defined(GTA_NY)
		void(*rendCB)() = [] ()
		{
#else
		uintptr_t a1;
		uintptr_t a2;

		EnqueueGenericDrawCommand([] (uintptr_t, uintptr_t)
		{
#endif
			if (nui::HasMainUI())
			{
				ClearRenderTarget(true, -1, true, 1.0f, true, -1);
			}

			nui::OnDrawBackground(nui::HasMainUI());

			Instance<NUIWindowManager>::Get()->ForAllWindows([] (fwRefContainer<NUIWindow> window)
			{
				window->UpdateFrame();
			});

			Instance<NUIWindowManager>::Get()->ForAllWindows([=] (fwRefContainer<NUIWindow> window)
			{
				if (window->GetPaintType() != NUIPaintTypePostRender)
				{
					return;
				}

#ifndef _HAVE_GRCORE_NEWSTATES
				// set the render state to account for premultiplied alpha
				SetRenderState(2, 13);
#else
				auto oldBlendState = GetBlendState();
				SetBlendState(GetStockStateIdentifier(BlendStatePremultiplied));
#endif

				if (window->GetTexture())
				{
					SetTextureGtaIm(window->GetTexture());

					int resX, resY;
					GetGameResolution(resX, resY);

					// we need to subtract 0.5f from each vertex coordinate (half a pixel after scaling) due to the usual half-pixel/texel issue
					uint32_t color = 0xFFFFFFFF;

#ifndef GTA_FIVE
					DrawImSprite(-0.5f, -0.5f, resX - 0.5f, resY - 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
#else
					DrawImSprite(0, 0, resX, resY, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
#endif
				}

#ifdef _HAVE_GRCORE_NEWSTATES
				SetBlendState(oldBlendState);
#endif
			});

			if (nui::HasMainUI())
			{
				POINT cursorPos = g_cursorPos;

#if defined(GTA_NY)
				if (true)//!GameInit::GetGameLoaded())
				{
					SetTextureGtaIm(*(rage::grcTexture**)(0x10C8310));
				}
				else
				{
					SetTextureGtaIm(*(rage::grcTexture**)(0x18AAC20));
				}

				uint32_t color = 0xFFFFFFFF;
				DrawImSprite((float)cursorPos.x, (float)cursorPos.y, (float)cursorPos.x + 40.0f, (float)cursorPos.y + 40.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
#else
#if defined(_HAVE_GRCORE_NEWSTATES)
				auto oldBlendState = GetBlendState();
				SetBlendState(GetStockStateIdentifier(BlendStatePremultiplied));
#endif

				if (g_cursorTexture)
				{
					SetTextureGtaIm(g_cursorTexture);

					uint32_t color = 0xFFFFFFFF;
					DrawImSprite(cursorPos.x, cursorPos.y, cursorPos.x + 40.0f, cursorPos.y + 40.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
				}

#if defined(_HAVE_GRCORE_NEWSTATES)
				SetBlendState(oldBlendState);
#endif
#endif
			}
#if defined(GTA_NY)
		};

		if (!IsOnRenderThread())
		{
			auto dc = new(0) CGenericDC(rendCB);

			dc->Enqueue();
		}
		else
		{
			rendCB();
		}
#else
		}, &a1, &a2);
#endif
	});
});