/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <CefOverlay.h>
#include <NetLibrary.h>
#include <strsafe.h>
#include <GlobalEvents.h>
#include <nutsnbolts.h>

static LONG WINAPI TerminateInstantly(LPEXCEPTION_POINTERS pointers)
{
	if (pointers->ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT)
	{
		TerminateProcess(GetCurrentProcess(), 0xDEADCAFE);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

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
		else if (!_wcsicmp(type, L"exit"))
		{
			// queue an ExitProcess on the next game frame
			OnGameFrame.Connect([] ()
			{
				AddVectoredExceptionHandler(FALSE, TerminateInstantly);

				CefShutdown();

				ExitProcess(0);
			});
		}
	});

	OnMsgConfirm.Connect([] ()
	{
		nui::SetMainUI(true);

		nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");
	});
});