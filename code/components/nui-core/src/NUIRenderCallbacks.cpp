#include "StdInc.h"
#include "CefOverlay.h"
#include "NUIWindowManager.h"

extern nui::GameInterface* g_nuiGi;

#include "memdbgon.h"

extern bool g_hasCursor;
extern POINT g_cursorPos;

extern bool g_isDragging;

extern nui::GITexture* g_cursorTexture;

static HookFunction initFunction([] ()
{
	g_nuiGi->OnRender.Connect([]()
	{
		if (nui::HasMainUI())
		{
			nui::GiveFocus(true);
		}

		nui::OnDrawBackground(nui::HasMainUI());

		Instance<NUIWindowManager>::Get()->ForAllWindows([](fwRefContainer<NUIWindow> window)
		{
			window->UpdateFrame();
		});

		Instance<NUIWindowManager>::Get()->ForAllWindows([=](fwRefContainer<NUIWindow> window)
		{
			if (window->GetPaintType() != NUIPaintTypePostRender)
			{
				return;
			}

			if (window->GetTexture())
			{
				g_nuiGi->SetTexture(window->GetTexture(), true);

				int resX, resY;
				g_nuiGi->GetGameResolution(&resX, &resY);

				nui::ResultingRectangle rr;
				rr.color = CRGBA(0xff, 0xff, 0xff, 0xff);
				rr.rectangle = CRect(0, resY, resX, 0);

				g_nuiGi->DrawRectangles(1, &rr);

				g_nuiGi->UnsetTexture();
			}

			if (window->GetPopupTexture())
			{
				g_nuiGi->SetTexture(window->GetPopupTexture(), true);


				const CefRect& rect = window->GetPopupRect();

				// we need to subtract 0.5f from each vertex coordinate (half a pixel after scaling) due to the usual half-pixel/texel issue
				nui::ResultingRectangle rr;
				rr.color = CRGBA(0xff, 0xff, 0xff, 0xff);
				rr.rectangle = CRect(rect.x, rect.y + rect.height, rect.x + rect.width, rect.y);

				g_nuiGi->DrawRectangles(1, &rr);

				g_nuiGi->UnsetTexture();
			}
		});

		if (nui::HasMainUI() || g_hasCursor)
		{
			POINT cursorPos = g_cursorPos;

			GetCursorPos(&cursorPos);
			ScreenToClient(g_nuiGi->GetHWND(), &cursorPos);

			if (g_cursorTexture)
			{
				g_nuiGi->SetTexture(g_cursorTexture);

				uint32_t color = 0xFFFFFFFF;

				if (g_isDragging)
				{
					color = 0xFFAAAAAA;
				}

				nui::ResultingRectangle rr;
				rr.color = (g_isDragging) ? CRGBA(0xaa, 0xaa, 0xaa, 0xff) : CRGBA(0xff, 0xff, 0xff, 0xff);
				rr.rectangle = CRect(cursorPos.x, cursorPos.y, cursorPos.x + 32.0f, cursorPos.y + 32.0f);

				g_nuiGi->DrawRectangles(1, &rr);

				g_nuiGi->UnsetTexture();
			}
		}
	});
});
