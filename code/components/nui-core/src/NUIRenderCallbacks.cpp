#include "StdInc.h"
#include "CefOverlay.h"
#include "CefOleHandler.h"
#include "NUIWindowManager.h"
#include <CL2LaunchMode.h>
#include <HostSharedData.h>
#include <ReverseGameData.h>

extern nui::GameInterface* g_nuiGi;
extern OsrDragHandlerWin g_dragHandler;

#include "memdbgon.h"

extern bool g_shouldHideCursor;
extern POINT g_cursorPos;

extern fwRefContainer<nui::GITexture> g_cursorTexture;

HCURSOR g_defaultCursor;
extern HCURSOR InitDefaultCursor();

extern void TranslateWindowRect(const fwRefContainer<NUIWindow>& window, CRect* rect);

static HookFunction initFunction([] ()
{
	g_nuiGi->OnRender.Connect([]()
	{
		static auto initCursor = ([]()
		{
			g_defaultCursor = InitDefaultCursor();
			g_nuiGi->SetHostCursor(g_defaultCursor);

			return true;
		})();

		// if we're in the main UI, make sure it has focus
		if (nui::HasMainUI())
		{
			nui::GiveFocus("mpMenu", true);
		}

		// draw background
		nui::OnDrawBackground(nui::HasMainUI());

		// update windows from a render context
		Instance<NUIWindowManager>::Get()->ForAllWindows([](fwRefContainer<NUIWindow> window)
		{
			window->UpdateFrame();
		});

		// collect all post-render windows
		std::map<std::string, fwRefContainer<NUIWindow>> renderWindows;

		Instance<NUIWindowManager>::Get()->ForAllWindows([&renderWindows](fwRefContainer<NUIWindow> window)
		{
			if (window->GetPaintType() != NUIPaintTypePostRender)
			{
				return;
			}

			renderWindows.insert({ window->GetName(), window });
		});

		std::list<std::string> windowOrder =
		{
			"nui_mpMenu",
			"root",
		};

		// show on top = render last
		std::reverse(windowOrder.begin(), windowOrder.end());

		for (const auto& windowName : windowOrder)
		{
			auto& window = renderWindows[windowName];

			if (!window.GetRef())
			{
				continue;
			}

			// does the window have a full-screen surface? if so, draw it
			if (window->GetTexture().GetRef())
			{
				g_nuiGi->SetTexture(window->GetTexture(), true);

				int resX, resY;
				g_nuiGi->GetGameResolution(&resX, &resY);

				nui::ResultingRectangle rr;
				rr.color = CRGBA(0xff, 0xff, 0xff, 0xff);

				// the texture is usually upside down (GL->DX coord system), so we draw it as such
				rr.rectangle = CRect(0, resY, resX, 0);

				if (window->IsFixedSizeWindow())
				{
					TranslateWindowRect(window, &rr.rectangle);
				}

				g_nuiGi->DrawRectangles(1, &rr);

				g_nuiGi->UnsetTexture();
			}

			// a popup (e.g. select dropdowns) has to be drawn separately
			if (window->GetPopupTexture().GetRef())
			{
				g_nuiGi->SetTexture(window->GetPopupTexture(), true);

				auto rect = window->GetPopupRect();

				nui::ResultingRectangle rr;
				rr.color = CRGBA(0xff, 0xff, 0xff, 0xff);

				// also upside down here
				rr.rectangle = CRect(rect.x, rect.y + rect.height, rect.x + rect.width, rect.y);

				g_nuiGi->DrawRectangles(1, &rr);

				g_nuiGi->UnsetTexture();
			}
		}

		// are we in any situation where we need a cursor?
		bool needsNuiCursor = (nui::HasCursor()) && !g_shouldHideCursor;
		g_nuiGi->SetHostCursorEnabled(needsNuiCursor);

		// we set the host cursor above unconditionally- this is for cases where the host cursor isn't sufficient
		if (needsNuiCursor)
		{
			// do we need to draw a non-host cursor?
			bool shouldDrawCursorTexture = false;

			// if the game doesn't support a host cursor, draw the cursor texture
			if (!g_nuiGi->CanDrawHostCursor())
			{
				shouldDrawCursorTexture = true;
			}
			// if we're SDK guest, also draw the cursor texture
			else if (launch::IsSDKGuest())
			{
				shouldDrawCursorTexture = true;
			}

			// draw the cursor if we should, and if the texture exists
			if (g_cursorTexture.GetRef() && shouldDrawCursorTexture)
			{
				POINT cursorPos = g_cursorPos;

				// SDK wants to get cursor data from RGD
				if (launch::IsSDKGuest())
				{
					static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

					cursorPos.x = rgd->mouseAbsX;
					cursorPos.y = rgd->mouseAbsY;
				}
				else
				{
					// if not SDK, our cursor follows the host cursor
					GetCursorPos(&cursorPos);
					ScreenToClient(g_nuiGi->GetHWND(), &cursorPos);
				}

				g_nuiGi->SetTexture(g_cursorTexture);

				nui::ResultingRectangle rr;
				rr.color = (g_dragHandler.IsDragging()) ? CRGBA(0xaa, 0xaa, 0xaa, 0xff) : CRGBA(0xff, 0xff, 0xff, 0xff);
				rr.rectangle = CRect(cursorPos.x, cursorPos.y, cursorPos.x + 32.0f, cursorPos.y + 32.0f);

				g_nuiGi->DrawRectangles(1, &rr);

				g_nuiGi->UnsetTexture();
			}
		}
	});
});
