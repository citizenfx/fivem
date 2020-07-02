/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <functional>
#include <include/cef_app.h>

class NUIApp : public CefApp, public CefRenderProcessHandler, public CefResourceBundleHandler, public CefV8Handler, public CefBrowserProcessHandler
{
public:
	typedef std::function<bool(CefRefPtr<CefBrowser>, CefRefPtr<CefProcessMessage>)> TProcessMessageHandler;
	typedef std::function<CefRefPtr<CefV8Value>(const CefV8ValueList&, CefString&)> TV8Handler;
	typedef std::function<void(CefRefPtr<CefV8Context>)> TContextReleaseHandler;

public:
	void AddProcessMessageHandler(std::string key, TProcessMessageHandler handler);

	void AddV8Handler(std::string key, TV8Handler handler);

	void AddContextReleaseHandler(TContextReleaseHandler handler);

protected:
	// CefApp overrides
	virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;

	virtual bool GetDataResource(int resourceID, void*& data, size_t& data_size) override;

	virtual bool GetDataResourceForScale(int resource_id, ScaleFactor scale_factor, void*& data, size_t& data_size) override;

	virtual bool GetLocalizedString(int messageID, CefString& string) override;

	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;

	virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

	virtual inline CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
	{
		return this;
	}

	// CefBrowserProcessHandler overrides
	virtual void OnContextInitialized() override;

private:
	std::map<std::string, TProcessMessageHandler> m_processMessageHandlers;

	std::map<std::string, TV8Handler> m_v8Handlers;

	std::map<int64, CefRefPtr<CefV8Value>> m_origEventListeners;

	std::vector<TContextReleaseHandler> m_v8ReleaseHandlers;

protected:
	virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

	// CefV8Handler implementation
	virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override;

	IMPLEMENT_REFCOUNTING(NUIApp);
};

DECLARE_INSTANCE_TYPE(NUIApp);
