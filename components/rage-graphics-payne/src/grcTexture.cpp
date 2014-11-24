#include "StdInc.h"
#include "grcTexture.h"
#include "Hooking.h"

namespace rage
{
static grcTextureFactory** g_textureFactory;
static grcTexture** g_noneTexture;

grcTextureFactory* grcTextureFactory::getInstance()
{
	return *g_textureFactory;
}

grcTexture* grcTextureFactory::GetNoneTexture()
{
	return *g_noneTexture;
}
}

static HookFunction hookFunction([] ()
{
	rage::g_textureFactory = *hook::pattern("EB 02 33 C0 80 7C 24 04 00 74 05 A3").count(1).get(0).get<rage::grcTextureFactory**>(12);
	rage::g_noneTexture = *hook::pattern("8B CE FF D2 A3 ? ? ? ? 83 47 20 FF 75 10").count(1).get(0).get<rage::grcTexture**>(5);
});

__declspec(dllexport) fwEvent<> OnD3DPostReset;

struct ClearRenderTargetDC
{
	char pad[4];
	int value1;
	float value2;
	int value3;
	bool a1;
	bool a2;
	bool a3;
};

hook::cdecl_stub<void(ClearRenderTargetDC*)> ClearRenderTargetDC__process([] ()
{
	return *hook::pattern("6A 00 68 ? ? ? 00 6A 5E E8").count(1).get(0).get<void*>(3);
});

void ClearRenderTarget(bool a1, int value1, bool a2, float value2, bool a3, int value3)
{
	ClearRenderTargetDC dc;
	dc.value1 = value1;
	dc.value2 = value2;
	dc.value3 = value3;
	dc.a1 = a1;
	dc.a2 = a2;
	dc.a3 = a3;

	ClearRenderTargetDC__process(&dc);
}

bool __declspec(dllexport) rage::grcTexture::IsRenderSystemColorSwapped()
{
	// D3D9 will swap 21 (BGRA) to 9, others will swap it to 0/something else
	return rage::grcTextureFactory::getInstance()->TranslateFormatToParamFormat(21) != 9;
}