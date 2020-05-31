#include <StdInc.h>
#include <concurrentqueue.h>

#include <fmt/time.h>
#include <filesystem>

#include <HostSharedData.h>

struct TickCountData
{
	uint64_t tickCount;
	SYSTEMTIME initTime;

	TickCountData()
	{
		tickCount = GetTickCount64();
		GetSystemTime(&initTime);
	}
};

static std::string GetProcessName()
{
	wchar_t fn[256];
	GetModuleFileName(GetModuleHandle(NULL), fn, std::size(fn));

	// strip to after first _
	wchar_t* fnPtr = fn;

	if (wcschr(fn, L'_'))
	{
		fnPtr = wcschr(fn, L'_') + 1;
	}

	// also strip to after backslash
	if (wcsrchr(fnPtr, L'\\'))
	{
		fnPtr = wcsrchr(fnPtr, L'\\') + 1;
	}

	// strip extension
	auto dotPtr = wcsrchr(fnPtr, L'.');

	if (dotPtr)
	{
		dotPtr[0] = '\0';
	}

	// return
	std::string rv = ToNarrow(fnPtr);

	if (rv.size() > 14)
	{
		rv = rv.substr(0, 14);
	}

	return rv;
}

static std::string GetThreadName()
{
	static auto _GetThreadDescription = (decltype(&GetThreadDescription))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetThreadDescription");
	std::string rv = fmt::sprintf("%d", GetCurrentThreadId());

	if (_GetThreadDescription)
	{
		PWSTR str;
		HRESULT hr = _GetThreadDescription(GetCurrentThread(), &str);

		if (SUCCEEDED(hr))
		{
			if (isalnum(str[0]) || str[0] == '[')
			{
				rv = ToNarrow(str);
			}

			LocalFree(str);
		}
	}

	if (rv.size() > 20)
	{
		rv = rv.substr(0, 20);
	}

	return rv;
}

static void PerformFileLog(const std::tuple<std::string, std::string>& pair)
{
	static std::vector<char> lineBuffer(8192);
	static size_t lineIndex;
	static SRWLOCK logMutex = SRWLOCK_INIT;

	static HostSharedData<TickCountData> initTickCount("CFX_SharedTickCount");
	static std::string processName = GetProcessName();

	{
		AcquireSRWLockExclusive(&logMutex);

		for (const char* p = std::get<1>(pair).c_str(); *p; ++p)
		{
			if (*p == '\n')
			{
				// flush the line
				static fwPlatformString dateStamp = fmt::sprintf(L"%04d-%02d-%02dT%02d%02d%02d", initTickCount->initTime.wYear, initTickCount->initTime.wMonth,
					initTickCount->initTime.wDay, initTickCount->initTime.wHour, initTickCount->initTime.wMinute, initTickCount->initTime.wSecond);

				static fwPlatformString fp = MakeRelativeCitPath(fmt::sprintf(L"logs/CitizenFX_log_%s.log", dateStamp));

				FILE* logFile = _wfopen(fp.c_str(), L"ab");

				if (logFile)
				{
					// null-terminate the string
					lineBuffer[lineIndex] = '\0';

					fmt::fprintf(logFile, "[%10lld] [%14s] %20s/ %s\r\n", GetTickCount64() - initTickCount->tickCount, processName, std::get<0>(pair), lineBuffer.data());
					fclose(logFile);
				}

				// clear the line
				lineIndex = 0;

				// skip this char
				continue;
			}

			// append the character
			lineBuffer[lineIndex] = *p;
			++lineIndex;

			// overflow? if so, resize
			if (lineIndex >= (lineBuffer.size() - 1))
			{
				lineBuffer.resize(lineBuffer.size() * 2);
			}
		}

		ReleaseSRWLockExclusive(&logMutex);
	}
}

static INIT_ONCE g_initLogFlag = INIT_ONCE_STATIC_INIT;
static CONDITION_VARIABLE g_logCondVar = CONDITION_VARIABLE_INIT;
static SRWLOCK g_logMutex = SRWLOCK_INIT;

static moodycamel::ConcurrentQueue<std::tuple<std::string, std::string>> g_logPrintQueue;

struct LoggerInit
{
	LoggerInit()
	{
		CreateThread(
		NULL, 0, [](LPVOID) -> DWORD
		{
			SetThreadName(-1, "[Cfx] File Log Thread");

			while (true)
			{
				AcquireSRWLockExclusive(&g_logMutex);
				SleepConditionVariableSRW(&g_logCondVar, &g_logMutex, INFINITE, 0);
				ReleaseSRWLockExclusive(&g_logMutex);

				std::tuple<std::string, std::string> str;

				while (g_logPrintQueue.try_dequeue(str))
				{
					PerformFileLog(str);
				}
			}

			return 0;
		},
		NULL, 0, NULL);
	}
};

static LoggerInit logger;

extern "C" DLL_EXPORT void AsyncTrace(const char* string)
{
	std::string threadName = GetThreadName();

	g_logPrintQueue.enqueue({ threadName, string });
	WakeAllConditionVariable(&g_logCondVar);
}

void InitLogging()
{
	SetThreadName(-1, "MainThrd");

	if (OpenMutex(SYNCHRONIZE, FALSE, L"CitizenFX_LogMutex_Mod") == nullptr)
	{
		// create the mutex
		CreateMutex(nullptr, TRUE, L"CitizenFX_LogMutex_Mod");

		CreateDirectory(MakeRelativeCitPath(L"logs/").c_str(), NULL);

		SYSTEMTIME curTime;
		GetSystemTime(&curTime);

		for (auto& f : std::filesystem::directory_iterator{ MakeRelativeCitPath(L"logs/") })
		{
			int year, month, day, hour, minute, second;
			int fields = swscanf(f.path().filename().c_str(), L"CitizenFX_log_%04d-%02d-%02dT%02d%02d%02d.log", &year, &month, &day, &hour, &minute, &second);

			if (fields == 6)
			{
				SYSTEMTIME fakeTime = { 0 };
				fakeTime.wSecond = second;
				fakeTime.wMinute = minute;
				fakeTime.wHour = hour;
				fakeTime.wDay = day;
				fakeTime.wMonth = month;
				fakeTime.wYear = year;

				FILETIME curStamp, fakeStamp;
				SystemTimeToFileTime(&curTime, &curStamp);
				SystemTimeToFileTime(&fakeTime, &fakeStamp);

				uint64_t curT = ((ULARGE_INTEGER*)&curStamp)->QuadPart;
				uint64_t fileT = ((ULARGE_INTEGER*)&fakeStamp)->QuadPart;

				if ((curT - fileT) > (7 * 24 * 60 * 60 * uint64_t(10000000)))
				{
					std::error_code ec;
					std::filesystem::remove(f.path(), ec);
				}
			}
		}

		AsyncTrace(fmt::format("--- BEGIN LOGGING AT {:%c} ---\n", fmt::gmtime(time(NULL))).c_str());
	}
}
