#include <StdInc.h>
#include <Hooking.h>

#include <MinHook.h>

HRESULT(*g_origInitializeGraphics)(void*, void*, void*);

LONG_PTR DontSetWindowLongPtrW(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
	return GetWindowLongPtrW(hWnd, nIndex);
}

HRESULT NullInitializeGraphics(void* rgscUI, void* d3dDevice, void* hWndStruct)
{
	trace("NullInitializeGraphics\n");

	auto fn = GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowLongPtrW");
	MH_CreateHook(fn, DontSetWindowLongPtrW, nullptr);
	MH_EnableHook(fn);

	g_origInitializeGraphics(rgscUI, d3dDevice, hWndStruct);

	MH_DisableHook(fn);

	return S_OK;
}

HRESULT NullRender(void* rgscUI)
{
	return S_OK;
}

HRESULT NullCopyBuffer(void* rgscUI, void* a1)
{
	return S_OK;
}

#define LOG_CALL() trace("%s\n", __FUNCTION__)

class IRgscUi
{

};

class IRgsc
{
public:
	virtual HRESULT QueryInterface(GUID* iid, void** out) = 0;
	virtual HRESULT m_01(void* a1, void* a2, void* a3) = 0;
	virtual HRESULT m_02() = 0;
	virtual void* m_03() = 0;
	virtual void* m_04() = 0; // get
	virtual void* m_05() = 0; // get
	virtual void* m_06() = 0; // get
	virtual IRgscUi* GetUI() = 0; // get
	virtual HRESULT Initialize(void*, void*, void*, void* a4, void* a5, void* a6) = 0;
	virtual void* m_07() = 0; // get
	virtual void* m_08() = 0;
	virtual void* m_09() = 0;
	virtual void* m_10() = 0;
	virtual void SetSomething(void*) = 0;
	virtual void* SetSomethingSteamTicket(void*) = 0;
	virtual void* m_11() = 0;
	virtual bool m_12(void*, void*) = 0;
	virtual bool m_13() = 0;
	virtual void* m_14() = 0;
	virtual void* m_15() = 0;
	virtual void* m_16() = 0;
	virtual void* m_17() = 0;
};

class RgscStub : public IRgsc
{
public:
	RgscStub(IRgsc* basePtr)
		: m_baseRgsc(basePtr)
	{

	}


	virtual HRESULT QueryInterface(GUID* iid, void** out) override
	{
		LOG_CALL();

		HRESULT hr = m_baseRgsc->QueryInterface(iid, out);

		if (*out == m_baseRgsc)
		{
			*out = this;
		}

		return hr;
	}

	virtual HRESULT m_01(void* a1, void* a2, void* a3) override
	{
		LOG_CALL();

		return m_baseRgsc->m_01(a1, a2, a3);
	}

	virtual HRESULT m_02() override
	{
		LOG_CALL();

		return m_baseRgsc->m_02();
	}

	virtual void* m_03() override
	{
		//LOG_CALL();

		return m_baseRgsc->m_03();
	}

	virtual void* m_04() override
	{
		LOG_CALL();

		return m_baseRgsc->m_04();
	}

	virtual void* m_05() override
	{
		LOG_CALL();

		return m_baseRgsc->m_05();
	}

	virtual void* m_06() override
	{
		LOG_CALL();

		return m_baseRgsc->m_06();
	}

	virtual IRgscUi* GetUI() override
	{
		LOG_CALL();

		IRgscUi* ui = m_baseRgsc->GetUI();

		uintptr_t* vtable = *(uintptr_t**)ui;
		uintptr_t* newVTable = new uintptr_t[64];

		memcpy(newVTable, vtable, 64 * sizeof(uintptr_t));

		g_origInitializeGraphics = (decltype(g_origInitializeGraphics))newVTable[4];
		newVTable[4] = (uintptr_t)NullInitializeGraphics;

		newVTable[1] = (uintptr_t)NullRender;
		newVTable[6] = (uintptr_t)NullCopyBuffer;

		*(uintptr_t**)ui = newVTable;

		return ui;
	}

	virtual HRESULT Initialize(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6) override
	{
		LOG_CALL();

		return m_baseRgsc->Initialize(a1, a2, a3, a4, a5, a6);
	}

	virtual void* m_07() override
	{
		LOG_CALL();

		return m_baseRgsc->m_07();
	}

	virtual void* m_08() override
	{
		LOG_CALL();

		return m_baseRgsc->m_08();
	}

	virtual void* m_09() override
	{
		LOG_CALL();

		return m_baseRgsc->m_09();
	}

	virtual void* m_10() override
	{
		LOG_CALL();

		return m_baseRgsc->m_10();
	}

	virtual void SetSomething(void* a1) override
	{
		LOG_CALL();

		return m_baseRgsc->SetSomething(a1);
	}

	virtual void* SetSomethingSteamTicket(void* a1) override
	{
		LOG_CALL();

		return m_baseRgsc->SetSomethingSteamTicket(a1);
	}

	virtual void* m_11() override
	{
		LOG_CALL();

		return m_baseRgsc->m_11();
	}

	virtual bool m_12(void* a1, void* a2) override
	{
		LOG_CALL();

		return m_baseRgsc->m_12(a1, a2);
	}

	virtual bool m_13() override
	{
		LOG_CALL();

		return m_baseRgsc->m_13();
	}

	virtual void* m_14() override
	{
		LOG_CALL();

		return m_baseRgsc->m_14();
	}

	virtual void* m_15() override
	{
		LOG_CALL();

		return m_baseRgsc->m_15();
	}

	virtual void* m_16() override
	{
		LOG_CALL();

		return m_baseRgsc->m_16();
	}

	virtual void* m_17() override
	{
		LOG_CALL();

		return m_baseRgsc->m_17();
	}

private:
	IRgsc* m_baseRgsc;
};

static RgscStub* g_rgsc;

IRgsc* GetScSdkStub()
{
	LOG_CALL();

	auto getFunc = (IRgsc*(*)())GetProcAddress(GetModuleHandle(L"socialclub.dll"), MAKEINTRESOURCEA(1));
	
	if (!g_rgsc)
	{
		g_rgsc = new RgscStub(getFunc());
	}

	return g_rgsc;
}

FARPROC GetProcAddressStub(HMODULE hModule, LPCSTR name)
{
	if (name == MAKEINTRESOURCEA(1) && hModule == GetModuleHandle(L"socialclub.dll"))
	{
		return (FARPROC)GetScSdkStub;
	}

	return GetProcAddress(hModule, name);
}

static HookFunction hookFunction([] ()
{
	hook::iat("kernel32.dll", GetProcAddressStub, "GetProcAddress");
});