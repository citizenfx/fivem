/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <time.h>
#include <dbghelp.h>
#include <client/windows/handler/exception_handler.h>
#include <client/windows/crash_generation/client_info.h>
#include <client/windows/crash_generation/crash_generation_server.h>
#include <common/windows/http_upload.h>

#include <commctrl.h>
#include <shellapi.h>

#include <json.hpp>

#include <regex>
#include <sstream>

#include <optional>

#include <CfxSubProcess.h>

#include <citversion.h>

using json = nlohmann::json;

static json load_error_pickup()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\error-pickup").c_str(), L"rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		int len = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<uint8_t> text(len);
		fread(&text[0], 1, len, f);

		fclose(f);

		return json::parse(text);
	}

	return json(nullptr);
}

using namespace google_breakpad;

static ExceptionHandler* g_exceptionHandler;

struct ErrorData
{
	std::string errorName;
	std::string errorDescription;

	ErrorData()
	{
	}

	ErrorData(const std::string& errorName, const std::string& errorDescription)
		: errorName(errorName), errorDescription(errorDescription)
	{

	}
};

static ErrorData LookupError(uint32_t hash)
{
	FILE* f = _wfopen(MakeRelativeGamePath(L"update/x64/data/errorcodes/american.txt").c_str(), L"r");

	if (f)
	{
		char line[8192] = { 0 };

		while (fgets(line, 8191, f))
		{
			if (line[0] == '[')
			{
				strrchr(line, ']')[0] = '\0';

				if (HashString(&line[1]) == hash)
				{
					char data[8192] = { 0 };
					fgets(data, 8191, f);

					return ErrorData{&line[1], data};
				}
			}
		}
	}

	return ErrorData{};
}

static std::optional<std::tuple<ErrorData, uint64_t>> LoadErrorData()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\error_out").c_str(), L"rb");

	if (f)
	{
		uint32_t error;
		uint64_t retAddr;
		fread(&error, 1, 4, f);
		fread(&retAddr, 1, 8, f);
		fclose(f);

		return { { LookupError(error), retAddr } };
	}

	return {};
}

template<typename T>
static std::string ParseLinks(const T& text)
{
	// parse hyperlinks in the error text
	std::regex url_re(R"((http|ftp|https):\/\/[\w-]+(\.[\w-]+)+([\w.,@?^=%&amp;:\/~+#-]*[\w@?^=%&amp;\/~+#-])?)");

	std::ostringstream oss;
	std::regex_replace(std::ostreambuf_iterator<char>(oss),
		text.begin(), text.end(), url_re, "<A HREF=\"$&\">$&</A>");

	return oss.str();
}

static void OverloadCrashData(TASKDIALOGCONFIG* config)
{
	// error files?
	{
		auto data = LoadErrorData();

		if (data)
		{
			_wunlink(MakeRelativeCitPath(L"cache\\error_out").c_str());

			static ErrorData errData = std::get<ErrorData>(*data);
			static uint64_t retAddr = std::get<uint64_t>(*data);

			if (!errData.errorName.empty())
			{
				static std::wstring errTitle = fmt::sprintf(L"RAGE error: %s", ToWide(errData.errorName));
				static std::wstring errDescription = fmt::sprintf(L"A game error (at %016llx) caused " PRODUCT_NAME L" to stop working. "
					L"A crash report has been uploaded to the " PRODUCT_NAME L" developers.\n"
					L"If you require immediate support, please visit <A HREF=\"https://forum.fivem.net/\">FiveM.net</A> and mention the details below.\n\n%s",
					retAddr,
					ToWide(ParseLinks(errData.errorDescription)));

				config->pszMainInstruction = errTitle.c_str();
				config->pszContent = errDescription.c_str();

				return;
			}
		}
	}

	// FatalError crash pickup?
	{
		json pickup = load_error_pickup();

		if (!pickup.is_null())
		{
			_wunlink(MakeRelativeCitPath(L"cache\\error-pickup").c_str());

			static std::wstring errTitle = fmt::sprintf(PRODUCT_NAME L" has encountered an error");
			static std::wstring errDescription = ToWide(fmt::sprintf("%s\n\nIf you require immediate support, please visit <A HREF=\"https://forum.fivem.net/\">FiveM.net</A> and mention the details in this window.", ParseLinks(pickup["message"].get<std::string>())));

			config->pszMainInstruction = errTitle.c_str();
			config->pszContent = errDescription.c_str();

			return;
		}
	}
}

static std::wstring GetAdditionalData()
{
	{
		json error_pickup = load_error_pickup();

		if (!error_pickup.is_null())
		{
			if (error_pickup["line"] != 99999)
			{
				error_pickup["type"] = "error_pickup";
			}

			return ToWide(error_pickup.dump());
		}
	}

	{
		auto errorData = LoadErrorData();

		if (errorData)
		{
			json jsonData = json::object({
				{ "type", "rage_error" },
				{ "key", std::get<ErrorData>(*errorData).errorName },
				{ "description", std::get<ErrorData>(*errorData).errorDescription },
				{ "retAddr", std::get<uint64_t>(*errorData) },
			});

			return ToWide(jsonData.dump());
		}
	}

	return L"{}";
}

void InitializeDumpServer(int inheritedHandle, int parentPid)
{
	static bool g_running = true;

	HANDLE inheritedHandleBit = (HANDLE)inheritedHandle;
	static HANDLE parentProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | SYNCHRONIZE, FALSE, parentPid);

	CrashGenerationServer::OnClientConnectedCallback connectCallback = [] (void*, const ClientInfo* info)
	{

	};

	CrashGenerationServer::OnClientDumpRequestCallback dumpCallback = [] (void*, const ClientInfo* info, const std::wstring* filePath)
	{
		std::map<std::wstring, std::wstring> parameters;
#ifdef GTA_NY
		parameters[L"ProductName"] = L"CitizenFX";
		parameters[L"Version"] = L"1.0";
		parameters[L"BuildID"] = L"20141213000000"; // todo i bet
#elif defined(GTA_FIVE)
		parameters[L"ProductName"] = L"FiveM";
		parameters[L"Version"] = va(L"1.3.0.%d", BASE_EXE_VERSION);
		parameters[L"BuildID"] = L"20170101"; // todo i bet

        parameters[L"prod"] = L"FiveM";
        parameters[L"ver"] = L"1.0";
#endif

		parameters[L"ReleaseChannel"] = L"release";

		parameters[L"AdditionalData"] = GetAdditionalData();

		std::wstring responseBody;
		int responseCode;

		std::map<std::wstring, std::wstring> files;
		files[L"upload_file_minidump"] = *filePath;

		TerminateProcess(parentProcess, -2);

		static std::optional<std::wstring> crashId;

		static const TASKDIALOG_BUTTON buttons[] = {
			{ 42, L"Close" }
		};

		static TASKDIALOGCONFIG taskDialogConfig = { 0 };
		taskDialogConfig.cbSize = sizeof(taskDialogConfig);
		taskDialogConfig.hInstance = GetModuleHandle(nullptr);
		taskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_EXPAND_FOOTER_AREA | TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER;
		taskDialogConfig.dwCommonButtons = 0;
		taskDialogConfig.cButtons = 1;
		taskDialogConfig.pButtons = buttons;
		taskDialogConfig.pszWindowTitle = PRODUCT_NAME L" Fatal Error";
		taskDialogConfig.pszMainIcon = TD_ERROR_ICON;
		taskDialogConfig.pszMainInstruction = PRODUCT_NAME L" has stopped working";
		taskDialogConfig.pszContent = L"An error caused " PRODUCT_NAME L" to stop working. A crash report is being uploaded to the " PRODUCT_NAME L" developers. If you require immediate support, please visit <A HREF=\"https://forum.fivem.net/\">FiveM.net</A> and mention the details below.";
		taskDialogConfig.pszExpandedInformation = L"...";
		taskDialogConfig.pfCallback = [](HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam, LONG_PTR data)
		{
			if (type == TDN_HYPERLINK_CLICKED)
			{
				ShellExecute(nullptr, L"open", (LPCWSTR)lParam, nullptr, nullptr, SW_NORMAL);
			}
			else if (type == TDN_BUTTON_CLICKED)
			{
				return S_OK;
			}
			else if (type == TDN_CREATED)
			{
				SendMessage(hWnd, TDM_ENABLE_BUTTON, 42, 0);
				SendMessage(hWnd, TDM_SET_MARQUEE_PROGRESS_BAR, 1, 0);
				SendMessage(hWnd, TDM_SET_PROGRESS_BAR_MARQUEE, 1, 15);
			}
			else if (type == TDN_TIMER)
			{
				if (crashId)
				{
					if (!crashId->empty())
					{
						SendMessage(hWnd, TDM_SET_ELEMENT_TEXT, TDE_EXPANDED_INFORMATION, (WPARAM)va(L"Crash ID: %s (use Ctrl+C to copy)", crashId->c_str()));
					}
					else
					{
						SendMessage(hWnd, TDM_SET_PROGRESS_BAR_STATE, PBST_ERROR, 0);
					}

					SendMessage(hWnd, TDM_ENABLE_BUTTON, 42, 1);
					SendMessage(hWnd, TDM_SET_MARQUEE_PROGRESS_BAR, 0, 0);
					SendMessage(hWnd, TDM_SET_PROGRESS_BAR_POS, 100, 0);
					SendMessage(hWnd, TDM_SET_PROGRESS_BAR_STATE, PBST_NORMAL, 0);

					crashId.reset();
				}
			}

			return S_FALSE;
		};

		OverloadCrashData(&taskDialogConfig);

		auto thread = std::thread([=]()
		{
			TaskDialogIndirect(&taskDialogConfig, nullptr, nullptr, nullptr);
		});

#ifdef GTA_NY
		if (HTTPUpload::SendRequest(L"http://cr.citizen.re:5100/submit", parameters, files, nullptr, &responseBody, &responseCode))
#elif defined(GTA_FIVE)
		if (HTTPUpload::SendRequest(L"http://updater.fivereborn.com:1127/post", parameters, files, nullptr, &responseBody, &responseCode))
#endif
		{
			crashId = responseBody;
		}
		else
		{
			crashId = L"";
		}

		if (thread.joinable())
		{
			thread.join();
		}
	};

	CrashGenerationServer::OnClientExitedCallback exitCallback = [] (void*, const ClientInfo* info)
	{
	};

	CrashGenerationServer::OnClientUploadRequestCallback uploadCallback = [] (void*, DWORD)
	{

	};

	std::wstring crashDirectory = MakeRelativeCitPath(L"crashes");

	std::wstring pipeName = L"\\\\.\\pipe\\CitizenFX_Dump";

	CrashGenerationServer server(pipeName, nullptr, connectCallback, nullptr, dumpCallback, nullptr, exitCallback, nullptr, uploadCallback, nullptr, true, &crashDirectory);
	if (server.Start())
	{
		SetEvent(inheritedHandleBit);
		WaitForSingleObject(parentProcess, INFINITE);
	}
}

void CheckModule(void* address)
{
	const char* moduleBaseString = "";
	HMODULE module = nullptr;

	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)address, &module))
	{
		char filename[MAX_PATH];
		GetModuleFileNameA(module, filename, _countof(filename));

		moduleBaseString = va(" - %s+%X", strrchr(filename, '\\') + 1, (char*)address - (char*)module);
	}

	if (module)
	{
		char filename[MAX_PATH];
		GetModuleFileNameA(module, filename, _countof(filename));

		const wchar_t* blame = nullptr;
		const wchar_t* blame_two = nullptr;

		if (strstr(filename, "nvwgf"))
		{
			blame = L"NVIDIA GPU drivers";
			blame_two = L"This is not the fault of the " PRODUCT_NAME L" developers, and can not be resolved by them. NVIDIA does not provide any error reporting contacts to use to report this problem, nor do they provide "
						L"debugging information that the developers can use to resolve this issue.";
		}

		if (strstr(filename, "guard64"))
		{
			blame = L"Comodo Internet Security";
			blame_two = L"Please uninstall Comodo Internet Security and try again, or report the issue on the Comodo forums.";
		}

		if (strstr(filename, ".asi"))
		{
			blame = L"a third-party game plugin";
			blame_two = L"Please try removing the \"plugins\" folder in your" PRODUCT_NAME L" installation and restarting the game.";
		}

		if (blame)
		{
			auto wbs = ToWide(moduleBaseString);

			TASKDIALOGCONFIG taskDialogConfig = { 0 };
			taskDialogConfig.cbSize = sizeof(taskDialogConfig);
			taskDialogConfig.hInstance = GetModuleHandle(nullptr);
			taskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_EXPAND_FOOTER_AREA;
			taskDialogConfig.dwCommonButtons = TDCBF_CLOSE_BUTTON;
			taskDialogConfig.pszWindowTitle = PRODUCT_NAME L" Fatal Error";
			taskDialogConfig.pszMainIcon = TD_ERROR_ICON;
			taskDialogConfig.pszMainInstruction = PRODUCT_NAME L" has stopped working";
			taskDialogConfig.pszContent = va(L"An error in %s caused " PRODUCT_NAME L" to stop working. A crash report has been uploaded to the " PRODUCT_NAME L" developers.\n%s", blame, blame_two);
			taskDialogConfig.pszExpandedInformation = wbs.c_str();
			taskDialogConfig.pfCallback = [](HWND, UINT type, WPARAM wParam, LPARAM lParam, LONG_PTR data)
			{
				if (type == TDN_HYPERLINK_CLICKED)
				{
					ShellExecute(nullptr, L"open", (LPCWSTR)lParam, nullptr, nullptr, SW_NORMAL);
				}
				else if (type == TDN_BUTTON_CLICKED)
				{
					return S_OK;
				}

				return S_FALSE;
			};

			TaskDialogIndirect(&taskDialogConfig, nullptr, nullptr, nullptr);
		}
	}
}

bool InitializeExceptionHandler()
{
	// don't initialize when under a debugger, as debugger filtering is only done when execution gets to UnhandledExceptionFilter in basedll
	if (IsDebuggerPresent())
	{
		/*SetUnhandledExceptionFilter([] (LPEXCEPTION_POINTERS pointers) -> LONG
		{
			__debugbreak();

			return 0;
		});*/

		return false;
	}

	std::wstring crashDirectory = MakeRelativeCitPath(L"crashes");
	CreateDirectory(crashDirectory.c_str(), nullptr);

	wchar_t* dumpServerBit = wcsstr(GetCommandLine(), L"-dumpserver");

	if (dumpServerBit)
	{
		wchar_t* parentPidBit = wcsstr(GetCommandLine(), L"-parentpid:");

		InitializeDumpServer(wcstol(&dumpServerBit[12], nullptr, 10), wcstol(&parentPidBit[11], nullptr, 10));

		return true;
	}

	CrashGenerationClient* client = new CrashGenerationClient(L"\\\\.\\pipe\\CitizenFX_Dump", (MINIDUMP_TYPE)(MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo), new CustomClientInfo());

	if (!client->Register())
	{
		auto applicationName = MakeCfxSubProcess(L"DumpServer");

		// prepare initial structures
		STARTUPINFO startupInfo = { 0 };
		startupInfo.cb = sizeof(STARTUPINFO);

		PROCESS_INFORMATION processInfo = { 0 };

		// create an init handle
		SECURITY_ATTRIBUTES securityAttributes = { 0 };
		securityAttributes.bInheritHandle = TRUE;

		HANDLE initEvent = CreateEvent(&securityAttributes, TRUE, FALSE, nullptr);

		// create the command line including argument
		wchar_t commandLine[MAX_PATH * 8];
		if (_snwprintf(commandLine, _countof(commandLine), L"\"%s\" -dumpserver:%i -parentpid:%i", applicationName, (int)initEvent, GetCurrentProcessId()) >= _countof(commandLine))
		{
			return false;
		}

		BOOL result = CreateProcess(applicationName, commandLine, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startupInfo, &processInfo);

		if (result)
		{
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}

		DWORD waitResult = WaitForSingleObject(initEvent, 7500);
		if (!client->Register())
		{
			trace("Could not register with breakpad server.\n");
		}
	}

	g_exceptionHandler = new ExceptionHandler(
							L"",
							[](void* context, EXCEPTION_POINTERS* exinfo,
								MDRawAssertionInfo* assertion)
							{
								void* pEx = exinfo->ExceptionRecord->ExceptionAddress;

								CheckModule(pEx);

								return true;
							},
							[] (const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
							{
								return succeeded;
							},
							nullptr,
							ExceptionHandler::HANDLER_ALL,
							client
						);

	g_exceptionHandler->set_handle_debug_exceptions(true);

	// disable Windows' SetUnhandledExceptionFilter
#ifdef _M_AMD64
	DWORD oldProtect;

	LPVOID unhandledFilters[] = { 
		GetProcAddress(GetModuleHandle(L"kernelbase.dll"), "SetUnhandledExceptionFilter"),
		GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetUnhandledExceptionFilter"),
	};

	for (auto unhandledFilter : unhandledFilters)
	{
		if (unhandledFilter)
		{
			VirtualProtect(unhandledFilter, 4, PAGE_EXECUTE_READWRITE, &oldProtect);

			*(uint8_t*)unhandledFilter = 0xC3;
		}
	}
#endif

	return false;
}