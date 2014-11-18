#include "StdInc.h"
#include "FontRenderer.h"
#include <gtest/gtest.h>

TEST(FontRendererTests, DummyTest)
{
	TheFonts->Initialize();

	CRect rect(5, 5, 705, 5);
	CRGBA color(255, 255, 255);

	//TheFonts->DrawText(L"Hey!", rect, color, 24.0f, "Segoe UI");

	//TheFonts->DrawText(L"Hey!", rect, color, 24.0f, "Segoe UI");

	TheFonts->DrawText(L"\xD83C\xDF4E... Hi! O\x448\x438\x431\x43A\x430", rect, color, 24.0f, 1.0f, "Segoe UI");
	TheFonts->DrawText(L"\xD83C\xDF4E... Hi!", rect, color, 24.0f, 1.0f, "Segoe UI");

	__asm int 3
}