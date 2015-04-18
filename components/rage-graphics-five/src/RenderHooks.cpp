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
static void(*g_origFrontend)(void*, void*, void*);

static void __fastcall DrawFrontendWrap(void* renderPhase, void* b, void* c)
{
	/*if (*g_frontendFlag)
	{
		((void(__thiscall*)(void*))g_frontendRenderOrig)(renderPhase);
	}*/

	OnPostFrontendRender();

	g_origFrontend(renderPhase, b, c);
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
	void* ptrFunc = hook::pattern("E8 ? ? ? ? 84 C0 75 ? B2 01 B9 2F A9 C2 F4").count(1).get(0).get<void>(33);

	hook::set_call(&g_origCreateCB, ptrFunc);
	hook::call(ptrFunc, InvokeCreateCB);

	// frontend render
	char* vtablePtrLoc = hook::pattern("41 B9 10 00 00 00 C6 44 24 28 00 48 8B D9").count(1).get(0).get<char>(28);
	void* vtablePtr = (void*)(*(int32_t*)vtablePtrLoc + vtablePtrLoc + 4);

	//g_frontendRenderOrig = ((void**)vtablePtr)[24 / 4];
	//((void**)vtablePtr)[24 / 4] = DrawFrontendWrap;

	hook::set_call(&g_origFrontend, ((uintptr_t*)vtablePtr)[2] + 0xAB);
	hook::call(((uintptr_t*)vtablePtr)[2] + 0xAB, DrawFrontendWrap);

	// ignore frozen render device (for PIX and such)
	//hook::put<uint32_t>(hook::pattern("8B 8E C0 0F 00 00 68 88 13 00 00 51 E8").count(2).get(0).get<void>(7), INFINITE);

	/*ptrFunc = hook::pattern("8B 8E F0 09 00 00 E8 ? ? ? ? 68").count(1).get(0).get<void>(12);

	hook::put(ptrFunc, InvokePostFrontendRender);*/

	// frontend render phase
	//hook::put(0xE9F1AC, InvokeFrontendCBStub);

	// in-menu check for renderphasefrontend
	//*(BYTE*)0x43AF21 = 0xEB;

	// temp: d3d debug layer
	//static void* gFunc = D3D11CreateDeviceAndSwapChain2;
	//hook::put(0xF107CE, &gFunc);
});