/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "FontRendererImpl.h"
#include "DrawCommands.h"
#include "Hooking.h"

static void callOurs()
{
	/*ClearRenderTarget(true, 0, true, 1.0f, true, 0);

	CRect rect(5, 5, 705, 105);
	CRGBA color(255, 255, 255);
	CRGBA color2(255, 0, 0);

	//TheFonts->DrawRectangle(rect, color2);

	wchar_t russian[] = { 0x041f, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442, 0x0020, 0x043c, 0x0438, 0x0440, 0x0000 };
	wchar_t chinese[] = { 0x4e16, 0x754c, 0x60a8, 0x597d, 0x0000 };
	wchar_t greek[] = { 0x0393, 0x03b5, 0x03b9, 0x03b1, 0x0020, 0x03c3, 0x03b1, 0x03c2, 0x0020, 0x03ba, 0x03cc, 0x03c3, 0x03bc, 0x03bf, 0x0000 };
	wchar_t japanese[] = { 0x4eca, 0x65e5, 0x306f, 0x4e16, 0x754c, 0x0000 };
	wchar_t runic[] = { 0x16ba, 0x16d6, 0x16da, 0x16df, 0x0020, 0x16b9, 0x16df, 0x16c9, 0x16da, 0x16de, 0x0000 };

	TheFonts->DrawText(va(L"\xD83C\xDF4E @ \xD83C\xDF55... Hi! O\x448\x438\x431\x43A\x430... %s %s %s %s %s", russian, chinese, greek, japanese, runic), rect, color, 26.0f, 1.0f, "Segoe UI");

	static wchar_t str[128];

	if (!str[0])
	{
		FILE* f = fopen("A:/lolunicode.txt", "rb");

		if (f)
		{
			fread(str, 1, sizeof(str), f);
			fclose(f);
		}
	}

	rect.SetRect(5, 205, 705, 205);

	TheFonts->DrawText(str, rect, color, 26.0f, 1.0f, "Segoe UI");*/

	//((void(*)())0x4112A0)();
}

static HookFunction hf([] ()
{
	TheFonts->Initialize();

	OnPostFrontendRender.Connect([] ()
	{
		callOurs();
	}, -500);
	//hook::put(0x44CD06, callOurs);
});