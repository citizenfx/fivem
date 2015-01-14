#include "StdInc.h"
#include "CefOverlay.h"
#include "NUIWindowManager.h"

#include <DrawCommands.h>

#include "memdbgon.h"

extern POINT g_cursorPos;

static InitFunction initFunction([] ()
{
	OnD3DPostReset.Connect([] ()
	{
		Instance<NUIWindowManager>::Get()->ForAllWindows([=] (fwRefContainer<NUIWindow> window)
		{
			window->Invalidate();
		});
	});

	OnPostFrontendRender.Connect([] ()
	{
		if (nui::HasMainUI())
		{
			nui::GiveFocus(true);
		}

#if defined(GTA_NY)
		auto dc = new(0) CGenericDC([] ()
		{
#else
		uint32_t a1;
		uint32_t a2;

		EnqueueGenericDrawCommand([] (uint32_t, uint32_t)
		{
#endif
			if (nui::HasMainUI())
			{
				ClearRenderTarget(true, -1, true, 1.0f, true, -1);
			}

			nui::OnDrawBackground(nui::HasMainUI());

#if !defined(GTA_NY)
			Instance<NUIWindowManager>::Get()->ForAllWindows([] (fwRefContainer<NUIWindow> window)
			{
				window->UpdateFrame();
			});
#endif

			Instance<NUIWindowManager>::Get()->ForAllWindows([=] (fwRefContainer<NUIWindow> window)
			{
				if (window->GetPaintType() != NUIPaintTypePostRender)
				{
					return;
				}

				// set the render state to account for premultiplied alpha
				SetRenderState(2, 13);

				if (window->GetTexture())
				{
					SetTextureGtaIm(window->GetTexture());

					int resX, resY;
					GetGameResolution(resX, resY);

					// we need to subtract 0.5f from each vertex coordinate (half a pixel after scaling) due to the usual half-pixel/texel issue
					uint32_t color = 0xFFFFFFFF;

					DrawImSprite(-0.5f, -0.5f, resX - 0.5f, resY - 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
				}
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
#endif
			}
#if defined(GTA_NY)
		});

		dc->Enqueue();
#else
		}, &a1, &a2);
#endif
	});

	/*g_hooksDLL->SetHookCallback(StringHash("msgConfirm"), [] (void*)
	{
		nui::SetMainUI(true);

		nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");
	});*/
});