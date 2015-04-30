#include "StdInc.h"
#include "DrawCommands.h"
#include "Hooking.h"

#include <mutex>

fwEvent<> OnGrcCreateDevice;
fwEvent<> OnPostFrontendRender;

static void(*g_origCreateCB)(const char*);

static void InvokeCreateCB(const char* arg)
{
	g_origCreateCB(arg);

	//OnGrcCreateDevice();
}

static void InvokeRender()
{
	static std::once_flag of;

	std::call_once(of, [] ()
	{
		OnGrcCreateDevice();
	});

	OnPostFrontendRender();
}

static void* g_frontendRenderOrig;
static bool* g_frontendFlag;
static void(*g_origFrontend)(void*, void*, void*);

static bool g_frontendCalled;

static void** g_curViewport;

static void __fastcall DrawFrontendWrap(void* renderPhase, void* b, void* c)
{
	/*if (*g_frontendFlag)
	{
		((void(__thiscall*)(void*))g_frontendRenderOrig)(renderPhase);
	}*/

	g_frontendCalled = true;

	InvokeRender();

	g_origFrontend(renderPhase, b, c);
}

static void(*g_loadsDoScene)();

static void LoadsDoSceneWrap()
{
	InvokeRender();

	g_loadsDoScene();
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

	/*ptrFunc = hook::pattern("83 64 24 28 00 41 B0 01 C6 44 24 20 01 41 8A C8").count(1).get(0).get<void>(-30);
	hook::set_call(&g_origEndScene, ptrFunc);
	hook::call(ptrFunc, EndSceneWrap);*/

	ptrFunc = hook::pattern("EB 0E 80 3D ? ? ? ? 00 74 05 E8").count(1).get(0).get<void>(55);
	hook::set_call(&g_loadsDoScene, ptrFunc);
	hook::call(ptrFunc, LoadsDoSceneWrap);

	// get 'current viewport' variable
	char* location = hook::pattern("57 48 83 EC 20 83 3D ? ? ? ? 00 48 8B 3D ? ? ? ?").count(1).get(0).get<char>(15);

	g_curViewport = (void**)(location + *(int32_t*)location + 4);

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