#include "ModalWindow.h"
#include "StdInc.h"

#include <sstream>

#include "os/Firewall.h"

#ifdef _WIN32
#include <commctrl.h>
#endif

// TODO: move this to a single header file containing all product information, shared over the whole project
#ifdef IS_FXSERVER
#define PRODUCT_NAME "Server"
#elif defined(GTA_FIVE)
#define PRODUCT_NAME "FiveM"
#elif defined(IS_RDR3)
#define PRODUCT_NAME "RedM"
#elif defined(GTA_NY)
#define PRODUCT_NAME "LibertyM"
#else
static_assert(false, "No recognized program definition found, are you missing a preprocessor definition?");
#endif

namespace fx
{

void ModalWindow::DisplayError(std::wstring_view title, std::wstring_view body)
{
#ifdef _WIN32
	TASKDIALOGCONFIG taskDialogConfig = { 0 };
	taskDialogConfig.cbSize = sizeof(taskDialogConfig);
	taskDialogConfig.hInstance = GetModuleHandle(nullptr);
	taskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_SIZE_TO_CONTENT;
	taskDialogConfig.dwCommonButtons = TDCBF_CLOSE_BUTTON;
	taskDialogConfig.pszWindowTitle = title.data();
	taskDialogConfig.pszMainIcon = TD_ERROR_ICON;
	taskDialogConfig.pszMainInstruction = NULL;
	taskDialogConfig.pszContent = body.data();

	TaskDialogIndirect(&taskDialogConfig, nullptr, nullptr, nullptr);
#else
	#error ModalWindow::DisplayError is not implemented for this platform
#endif
}

void ModalWindow::DisplayCurlError(std::wstring_view title, std::wstring_view body, int errorCode, std::wstring_view url)
{
	using namespace std::literals::string_view_literals;
	constexpr std::wstring_view contactSupport = L"contact <a href=\"https://support.cfx.re/\">support</a> or file an <a href=\"https://github.com/citizenfx/fivem/issues/new/choose\">issue</a> with the above error."sv;

	assert(errorCode != 0);

	std::wstringstream solutions;
	solutions << L"Potential solutions and things to try:\n";
	solutions << L"  • Make sure you've got a working internet connection\n";
	if (!url.empty())
		solutions << L"  • Make sure <a href=\"" << url << "\">" << url << L"</a> is available in your web browser.\n";

	std::wstringstream explanation;
	explanation << body;

	// see https://curl.se/libcurl/c/libcurl-errors.html for errors and their explanation

	switch (errorCode)
	{
	case 1: // UNSUPPORTED_PROTOCOL
	case 2: // CURLE_FAILED_INIT
	case 3: // CURLE_URL_MALFORMAT
	case 4: // CURLE_NOT_BUILT_IN
	case 5: // CURLE_COULDNT_RESOLVE_PROXY, we didn't set any
	case 8: // CURLE_WEIRD_SERVER_REPLY
		explanation << L"\n\nPlease " << contactSupport;
		break;
	case 6: // CURLE_COULDNT_RESOLVE_HOST
		explanation << L"\n\nUnable to resolve the host, please make sure the steps below work, if not then contact your network administrator.";
		break;
	case 7: // CURLE_COULDNT_CONNECT, request is blocked by a firewall (or similar) on current computer or somewhere else on the network.
	{
		explanation << L"\n\nSomething between your machine and our servers (e.g.: firewall) is blocking your request.";

		solutions << L"  • Allow " PRODUCT_NAME L" through your firewall";

		std::wstring firewall = os::Firewall::GetActiveName(L"");
		if (!firewall.empty())
			solutions << L", currently active: " << firewall;

		solutions << L".\n  • Contact your network administrator.\n";

		break;
	}
	case 9: // CURLE_REMOTE_ACCESS_DENIED
		explanation << L"\n\nUnable to retrieve the requested resource from our server, if you think this is an error then please " << contactSupport;
		break;
	default:
		break;
	}

	explanation << "\n\n" << solutions.rdbuf();

	DisplayError(title, explanation.str());
}
}
