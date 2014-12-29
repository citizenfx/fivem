/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_NUI_CORE
#define OVERLAY_DECL __declspec(dllexport)
#else
#define OVERLAY_DECL __declspec(dllimport)
#endif

#include <memory>

#include "grcTexture.h"

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

enum NUIPaintType
{
	NUIPaintTypeDummy,
	NUIPaintTypePostRender
};

class 
#ifdef COMPILING_NUI_CORE
	__declspec(dllexport)
#endif
	NUIWindow : public fwRefCountable
{
private:
	CefRefPtr<CefClient> m_client;

	void(__cdecl* m_onClientCreated)(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context);

	void Initialize(CefString url);

public:
	NUIWindow(bool primary, int width, int height);

private:
	bool primary;
	int width;
	int height;

	int roundedWidth;
	int roundedHeight;

	bool renderBufferDirty;
	CRITICAL_SECTION renderBufferLock;
	char* renderBuffer;

	std::queue<CefRect> dirtyRects;

	std::set<std::string> pollQueue;

	rage::grcTexture* nuiTexture;

	NUIPaintType paintType;

public:
	inline int		GetWidth()			{ return width;			}
	inline int		GetHeight()			{ return height;		}

	inline void*	GetRenderBuffer()	{ return renderBuffer;	}
	inline int		GetRoundedWidth()	{ return roundedWidth;	}

public:
	void			AddDirtyRect(const CefRect& rect);

	inline void		MarkRenderBufferDirty() { renderBufferDirty = true; }

public:
	static fwRefContainer<NUIWindow> Create(bool primary, int width, int height, CefString url);

public:
	~NUIWindow();

	void UpdateFrame();

	void Invalidate();

	void SetPaintType(NUIPaintType type);

	CefBrowser* GetBrowser();

	void SignalPoll(std::string& argument);

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

	inline rage::grcTexture* GetTexture() { return nuiTexture; }

	inline NUIPaintType GetPaintType() { return paintType; }
};

namespace nui
{
	//void EnterV8Context(const char* type);
	//void LeaveV8Context(const char* type);
	//void InvokeNUICallback(const char* type, const CefString& name, const CefV8ValueList& arguments);
	void OVERLAY_DECL ReloadNUI();

	void OVERLAY_DECL CreateFrame(fwString frameName, fwString frameURL);
	void OVERLAY_DECL DestroyFrame(fwString frameName);
	void OVERLAY_DECL SignalPoll(fwString frameName);

	void OVERLAY_DECL GiveFocus(bool hasFocus);
	bool OVERLAY_DECL HasMainUI();
	void OVERLAY_DECL SetMainUI(bool enable);

	void ProcessInput();

	void OVERLAY_DECL ExecuteRootScript(const char* scriptBit);

	OVERLAY_DECL CefBrowser* GetBrowser();

	bool OnPreLoadGame(void* cefSandbox);

	OVERLAY_DECL NUISchemeHandlerFactory* GetSchemeHandlerFactory();

	extern
		OVERLAY_DECL
		fwEvent<const wchar_t*, const wchar_t*> OnInvokeNative;
}

#define REQUIRE_IO_THREAD()   assert(CefCurrentlyOn(TID_IO));

struct nui_s
{
	bool initialized;

	DWORD nuiWidth;
	DWORD nuiHeight;
};

extern
	OVERLAY_DECL
	fwEvent<const char*, CefRefPtr<CefResourceHandler>&> OnSchemeCreateRequest;

extern nui_s g_nui;