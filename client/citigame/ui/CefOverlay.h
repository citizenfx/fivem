#pragma once

//#include "grcTexture.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_runnable.h>

#include <queue>

class NUISchemeHandlerFactory : public CefSchemeHandlerFactory
{
public:
	virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request);

	IMPLEMENT_REFCOUNTING(NUISchemeHandlerFactory);
};

class NUIExtensionHandler : public CefV8Handler
{
public:
	NUIExtensionHandler();

	virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception);

	void InvokeNUICallback(const char* type, const CefString& name, const CefV8ValueList& arguments);

	void EnterV8Context(const char* type);
	void ExitV8Context(const char* type);

private:
	std::map<std::string, CefRefPtr<CefV8Value>> _callbackFunctions;
	std::map<std::string, CefRefPtr<CefV8Context>> _callbackContexts;

	IMPLEMENT_REFCOUNTING(NUIExtensionHandler);
};

#include <functional>

class NUIWindow
{
private:
	CefRefPtr<CefClient> m_client;

	void(__cdecl* m_onClientCreated)(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context);

public:
	bool primary;
	int width;
	int height;

	int roundedWidth;
	int roundedHeight;

	bool renderBufferDirty;
	CRITICAL_SECTION renderBufferLock;
	char* renderBuffer;

	std::queue<CefRect> dirtyRects;

	//rage::grcTexture* nuiTexture;
	void* nuiTexture;

	NUIWindow(bool primary, int width, int height);

	void Initialize(CefString url);
	void UpdateFrame();

	CefBrowser* GetBrowser();

	inline void SetClientContextCreated(void(__cdecl* cb)(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context))
	{
		m_onClientCreated = cb;
	}

	inline void OnClientContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
	{
		if (m_onClientCreated)
		{
			m_onClientCreated(browser, frame, context);
		}
	}

	IMPLEMENT_REFCOUNTING(NUIWindow);
};

namespace nui
{
	void EnterV8Context(const char* type);
	void LeaveV8Context(const char* type);
	void InvokeNUICallback(const char* type, const CefString& name, const CefV8ValueList& arguments);
	void ReloadNUI();

	bool OnPreLoadGame(void* cefSandbox);
}

#define REQUIRE_IO_THREAD()   assert(CefCurrentlyOn(TID_IO));

struct nui_s
{
	bool initialized;

	DWORD nuiWidth;
	DWORD nuiHeight;
};

extern nui_s g_nui;