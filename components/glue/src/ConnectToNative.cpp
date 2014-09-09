#include "StdInc.h"
#include <CefOverlay.h>
#include <NetLibrary.h>
#include <strsafe.h>
#include <GlobalEvents.h>

static InitFunction initFunction([] ()
{
	static NetLibrary* netLibrary;

	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* lib)
	{
		netLibrary = lib;

		netLibrary->OnConnectionError.Connect([] (const char* error)
		{
			nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connectFailed', message: '%s' }, '*');", error));
		});
	});

	nui::OnInvokeNative.Connect([] (const wchar_t* type, const wchar_t* arg)
	{
		if (!_wcsicmp(type, L"connectTo"))
		{
			std::wstring hostnameStrW = arg;
			std::string hostnameStr(hostnameStrW.begin(), hostnameStrW.end());

			static char hostname[256];

			StringCbCopyA(hostname, sizeof(hostname), hostnameStr.c_str());

			char* port = strrchr(hostname, ':');

			if (!port)
			{
				port = "30120";
			}
			else
			{
				*port = '\0';
				port++;
			}

			netLibrary->ConnectToServer(hostname, atoi(port));

			nui::ExecuteRootScript("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connecting' }, '*');");
		}
	});

	OnMsgConfirm.Connect([] ()
	{
		nui::SetMainUI(true);

		nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");
	});
});