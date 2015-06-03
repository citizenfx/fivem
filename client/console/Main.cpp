/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

/* Console-forwarding executable. */

#include "StdInc.h"

static bool SetupPipe(HANDLE* ourHandle, HANDLE* otherHandle, bool ourIsWrite)
{
	HANDLE readPipe;
	HANDLE writePipe;

	if (!CreatePipe(&readPipe, &writePipe, nullptr, 1024))
	{
		return false;
	}

	// define which handle belongs where
	HANDLE otherHandleRef = (ourIsWrite) ? readPipe : writePipe;
	HANDLE ourHandleRef = (ourIsWrite) ? writePipe : readPipe;

	// duplicate the other handle to be inheritable
	if (!DuplicateHandle(GetCurrentProcess(), otherHandleRef, GetCurrentProcess(), otherHandle, 0, TRUE, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
	{
		CloseHandle(ourHandleRef);

		return false;
	}

	// store the handle for our usage
	*ourHandle = ourHandleRef;

	return true;
}

static HANDLE g_processHandle;

static BOOL WINAPI ControlHandler(DWORD ctrlType)
{
	if (g_processHandle)
	{
		TerminateProcess(g_processHandle, -1);
		g_processHandle = NULL;

		return TRUE;
	}

	return FALSE;
}

bool CoreIsDebuggerPresent()
{
	return false;
}

int wmain(int argc, const wchar_t** argv)
{
	// get our application name, and replace the extension with '.exe'.
	const wchar_t* applicationNameIn = argv[0];
	std::vector<wchar_t> applicationName(wcslen(applicationNameIn) + 5);

	wcscpy(&applicationName[0], applicationNameIn);

	// extension
	wchar_t* extStart = wcsrchr(&applicationName[0], L'.');
	wchar_t* separatorStart = wcsrchr(&applicationName[0], L'\\');

	// replace the extension
	if (!extStart || extStart < separatorStart) // if no extension exists, or the . is before the separator, start at the end of the string
	{
		extStart = &applicationName[wcslen(&applicationName[0])];
	}

	wcscpy(extStart, L".exe");

	// replace the command line's extension
	wchar_t* commandLineIn = GetCommandLine();

	// try finding the original application name
	wchar_t* applicationNameSubStr = wcsstr(commandLineIn, applicationNameIn);
	wchar_t* spaceEntry = wcschr(applicationNameSubStr, L' ');

	if (!spaceEntry)
	{
		spaceEntry = L"";
	}

	// and replace it with our 'new' application name
	std::wstring newCommandLine = va(L"\"%s\" %s", &applicationName[0], spaceEntry);

	// next up: application initialization

	// get our STARTUPINFO and modify it to say we use inherited standard handles
	STARTUPINFO startupInfo;
	GetStartupInfo(&startupInfo);

	startupInfo.dwFlags |= STARTF_USESTDHANDLES;

	// initialize the stdin/stdout pipes
	HANDLE hWriteStdin;
	HANDLE hReadStdin;

	HANDLE hWriteStdout;
	HANDLE hReadStdout;

	if (!SetupPipe(&hWriteStdin, &hReadStdin, true))
	{
		return 1;
	}

	if (!SetupPipe(&hReadStdout, &hWriteStdout, false))
	{
		return 2;
	}

	// set up the environment for the client application
	SetConsoleCtrlHandler(ControlHandler, TRUE);
	SetEnvironmentVariable(L"CitizenFX_ToolMode", L"1");

	startupInfo.hStdInput = hReadStdin;
	startupInfo.hStdOutput = hWriteStdout;
	startupInfo.hStdError = hWriteStdout;

	// create the process
	PROCESS_INFORMATION processInfo;

	if (CreateProcess(&applicationName[0], const_cast<wchar_t*>(newCommandLine.c_str()), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startupInfo, &processInfo))
	{
		// store our stdout handle (for writing to it)
		HANDLE hOurStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		// pass the handle to the ctrl-C handler
		g_processHandle = processInfo.hProcess;
		CloseHandle(processInfo.hThread);

		// close our copies of the inherited handles
		CloseHandle(hReadStdin);
		CloseHandle(hWriteStdout);

		// do the main output forwarding loop
		while (true)
		{
			// read from the buffer
			char outputBuffer[4];
			DWORD outRead;

			if (!ReadFile(hReadStdout, outputBuffer, sizeof(outputBuffer), &outRead, nullptr))
			{
				break;
			}

			// should we detach the console? (UI started)
			if (outRead == 1 && outputBuffer[0] == '\x01')
			{
				break;
			}

			// write to our stdout
			WriteFile(hOurStdout, outputBuffer, outRead, &outRead, nullptr);

			// check if the process terminated
			if (WaitForSingleObject(processInfo.hProcess, 0) != WAIT_TIMEOUT)
			{
				break;
			}
		}

		// exit cleanly
		CloseHandle(g_processHandle);
		CloseHandle(hOurStdout);
		CloseHandle(hWriteStdin);
		CloseHandle(hReadStdout);

		return 0;
	}

	return 3;
}