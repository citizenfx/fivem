#include <StdInc.h>
#include "DiagLocal.h"

#include <combaseapi.h>

#include <shellapi.h>
#include <citversion.h>

#include <fstream>

#pragma comment(lib, "shell32.lib")

struct CfxDiagnosticsCommand
{
	std::string label;
	TDiagnosticsFunction function;
};

static std::vector<CfxDiagnosticsCommand> g_diagCommands;

int main()
{
	CoInitialize(nullptr);

	// run all initialization routines
	InitFunctionBase::RunAll();

	SYSTEMTIME curTime;
	GetSystemTime(&curTime);

	std::wstring tempDir = _wgetenv(L"temp");
	tempDir += fmt::sprintf(L"\\CfxDiag_%04d_%02d_%02d_%02d_%02d_%02d.log", curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond);

	{
		std::ofstream outLog(tempDir);

		outLog
			<< fmt::sprintf(
				"CfxDiag running at %04d-%02d-%02d at %02d:%02d:%02d...",
				curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond
			)
			<< std::endl;

		fmt::printf("This is CfxDiag version %d.\n", BASE_EXE_VERSION);

		// TODO: add a print for the game directory used

		fmt::printf("Please wait while all the procedures finish!\n", BASE_EXE_VERSION);

		// gather diagnostics data
		for (int i = 0; i < g_diagCommands.size(); i++)
		{
			const auto& cmdEntry = g_diagCommands[i];

			fmt::printf("Processing %d of %d (%s)\n", i + 1, g_diagCommands.size(), cmdEntry.label);

			auto buffer = std::make_shared<CfxDiagnosticsOutputBuffer>();
			cmdEntry.function(buffer);

			outLog << "--------------------------------------------" << std::endl;
			outLog << cmdEntry.label << std::endl;
			outLog << "--------------------------------------------" << std::endl;
			outLog << buffer->ToString();
			outLog << std::endl;
			outLog << std::endl;
		}

		outLog.close();
	}

	ShellExecuteW(nullptr, L"edit", tempDir.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

	// TODO: upload our little log file, or copy it if failed

	fmt::printf("Done. Thanks for using CfxDiag!\n");

	// this is ok here
	system("pause");

	return 0;
}

void RegisterCfxDiagnosticsCommand(const std::string& label, const TDiagnosticsFunction& function)
{
	CfxDiagnosticsCommand cmd;
	cmd.label = label;
	cmd.function = function;

	g_diagCommands.emplace_back(std::move(cmd));
}

std::string CfxDiagnosticsOutputBuffer::ToString()
{
	return m_outputStream.str();
}

void CfxDiagnosticsOutputBuffer::AppendText(const std::string& data)
{
	m_outputStream << data << std::endl;
}

bool CfxDiagnosticsOutputBuffer::AppendCommand(const std::string& commandLine, std::chrono::milliseconds timeout /* = std::chrono::milliseconds(15000) */)
{
	HANDLE hRead;
	HANDLE hWrite;

	// create inheritable pipe
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = nullptr;
	sa.bInheritHandle = TRUE;

	CreatePipe(&hRead, &hWrite, &sa, 0);

	// make stderr handle copy
	HANDLE hWriteError;
	DuplicateHandle(GetCurrentProcess(), hWrite, GetCurrentProcess(), &hWriteError, 0, TRUE, DUPLICATE_SAME_ACCESS);

	// set up process launch
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	si.hStdInput = nullptr;
	si.hStdOutput = hWrite;
	si.hStdError = hWriteError;
	si.dwFlags = STARTF_USESTDHANDLES;

	// OK, go.
	PROCESS_INFORMATION pi = { 0 };
	BOOL created = CreateProcessW(nullptr, _wcsdup(ToWide(commandLine).c_str()), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);

	bool success = false;

	auto start = std::chrono::high_resolution_clock::now().time_since_epoch();

	if (created)
	{
		while (true)
		{
			HANDLE waitHandles[] =
			{
				pi.hProcess,
				hRead
			};

			DWORD waitResult = WaitForMultipleObjects(_countof(waitHandles), waitHandles, FALSE, 500);

			if (waitResult == WAIT_OBJECT_0 + 1)
			{
				DWORD bytesAvail = 0;
				PeekNamedPipe(hRead, nullptr, 0, nullptr, &bytesAvail, nullptr);

				if (bytesAvail > 0)
				{
					// inner loop, read from pipe
					DWORD bytesRead;
					char buffer[8192];

					if (ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead)
					{
						m_outputStream << std::string(buffer, bytesRead);
					}
				}
			}
			else if (waitResult == WAIT_OBJECT_0)
			{
				success = true;
				break;
			}

			if ((std::chrono::high_resolution_clock::now().time_since_epoch() - start > timeout))
			{
				m_outputStream << std::endl << "!! execution timed out !!" << std::endl;

				break;
			}
		}
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	CloseHandle(hRead);
	CloseHandle(hWrite);
	CloseHandle(hWriteError);

	return success;
}
