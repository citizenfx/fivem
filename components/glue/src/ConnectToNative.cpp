#include "StdInc.h"
#include <CefOverlay.h>
#include <NetLibrary.h>
#include <strsafe.h>

static InitFunction initFunction([] ()
{
	static NetLibrary* netLibrary;

	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* lib)
	{
		netLibrary = lib;
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
});