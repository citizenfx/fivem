/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIClient.h"
#include "NUIRenderHandler.h"
#include "CefOverlay.h"
#include "memdbgon.h"
#include "HttpClient.h"

#include <rapidjson/document.h>

#include <sstream>

NUIClient::NUIClient(NUIWindow* window)
	: m_window(window)
{
	m_windowValid = true;

	m_renderHandler = new NUIRenderHandler(this);

	auto httpClient = Instance<HttpClient>::Get();
	httpClient->DoGetRequest("https://runtime.fivem.net/nui-blacklist.json", [=](bool success, const char* data, size_t length)
	{
		if (success)
		{
			rapidjson::Document doc;
			doc.Parse(data, length);

			if (!doc.HasParseError())
			{
				if (doc.IsArray())
				{
					for (auto it = doc.Begin(); it != doc.End(); ++it)
					{
						if (it->IsString())
						{
							m_requestBlacklist.emplace_back(it->GetString(), std::regex_constants::ECMAScript | std::regex_constants::icase);
						}
					}
				}
			}
		}
	});
}

void NUIClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transitionType)
{
	GetWindow()->OnClientContextCreated(browser, frame, nullptr);
}

void NUIClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	auto url = frame->GetURL();

	if (url == "nui://game/ui/root.html" && nui::HasMainUI())
	{
		nui::CreateFrame("mpMenu", "nui://game/ui/app/index.html");
	}

	// replace any segoe ui symbol font
	frame->ExecuteJavaScript("[].concat.apply([], Array.from(document.styleSheets).map(a => Array.from(a.rules).filter(b => b.style && b.style.fontFamily))).forEach(a => a.style.fontFamily = a.style.fontFamily.replace(/Segoe UI Symbol/g, 'Segoe UI Emoji'));", "nui://patches", 0);
}

void NUIClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	m_browser = browser;

	OnClientCreated(this);
}

bool NUIClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	auto handler = m_processMessageHandlers.find(message->GetName());
	bool success = false;

	if (handler != m_processMessageHandlers.end())
	{
		success = handler->second(browser, message);
	}
	else
	{
		trace("Unknown NUI process message: %s\n", message->GetName().ToString().c_str());
	}

	return success;
}

void NUIClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
	model->Clear();
}

bool NUIClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level, const CefString& message, const CefString& source, int line)
{
	std::wstring sourceStr = source.ToWString();
	std::wstring messageStr = message.ToWString();

	std::wstringstream msg;
	msg << sourceStr << ":" << line << ", " << messageStr << std::endl;

	OutputDebugString(msg.str().c_str());
	trace("%s\n", ToNarrow(msg.str()));

	return false;
}

auto NUIClient::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) -> ReturnValue
{
	for (auto& reg : m_requestBlacklist)
	{
		std::string url = request->GetURL().ToString();

		if (std::regex_search(url, reg))
		{
			trace("Blocked a request for blacklisted URI %s\n", url);
			return RV_CANCEL;
		}
	}

	return RV_CONTINUE;
}

void NUIClient::AddProcessMessageHandler(std::string key, TProcessMessageHandler handler)
{
	m_processMessageHandlers[key] = handler;
}

CefRefPtr<CefLifeSpanHandler> NUIClient::GetLifeSpanHandler()
{
	return this;
}

CefRefPtr<CefDisplayHandler> NUIClient::GetDisplayHandler()
{
	return this;
}

CefRefPtr<CefContextMenuHandler> NUIClient::GetContextMenuHandler()
{
	return this;
}

CefRefPtr<CefLoadHandler> NUIClient::GetLoadHandler()
{
	return this;
}

CefRefPtr<CefRequestHandler> NUIClient::GetRequestHandler()
{
	return this;
}

CefRefPtr<CefRenderHandler> NUIClient::GetRenderHandler()
{
	return m_renderHandler;
}

fwEvent<NUIClient*> NUIClient::OnClientCreated;
