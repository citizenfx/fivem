//
// The server watchdog will perform a live dump when a core component does not
// check in after a very long period (~60-120s).
//
// It'll also upload a live dump to the live dump ingress service.
//

#include <StdInc.h>

#include <CoreConsole.h>
#include <DebugAlias.h>

#include <Resource.h>
#include <ResourceEventComponent.h>

#ifdef _WIN32
#include <dbghelp.h>
#include <common/windows/http_upload.h>

#include <ProcessSnapshot.h>
#endif

#include <UvLoopManager.h>
#include <ServerInstanceBase.h>

#include <StructuredTrace.h>

#include <GameServer.h>

namespace watchdog
{
#ifdef _WIN32
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "winhttp.lib")

BOOL CALLBACK MyMiniDumpWriteDumpCallback(
	__in     PVOID CallbackParam,
	__in     const PMINIDUMP_CALLBACK_INPUT CallbackInput,
	__inout  PMINIDUMP_CALLBACK_OUTPUT CallbackOutput
)
{
	switch (CallbackInput->CallbackType)
	{
	case IsProcessSnapshotCallback:
		CallbackOutput->Status = S_FALSE;
		break;
	}
	return TRUE;
}

void PlatformBark(const std::string& loopName)
{
	// TODO: this is a bit broken and doesn't support SetThreadDescription
	// do we really need snapshots for a process that'll be dead soon?
	auto _PssCaptureSnapshot = (decltype(&PssCaptureSnapshot))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "PssCaptureSnapshot");
	auto _PssFreeSnapshot = (decltype(&PssFreeSnapshot))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "PssFreeSnapshot");

	if (!_PssCaptureSnapshot || !_PssFreeSnapshot)
	{
		return;
	}

	char loopNameStr[64];
	strncpy(loopNameStr, loopName.c_str(), std::size(loopNameStr));

	debug::Alias(loopNameStr);

	PSS_CAPTURE_FLAGS CaptureFlags = PSS_CAPTURE_VA_CLONE
		| PSS_CAPTURE_HANDLES
		| PSS_CAPTURE_HANDLE_NAME_INFORMATION
		| PSS_CAPTURE_HANDLE_BASIC_INFORMATION
		| PSS_CAPTURE_HANDLE_TYPE_SPECIFIC_INFORMATION
		| PSS_CAPTURE_HANDLE_TRACE
		| PSS_CAPTURE_THREADS
		| PSS_CAPTURE_THREAD_CONTEXT
		| PSS_CAPTURE_THREAD_CONTEXT_EXTENDED
		| PSS_CREATE_BREAKAWAY
		| PSS_CREATE_BREAKAWAY_OPTIONAL
		| PSS_CREATE_USE_VM_ALLOCATIONS
		| PSS_CREATE_RELEASE_SECTION;

	HPSS SnapshotHandle;
	DWORD dwResultCode = _PssCaptureSnapshot(GetCurrentProcess(),
		CaptureFlags,
		CONTEXT_ALL,
		&SnapshotHandle);

	if (dwResultCode != ERROR_SUCCESS)
	{
		return;
	}

	MINIDUMP_CALLBACK_INFORMATION CallbackInfo;
	ZeroMemory(&CallbackInfo, sizeof(MINIDUMP_CALLBACK_INFORMATION));
	CallbackInfo.CallbackRoutine = MyMiniDumpWriteDumpCallback;
	CallbackInfo.CallbackParam = NULL;

	auto dumpPath = MakeRelativeCitPath(fmt::sprintf(L"crashes\\livedump-%d.dmp", time(NULL)));

	HANDLE hFile = CreateFile(
		dumpPath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		_PssFreeSnapshot(GetCurrentProcess(), SnapshotHandle);
		return;
	}

	BOOL success = MiniDumpWriteDump((HANDLE)SnapshotHandle,
		GetCurrentProcessId(),
		hFile,
		(MINIDUMP_TYPE)(MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo),
		NULL,
		NULL,
		&CallbackInfo);

	_PssFreeSnapshot(GetCurrentProcess(), SnapshotHandle);

	CloseHandle(hFile);

	if (success)
	{
		std::map<std::wstring, std::wstring> parameters;

		std::wstring responseBody;
		int responseCode;

		std::map<std::wstring, std::wstring> files;
		files[L"upload_file_minidump"] = dumpPath;

		if (google_breakpad::HTTPUpload::SendMultipartPostRequest(L"https://sentry.fivem.net/api/4/minidump/?sentry_key=8bc0468f1732468ab52d15b77c5fb2fb", parameters, files, nullptr, &responseBody, &responseCode))
		{
			if (responseCode >= 200 && responseCode < 399)
			{
				console::Printf("server", "Uploaded a live hang dump to the CitizenFX crash reporting service. The report ID is %s.\n", ToNarrow(responseBody));
			}
		}		
	}
}

bool IsDesktopPlatform()
{
	return !IsWindowsServer();
}

// noinline so we can correctly attribute this in stack traces
__declspec(noinline) void PlatformAbort()
{
	*(volatile int*)0 = 0;
}
#else
void PlatformBark(const std::string& loopName)
{

}

bool IsDesktopPlatform()
{
	return false;
}

void PlatformAbort()
{
	// Mono on Linux breaks SIGSEGV sometimes so we'll just raise SIGABRT
	abort();
}
#endif

constexpr int NumThreads = 4;

using TThreadStack = std::deque<std::string>;
using TThreadMap = std::array<TThreadStack, NumThreads>;

static TThreadMap g_threadStacks;

static int GetThreadIdx(uint32_t th)
{
	switch (th)
	{
	case HashString("default"):
		return 0;
	case HashString("svMain"):
		return 1;
	case HashString("svNetwork"):
		return 2;
	case HashString("svSync"):
		return 3;
	}

	return -1;
}

static int GetThreadIdx()
{
	static auto uvmgr = Instance<net::UvLoopManager>::Get();

	auto curLoop = uvmgr->GetCurrent();

	if (!curLoop.GetRef())
	{
		return -1;
	}

	return GetThreadIdx(HashString(curLoop->GetLoopTag().c_str()));
}

static TThreadStack* GetThread()
{
	auto idx = GetThreadIdx();

	if (idx >= 0)
	{
		return &g_threadStacks[idx];
	}

	return nullptr;
}

struct WatchdogWarningComponent : public fwRefCountable, public fx::IAttached<fx::Resource>
{
	WatchdogWarningComponent()
		: initialized(false)
	{

	}

	virtual void AttachToObject(fx::Resource* object) override
	{
		object->OnStart.Connect([this, object]()
		{
			InitializeOnce(object);
		}, INT32_MAX);
	}

	void InitializeOnce(fx::Resource* resource)
	{
		if (initialized)
		{
			return;
		}

		auto resourceName = resource->GetName();

		auto ev = resource->GetComponent<fx::ResourceEventComponent>();
		ev->OnTriggerEvent.Connect([resourceName](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
		{
			auto th = GetThread();

			if (th)
			{
				th->push_front(fmt::sprintf("%s: event %s", resourceName, eventName));
			}
		}, -100);

		ev->OnTriggerEvent.Connect([](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
		{
			auto th = GetThread();

			if (th)
			{
				th->pop_front();
			}
		}, 100);

		resource->OnTick.Connect([resourceName]()
		{
			auto th = GetThread();

			if (th)
			{
				th->push_front(fmt::sprintf("%s: tick", resourceName));
			}
		}, INT32_MIN);

		resource->OnTick.Connect([]()
		{
			auto th = GetThread();

			if (th)
			{
				th->pop_front();
			}
		}, INT32_MAX);

		initialized = true;
	}

private:
	bool initialized;
};

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new WatchdogWarningComponent());
	});

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->OnInitialConfiguration.Connect([instance]()
		{
			using namespace std::chrono_literals;

			static tbb::concurrent_unordered_map<std::string, std::chrono::milliseconds> bites;
			static tbb::concurrent_unordered_map<std::string, bool> barks;

			auto bite = [](const char* name)
			{
				bites[name] = msec();
			};

			auto bark = [](const std::string& name)
			{
				if (barks[name])
				{
					return;
				}

				barks[name] = true;

				auto dur = std::chrono::duration_cast<std::chrono::seconds>(msec() - bites[name]);

				console::PrintError("server", "Loop %s seems hung! (last checkin %d seconds ago)\n",
					name,
					dur.count()
				);

				std::stringstream s;

				auto& threadStack = g_threadStacks[GetThreadIdx(HashString(name.c_str()))];

				for (auto& entry : threadStack)
				{
					s << entry << " <- ";
				}

				s << "root";

				console::PrintWarning("server", "%s watchdog stack: %s\n", name, s.str());

				StructuredTrace({ "type", "watchdog_bark" }, { "thread", name }, { "time", dur.count() }, { "stack", s.str() });

				PlatformBark(name);
			};

			static std::vector<std::shared_ptr<uvw::TimerHandle>> timers;

			for (auto loopName : { "svMain", "default", "svNetwork", "svSync" })
			{
				auto loop = Instance<net::UvLoopManager>::Get()->Get(loopName);

				if (loop.GetRef())
				{
					auto refLoop = loop->Get();

					// doing this is bollocks, it's on the wrong thread
					// at least it's at init-time so it'll *hopefully* not race
					// if it does, might need to add a GetLocked that'll not run if the loop is being polled?
					auto loopTimer = refLoop->resource<uvw::TimerHandle>();

					loopTimer->on<uvw::TimerEvent>([bite, loopName](const uvw::TimerEvent& ev, uvw::TimerHandle& handle)
					{
						bite(loopName);
					});

					loopTimer->start(0ms, 5000ms);

					timers.push_back(loopTimer);
				}
			}

			{
				auto dogLoop = Instance<net::UvLoopManager>::Get()->GetOrCreate("watchdog");

				auto refLoop = dogLoop->Get();

				// doing this is bollocks, it's on the wrong thread
				// at least it's at init-time so it'll *hopefully* not race
				// if it does, might need to add a GetLocked that'll not run if the loop is being polled?
				auto loopTimer = refLoop->resource<uvw::TimerHandle>();

				loopTimer->on<uvw::TimerEvent>([bark](const uvw::TimerEvent& ev, uvw::TimerHandle& handle)
				{
					for (const auto& pair : bites)
					{
						if ((msec() - pair.second) > 300s && !IsDesktopPlatform())
						{
							char loopNameStr[64];
							strncpy(loopNameStr, pair.first.c_str(), std::size(loopNameStr));

							debug::Alias(loopNameStr);

							StructuredTrace({ "type", "watchdog_bite" }, { "thread", loopNameStr });

							console::PrintError("server", "Loop %s is hung for over 5 minutes - terminating with a fatal exception.\n", pair.first);

							PlatformAbort();
						}

						if ((msec() - pair.second) > 90s)
						{
							bark(pair.first);
						}
					}
				});

				loopTimer->start(0ms, 1000ms);

				timers.push_back(loopTimer);
			}
		}, INT32_MAX);
	});
});
}

DECLARE_INSTANCE_TYPE(watchdog::WatchdogWarningComponent);
