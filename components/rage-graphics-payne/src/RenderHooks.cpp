#include "StdInc.h"
#include "DrawCommands.h"
#include "Hooking.h"

fwEvent<> OnGrcCreateDevice;
fwEvent<> OnPostFrontendRender;

static void(*g_origCreateCB)(const char*);

static void InvokeCreateCB(const char* arg)
{
	g_origCreateCB(arg);

	OnGrcCreateDevice();
}

static void* g_frontendRenderOrig;
static bool* g_frontendFlag;

static void __fastcall DrawFrontendWrap(void* renderPhase)
{
	/*if (*g_frontendFlag)
	{
		((void(__thiscall*)(void*))g_frontendRenderOrig)(renderPhase);
	}*/

	OnPostFrontendRender();
}

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

HRESULT WINAPI D3D11CreateDeviceAndSwapChain2(
	_In_   IDXGIAdapter *pAdapter,
	_In_   D3D_DRIVER_TYPE DriverType,
	_In_   HMODULE Software,
	_In_   UINT Flags,
	_In_   const D3D_FEATURE_LEVEL *pFeatureLevels,
	_In_   UINT FeatureLevels,
	_In_   UINT SDKVersion,
	_In_   const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
	_Out_  IDXGISwapChain **ppSwapChain,
	_Out_  ID3D11Device **ppDevice,
	_Out_  D3D_FEATURE_LEVEL *pFeatureLevel,
	_Out_  ID3D11DeviceContext **ppImmediateContext
	)
{
	return D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, D3D11_CREATE_DEVICE_DEBUG | Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
}


static HookFunction hookFunction([] ()
{
	// device creation
	void* ptrFunc = hook::pattern("83 C4 08 68 ? ? ? ? E8 ? ? ? ? 83 C4 04 84").count(1).get(0).get<void>(8);

	hook::set_call(&g_origCreateCB, ptrFunc);
	hook::call(ptrFunc, InvokeCreateCB);

	// frontend render
	void* vtablePtr = *hook::pattern("C7 86 A0 09 00 00 07 00 00 00 C7 86 DC 09 00 00 0E 00").count(1).get(0).get<void*>(-4);

	//g_frontendRenderOrig = ((void**)vtablePtr)[24 / 4];
	//((void**)vtablePtr)[24 / 4] = DrawFrontendWrap;

	hook::call(((uintptr_t*)vtablePtr)[24 / 4] + 0xB9, DrawFrontendWrap);

	// twist around the frontend draw flag
	//auto frontendMatch = hook::pattern("80 3D ? ? ? ? 00 68 00 0A 00 00 75 16").count(1).get(0);

	//g_frontendFlag = *frontendMatch.get<bool*>(2);
	//hook::put<uint8_t>(frontendMatch.get<void>(12), 0xEB);

	// don't break on alt-tab
	hook::put<uint16_t>(hook::pattern("E8 ? ? ? ? 84 C0 0F 84 66 07 00 00 8B 0D").count(1).get(0).get<void>(7), 0xE990);

	// ignore frozen render device (for PIX and such)
	hook::put<uint32_t>(hook::pattern("8B 8E C0 0F 00 00 68 88 13 00 00 51 E8").count(2).get(0).get<void>(7), INFINITE);

	/*ptrFunc = hook::pattern("8B 8E F0 09 00 00 E8 ? ? ? ? 68").count(1).get(0).get<void>(12);

	hook::put(ptrFunc, InvokePostFrontendRender);*/

	// frontend render phase
	//hook::put(0xE9F1AC, InvokeFrontendCBStub);

	// in-menu check for renderphasefrontend
	//*(BYTE*)0x43AF21 = 0xEB;

	// temp: d3d debug layer
	static void* gFunc = D3D11CreateDeviceAndSwapChain2;
	hook::put(0xF107CE, &gFunc);
});