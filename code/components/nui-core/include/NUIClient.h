/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <mutex>

#include <NUIWindow.h>
#include <include/cef_client.h>

class NUIClient : public CefClient, public CefLifeSpanHandler, public CefDisplayHandler, public CefContextMenuHandler, public CefLoadHandler
{
private:
	fwRefContainer<NUIWindow> m_window;
	bool m_windowValid;

	std::recursive_mutex m_windowLock;

	CefRefPtr<CefBrowser> m_browser;

public:
	NUIClient(NUIWindow* window);

	inline NUIWindow*	GetWindow()					{ return m_window.GetRef();		}
	inline CefBrowser*	GetBrowser()				{ return m_browser.get();		}

	inline bool			GetWindowValid()			{ return m_windowValid;			}
	inline void			SetWindowValid(bool valid)	{ m_windowValid = valid;		}

	inline std::recursive_mutex& GetWindowLock()
	{
		return m_windowLock;
	}

public:
	static fwEvent<NUIClient*> OnClientCreated;

// CefClient
protected:
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override;
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override;

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

public:
	typedef std::function<bool(CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>)> TProcessMessageHandler;

	void AddProcessMessageHandler(std::string key, TProcessMessageHandler handler);

private:
	std::map<std::string, TProcessMessageHandler> m_processMessageHandlers;

	CefRefPtr<CefRenderHandler> m_renderHandler;

// CefLifeSpanHandler
protected:
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

// CefLoadHandler
protected:
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transitionType) override;

	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;

// CefContextMenuHandler
protected:
	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;

// CefDisplayHandler
protected:
	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) override;

	IMPLEMENT_REFCOUNTING(NUIClient);
};