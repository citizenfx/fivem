/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <regex>
#include <vector>

#include <include/cef_scheme.h>

class NUISchemeHandlerFactory : public CefSchemeHandlerFactory
{
public:
	virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request);

	void SetRequestBlacklist(const std::vector<std::regex>& requestBlacklist);

	IMPLEMENT_REFCOUNTING(NUISchemeHandlerFactory);

private:
	std::vector<std::regex> m_requestBlacklist;
};

DECLARE_INSTANCE_TYPE(NUISchemeHandlerFactory);
