#include <StdInc.h>
#include "DiagLocal.h"

#include <combaseapi.h>
#include <shellapi.h>
#include <citversion.h>

#include <fstream>
#include <vector>
#include <chrono>
#include <sstream>

#pragma comment(lib, "shell32.lib")

struct CfxDiagnosticsCommand {
    std::string label;
    TDiagnosticsFunction function;
};

static std::vector<CfxDiagnosticsCommand> g_diagCommands;

int main() {
    CoInitialize(nullptr);

    // Run all initialization routines
    InitFunctionBase::RunAll();

    SYSTEMTIME curTime;
    GetSystemTime(&curTime);

    std::wstring tempDir = _wgetenv(L"temp");
    tempDir += fmt::sprintf(L"\\CfxDiag_%04d_%02d_%02d_%02d_%02d_%02d.log",
                            curTime.wYear, curTime.wMonth, curTime.wDay,
                            curTime.wHour, curTime.wMinute, curTime.wSecond);

    {
        std::ofstream outLog(tempDir);

        outLog << fmt::sprintf("CfxDiag running at %04d-%02d-%02d at %02d:%02d:%02d...",
                               curTime.wYear, curTime.wMonth, curTime.wDay,
                               curTime.wHour, curTime.wMinute, curTime.wSecond)
               << std::endl;

        fmt::printf("This is CfxDiag version %d.\n", BASE_EXE_VERSION);
        fmt::printf("Please wait while all the procedures finish!\n", BASE_EXE_VERSION);

        // Gather diagnostics data
        for (size_t i = 0; i < g_diagCommands.size(); i++) {
            const auto& cmdEntry = g_diagCommands[i];

            fmt::printf("Processing %zu of %zu (%s)\n", i + 1, g_diagCommands.size(), cmdEntry.label);

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

    fmt::printf("Done. Thanks for using CfxDiag!\n");

    // This is okay here
    system("pause");

    return 0;
}

void RegisterCfxDiagnosticsCommand(const std::string& label, const TDiagnosticsFunction& function) {
    CfxDiagnosticsCommand cmd;
    cmd.label = label;
    cmd.function = function;

    g_diagCommands.emplace_back(std::move(cmd));
}

std::string CfxDiagnosticsOutputBuffer::ToString() {
    return m_outputStream.str();
}

void CfxDiagnosticsOutputBuffer::AppendText(const std::string& data) {
    m_outputStream << data << std::endl;
}

bool CfxDiagnosticsOutputBuffer::AppendCommand(const std::string& commandLine, std::chrono::milliseconds timeout) {
    // Use smart handles to manage resources
    auto hRead = std::make_unique<HANDLE>(nullptr, &::CloseHandle);
    auto hWrite = std::make_unique<HANDLE>(nullptr, &::CloseHandle);
    auto hWriteError = std::make_unique<HANDLE>(nullptr, &::CloseHandle);

    SECURITY_ATTRIBUTES sa = {sizeof(sa), nullptr, TRUE};
    CreatePipe(hRead.get(), hWrite.get(), &sa, 0);
    DuplicateHandle(GetCurrentProcess(), hWrite.get(), GetCurrentProcess(), hWriteError.get(), 0, TRUE, DUPLICATE_SAME_ACCESS);

    STARTUPINFOW si = {sizeof(si), nullptr, nullptr, hWrite.get(), hWriteError.get(), nullptr, STARTF_USESTDHANDLES};
    PROCESS_INFORMATION pi = {0};

    // OK, go.
    BOOL created = CreateProcessW(nullptr, _wcsdup(ToWide(commandLine).c_str()), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi);

    bool success = false;

    auto start = std::chrono::high_resolution_clock::now().time_since_epoch();

    if (created) {
        while (true) {
            HANDLE waitHandles[] = {pi.hProcess, hRead.get()};
            DWORD waitResult = WaitForMultipleObjects(_countof(waitHandles), waitHandles, FALSE, 500);

            if (waitResult == WAIT_OBJECT_0 + 1) {
                DWORD bytesAvail = 0;
                PeekNamedPipe(hRead.get(), nullptr, 0, nullptr, &bytesAvail, nullptr);

                if (bytesAvail > 0) {
                    // Inner loop, read from the pipe
                    DWORD bytesRead;
                    char buffer[8192];

                    if (ReadFile(hRead.get(), buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead) {
                        m_outputStream << std::string(buffer, bytesRead);
                    }
                }
            } else if (waitResult == WAIT_OBJECT_0) {
                success = true;
                break;
            }

            if ((std::chrono::high_resolution_clock::now().time_since_epoch() - start > timeout)) {
                m_outputStream << std::endl << "!! Execution timed out !!" << std::endl;
                break;
            }
        }
    }

    return success;
}
