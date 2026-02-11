/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "NUIClient.h"
#include "CefOverlay.h"
#include <CoreConsole.h>
#include <json.hpp>
#include "memdbgon.h"
#include <shellapi.h>

#include <skyr/url.hpp>

#include "UrlConfirmationExport.h"

void NuiConsole_SetConvars();

namespace nui
{
std::atomic<bool> nui::g_showUrlConfirmModal{ false };
std::string g_pendingUrl;
std::mutex g_urlModalMutex;
}

static bool IsUrlTrusted(const std::string& url)
{
	static constexpr std::array<std::string_view, 6> trustedDomains = {
		"cfx.re",
		"fivem.net",
		"redm.net",
		"rockstargames.com",
		"rsg.ms",
		"take2games.com"
	};

	// Untrusted subdomains - checked BEFORE trusted domains
	static constexpr std::array<std::string_view, 1> untrustedSubdomains = {
		"users.cfx.re"
	};

	auto parsed = skyr::make_url(url);
	if (!parsed)
	{
		return false;
	}

	std::string host = parsed->host();
	if (host.empty())
	{
		return false;
	}

	std::transform(host.begin(), host.end(), host.begin(),
	[](unsigned char c)
	{
		return static_cast<char>(std::tolower(c));
	});

	// Check untrusted subdomains first
	for (const auto& untrusted : untrustedSubdomains)
	{
		// Exact match with untrusted subdomain
		if (host == untrusted)
		{
			return false;
		}

		// Is a subdomain of untrusted pattern (e.g., "foo.users.cfx.re")
		if (host.size() > untrusted.size())
		{
			size_t suffixStart = host.size() - untrusted.size();
			if (host[suffixStart - 1] == '.' && std::string_view(host).substr(suffixStart) == untrusted)
			{
				return false;
			}
		}
	}

	for (const auto& domain : trustedDomains)
	{
		// Exact match
		if (host == domain)
		{
			return true;
		}

		// Subdomain match: host must end with ".domain"
		if (host.size() > domain.size())
		{
			size_t suffixStart = host.size() - domain.size();
			if (host[suffixStart - 1] == '.' && std::string_view(host).substr(suffixStart) == domain)
			{
				return true;
			}
		}
	}

	return false;
}

static InitFunction initFunction([] ()
{
	auto nuiApp = Instance<NUIApp>::Get();

	nuiApp->AddV8Handler("invokeNative", [] (const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 2)
		{
			auto msg = CefProcessMessage::Create("invokeNative");
			auto argList = msg->GetArgumentList();

			argList->SetSize(2);
			argList->SetString(0, arguments[0]->GetStringValue());

			CefString str = arguments[1]->GetStringValue();
			argList->SetString(1, str);

			CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, msg);
		}

		return CefV8Value::CreateUndefined();
	});

	NUIClient::OnClientCreated.Connect([] (NUIClient* client)
	{
		client->AddProcessMessageHandler("invokeNative", [client] (CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
		{
			auto args = message->GetArgumentList();
			auto nativeType = args->GetString(0);

			nui::OnInvokeNative(nativeType.c_str(), ToWide(args->GetString(1).ToString()).c_str());

			if (nativeType == "quit")
			{
				// TODO: CEF shutdown and native stuff related to it (set a shutdown flag)
				ExitProcess(0);
			}
			else if (nativeType == "openUrl" && !nui::g_showUrlConfirmModal.load())
			{
				std::string arg = args->GetString(1).ToString();

				if (arg.find("http://") == 0 || arg.find("https://") == 0)
				{
					if (client->IsUrlBlocked(arg))
					{
						trace("Blocked opening of URL: %s\n", arg);
						return true;
					}

					if (!IsUrlTrusted(arg))
					{
						std::lock_guard<std::mutex> lock(nui::g_urlModalMutex);
						nui::g_pendingUrl = arg;
						nui::g_showUrlConfirmModal.store(true);
					}
					else
					{
						ShellExecuteW(nullptr, L"open", ToWide(arg).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
					}
				}
			}
			else if (nativeType == "setConvar" || nativeType == "setArchivedConvar")
			{
				if (nui::HasMainUI())
				{
					auto json = nlohmann::json::parse(args->GetString(1).ToString());

					try
					{
						auto cmd = (nativeType == "setArchivedConvar") ? "seta" : "set";
						auto name = json.value("name", "");
						auto value = json.value("value", "");

						se::ScopedPrincipal ps{ se::Principal{"system.console"} };
						console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ cmd, name, value });
					}
					catch (std::exception&)
					{

					}
				}
			}
			else if (nativeType == "getConvars")
			{
				if (nui::HasMainUI())
				{
					NuiConsole_SetConvars();
				}
			}

			return true;
		});
	});
}, 1);
