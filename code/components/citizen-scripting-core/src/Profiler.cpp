#include "StdInc.h"
#include <mutex>

#include <CoreConsole.h>
#include <Profiler.h>
#include <ResourceEventComponent.h>
#include <ResourceScriptingComponent.h>
#include <ResourceManager.h>
#include <ScriptEngine.h>
#include <VFSManager.h>

#include <msgpack.hpp>
#include <json.hpp>

#include <botan/base64.h>
#include <toojpeg.h>

using json = nlohmann::json;

MSGPACK_ADD_ENUM(fx::ProfilerEventType);

/// <summary>
/// Ensure m_events is valid for the duration of a ConvertToStorage operation.
/// </summary>
static std::recursive_mutex m_eventsMutex;

/// <summary>
/// Execute a profiler command on a separate thread.
///
/// Decorator designed to play nice with detail::make_function.
/// </summary>
template<typename... Args, typename Fn>
static auto ExecuteOffThread(Fn&& fn, bool requiresFxComponent = true)
{
	// If another console command is executed prior to the thread acquiring the
	// lock, e.g., 'profiler view ; profiler record start' then the thread will
	// likely return without processing. As this is a developer tool, corner
	// cutting should be acceptable.
	return [fn, requiresFxComponent](Args... args)
	{
		std::thread([=]()
		{
			if (requiresFxComponent)
			{
				std::lock_guard<std::recursive_mutex> guard(m_eventsMutex);
				fn(std::move(args)...);
			}
			else
			{
				fn(std::move(args)...);
			}
		}).detach();
	};
}

/// <summary>
/// Execute a profiler command on the console thread.
/// </summary>
template<typename... Args, typename Fn>
static auto ExecuteOnThread(Fn&& fn)
{
	return [fn](Args... args)
	{
		std::lock_guard<std::recursive_mutex> guard(m_eventsMutex);
		fn(std::move(args)...);
	};
}

struct ProfilerRecordingEvent {
	int who;
	int what;
	uint64_t when;
	std::string where;
	std::string why;
	fx::ProfilerEvent::memory_t much;

	MSGPACK_DEFINE_ARRAY(who, what, when, where, why, much)
};

struct ProfilerRecordingThread {
	int thread_id;
	std::string name;

	MSGPACK_DEFINE_MAP(thread_id, name)
};

struct ProfilerRecording {
	std::vector<uint64_t> ticks;
	std::vector<ProfilerRecordingEvent> events;
	std::vector<ProfilerRecordingThread> threads;

	MSGPACK_DEFINE_MAP(ticks, events, threads)
};

auto ConvertToStorage(fwRefContainer<fx::ProfilerComponent>& r_profiler) -> ProfilerRecording
{
	fx::ProfilerComponent *profiler = r_profiler.GetRef();
	const tbb::concurrent_vector<fx::ProfilerEvent>& evs = profiler->Get();

	std::vector<uint64_t> ticks;
	std::vector<ProfilerRecordingEvent> events;
	std::vector<ProfilerRecordingThread> threads;
	events.reserve(evs.size());
	auto start_of_tick = true;
	for (auto i = 0; i < evs.size(); i++)
	{
		const fx::ProfilerEvent& ev = evs[i];
		events.push_back({ ev.who, (int)ev.what, (uint64_t)ev.when.count(), ev.where, ev.why, ev.much });


		if (start_of_tick)
		{
			ticks.push_back(i);
			start_of_tick = false;
		}

		if (ev.what == fx::ProfilerEventType::BEGIN_TICK)
		{
			start_of_tick = true;
		}
	}

	for (auto it = profiler->Threads().cbegin(); it != profiler->Threads().cend(); ++it)
	{
		if (std::get<1>(it->second))
		{
			threads.push_back({ std::get<0>(it->second), it->first });
		}
	}

	return {
		ticks,
		events,
		threads
	};
}

auto ConvertToJSON(const ProfilerRecording& recording) -> json
{
	auto obj = json::object();
	auto traceEvents = json::array();

	auto UpdateCounters = [&traceEvents](const ProfilerRecordingEvent &event) -> void {
		if (event.much != 0) {
			traceEvents.push_back(json::object({
				{ "cat", "disabled-by-default-devtools.timeline" },
				{ "name", "UpdateCounters" },
				{ "ph", "I" },
				{ "s", "g" },
				{ "ts", event.when },
				{ "pid", TRACE_PROCESS_MAIN },
				{ "tid", event.who },
				{ "args",
					{{
						"data",
						{
							{ "jsHeapSizeUsed", event.much },
						}
					}}
				}
			}));
		}
	};

	traceEvents.push_back(json::object({
		{ "cat", "__metadata" },
		{ "name", "process_name" },
		{ "ph", "M" },
		{ "ts", 0 },
		{ "pid", TRACE_PROCESS_MAIN },
		{ "tid", TRACE_THREAD_MAIN },
		{ "args", json::object({
			{ "name", "Browser" }
		}) }
	}));

	traceEvents.push_back(json::object({
		{ "cat", "__metadata" },
		{ "name", "thread_name" },
		{ "ph", "M" },
		{ "ts", 0 },
		{ "pid", TRACE_PROCESS_MAIN },
		{ "tid", TRACE_THREAD_MAIN },
		{ "args", json::object({
			{ "name", "CrBrowserMain" }
		}) }
	}));

	traceEvents.push_back(json::object({
		{ "cat", "__metadata" },
		{ "name", "thread_name" },
		{ "ph", "M" },
		{ "ts", 0 },
		{ "pid", TRACE_PROCESS_MAIN },
		{ "tid", TRACE_THREAD_BROWSER },
		{ "args", json::object({
			{ "name", "CrRendererMain" }
		}) }
	}));

	for (auto it = recording.threads.cbegin(); it != recording.threads.cend(); ++it)
	{
		traceEvents.push_back(json::object({
			{ "cat", "__metadata" },
			{ "name", "thread_name" },
			{ "ph", "M" },
			{ "ts", 0 },
			{ "pid", TRACE_PROCESS_MAIN },
			{ "tid", it->thread_id },
			{ "args", json::object({
				{ "name", it->name }
			}) }
		}));
	}

	// needed to make the devtools enable frames
	traceEvents.push_back(json::object({
		{ "cat", "disabled-by-default-devtools.timeline" },
		{ "name", "TracingStartedInBrowser" },
		{ "ph", "I" },
		{ "ts", 0 },
		{ "pid", TRACE_PROCESS_MAIN },
		{ "tid", TRACE_THREAD_MAIN },
		{ "args", json::object({
			{ "data", json::object({
				{ "frameTreeNodeId", 1 },
				{ "persistentIds", true },
				{ "frames", json::array({
					json::object({
						{ "frame", "FADE" },
						{ "url", "https://cfx.re/" },
						{ "name", "CitizenFX" },
						{ "processId", 1 },
					})
				}) },
			}) }
		}) }
	}));

	size_t frameNum = 0;

	std::map<fx::ProfilerEvent::thread_t, std::stack<ProfilerRecordingEvent>> eventMap;

	for (const auto& event : recording.events)
	{
		std::stack<ProfilerRecordingEvent>& eventStack = eventMap[event.who];
		switch ((fx::ProfilerEventType)event.what)
		{
		case fx::ProfilerEventType::BEGIN_TICK:
			traceEvents.push_back(json::object({
				{ "cat", "disabled-by-default-devtools.timeline.frame" },
				{ "name", "BeginFrame" },
				{ "s", "t" },
				{ "ph", "I" },
				{ "ts", event.when },
				{ "pid", TRACE_PROCESS_MAIN },
				{ "tid", event.who },
				{ "args", json::object({
					{ "layerTreeId", nullptr }
				}) }
			}));
			UpdateCounters(event);

			break;
		case fx::ProfilerEventType::END_TICK:
			traceEvents.push_back(json::object({
				{ "cat", "disabled-by-default-devtools.timeline.frame" },
				{ "name", "ActivateLayerTree" },
				{ "s", "t" },
				{ "ph", "I" },
				{ "ts", event.when },
				{ "pid", TRACE_PROCESS_MAIN },
				{ "tid", event.who },
				{ "args", json::object({
					{ "layerTreeId", nullptr },
					{ "frameId", frameNum },
				}) }
				}));

			traceEvents.push_back(json::object({
				{ "cat", "disabled-by-default-devtools.timeline.frame" },
				{ "name", "DrawFrame" },
				{ "s", "t" },
				{ "ph", "I" },
				{ "ts", event.when },
				{ "pid", TRACE_PROCESS_MAIN },
				{ "tid", event.who },
				{ "args", json::object({
					{ "layerTreeId", nullptr }
				}) }
			}));

			traceEvents.push_back(json::object({
				{ "cat", "disabled-by-default-devtools.screenshot" },
				{ "name", "Screenshot" },
				{ "id", frameNum },
				{ "ph", "O" },
				{ "ts", event.when },
				{ "pid", TRACE_PROCESS_MAIN },
				{ "tid", event.who },
				{ "args", json::object({
					// TODO: screenshot if client?
					{ "snapshot", 
						(event.why.empty()) 
							? "/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAMCAgICAgMCAgIDAwMDBAYEBAQEBAgGBgUGCQgKCgkICQkKDA8MCgsOCwkJDRENDg8QEBEQCgwSExIQEw8QEBD/2wBDAQMDAwQDBAgEBAgQCwkLEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBD/wAARCAABAAEDASIAAhEBAxEB/8QAFQABAQAAAAAAAAAAAAAAAAAAAAj/xAAUEAEAAAAAAAAAAAAAAAAAAAAA/8QAFQEBAQAAAAAAAAAAAAAAAAAABwn/xAAUEQEAAAAAAAAAAAAAAAAAAAAA/9oADAMBAAIRAxEAPwCdAAYqm//Z"
							: event.why }
				}) }
			}));

			UpdateCounters(event);
			frameNum++;
			break;
		case fx::ProfilerEventType::ENTER_RESOURCE:
		case fx::ProfilerEventType::ENTER_SCOPE:
		{
			traceEvents.push_back(json::object({
				{ "cat", "blink.user_timing" },
				{ "name", (event.what == (int)fx::ProfilerEventType::ENTER_RESOURCE)
					? fmt::sprintf("%s (%s)", event.why, event.where)
					: event.where },
				{ "ph", "B" },
				{ "ts", event.when },
				{ "pid", TRACE_PROCESS_MAIN },
				{ "tid", event.who },
			}));

			eventStack.push(event);
			UpdateCounters(event);

			break;
		}
		case fx::ProfilerEventType::EXIT_RESOURCE:
		case fx::ProfilerEventType::EXIT_SCOPE:
		{
			ProfilerRecordingEvent exitEvent;

			if (eventStack.size() != 0)
			{
				auto thisExit = eventStack.top();
				eventStack.pop();

				traceEvents.push_back(json::object({
					{ "cat", "blink.user_timing" },
					{ "name", (thisExit.what == (int)fx::ProfilerEventType::ENTER_RESOURCE)
						? fmt::sprintf("%s (%s)", thisExit.why, thisExit.where)
						: thisExit.where },
					{ "ph", "E" },
					{ "ts", event.when },
					{ "pid", TRACE_PROCESS_MAIN },
					{ "tid", event.who },
					}));

				UpdateCounters(event);
			}
			break;
		}
		}

		frameNum++;
	}

	obj["traceEvents"] = traceEvents;

	return obj;
}

void ViewProfile(const json& jsonData)
{
	auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
	profiler->OnRequestView(jsonData);
}

std::string LookupName(fx::ProfilerEventType ty) {
	using EvType = fx::ProfilerEventType;
	switch (ty)
	{
	case EvType::BEGIN_TICK:     return ">FRM";
	case EvType::END_TICK:       return "<FRM";
	case EvType::ENTER_RESOURCE: return ">RES";
	case EvType::EXIT_RESOURCE:  return "<RES";
	case EvType::ENTER_SCOPE:    return ">SCO";
	case EvType::EXIT_SCOPE:     return "<SCO";
	default:					 return "";
	}
}

template<typename TContainer>
void PrintEvents(const TContainer& evs) {
	using EvType = fx::ProfilerEventType;

	auto indent = 0;

	for (const fx::ProfilerEvent& ev : evs) {
		if (ev.what == EvType::EXIT_RESOURCE || ev.what == EvType::EXIT_SCOPE)
		{
			indent -= 2;
		}

		auto str = fmt::sprintf("%s%s @ %d", std::string(indent, ' '), LookupName(ev.what), ev.when.count());

		switch (ev.what)
		{
		case EvType::ENTER_RESOURCE:
			console::Printf("cmd", "%s res: %s, why: %s\n", str, ev.where, ev.why);
			break;
		case EvType::ENTER_SCOPE:
			console::Printf("cmd", "%s scope: %s\n", str,  ev.where);
			break;
		default:
			console::Printf("cmd", "%s \n", str);
		}

		if (ev.what == EvType::ENTER_RESOURCE || ev.what == EvType::ENTER_SCOPE)
		{
			indent += 2;
		}
	}
}


// profiler help
// profiler status

// profiler record start   | Record forever
// profiler record <ticks> | Record <ticks>
// profiler record stop    | Stop recording
// profiler save <file>    |

// profiler view              | View the current profile
// profiler view <filename>   | View the specified file

namespace profilerCommand {
	const static std::unordered_map<std::string, std::string> commandUsage = {
		{"help",   ""},
		{"status", ""},
		{"record", " start | <frames> | stop"},
		{"resource", " <resource, frames> | stop"},
		{"save",   " <filename>"},
		{"load",   " <filename>"},
		{"view",   " [filename]" }
	};

	static fwRefContainer<console::Context> profilerCtx;
	auto Setup()
	{
		console::CreateContext(nullptr, &profilerCtx);

		static ConsoleCommand helpCmd(profilerCtx.GetRef(), "help", []() {
			profilerCtx->GetCommandManager()->ForAllCommands([](std::string cmd)
			{
				auto cmddesc = commandUsage.find(cmd);
				if (cmddesc != commandUsage.end()) {
					console::Printf("cmd", "profiler %s%s\n", cmd, cmddesc->second);
				}
			});
		});

		static ConsoleCommand statusCmd(profilerCtx.GetRef(), "status", ExecuteOnThread([]()
		{
			// Recording: <Yes/No> (<frames>)
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			if (profiler->IsRecording())
			{
				if (profiler->GetFrames() > 0)
				{
					console::Printf("cmd", "Recording: Yes (%d)\n", profiler->GetFrames());
				}
				else
				{
					console::Printf("cmd", "Recording: Yes\n");
				}
			}
			else
			{
				console::Printf("cmd", "Recording: No\n");
			}
			// Buffer: <y> events over <x> frames
			auto eventCount = 0;
			auto frameCount = 0;
			for (const auto& ev : profiler->Get())
			{
				if (ev.what == fx::ProfilerEventType::BEGIN_TICK)
				{
					frameCount++;
				}
				else if (ev.what != fx::ProfilerEventType::END_TICK)
				{
					eventCount++;
				}
			}
			console::Printf("cmd", "Buffer: %d events over %d frames\n", eventCount, frameCount);
		}));

		static ConsoleCommand recordCmd(profilerCtx.GetRef(), "record", ExecuteOnThread<std::string>([](std::string arg)
		{
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			if (arg == "stop")
			{
				if (profiler->IsScriptRecording())
				{
					profiler->ScriptStopRecording();
					console::Printf("cmd", "Stopping the recording\n");
				}
				else if (profiler->IsRecording())
				{
					profiler->StopRecording();
					console::Printf("cmd", "Stopped the recording\n");
				}
			}
			else if (!profiler->IsRecording())
			{
				try
				{
					profiler->StartRecording(arg == "start" ? -1 : std::stoi(arg));
					console::Printf("cmd", "Started recording\n");
				}
				catch (const std::invalid_argument& )
				{
					console::Printf("cmd", "Invalid argument to `profiler record` expected 'start', 'stop' or an integer\n");
				}
			}
			else
			{
				console::Printf("cmd", "A recording is already taking place\n");
			}
		}));

		static ConsoleCommand resourceStopCmd(profilerCtx.GetRef(), "resource", ExecuteOnThread<std::string>([](std::string arg)
		{
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			if (arg == "stop")
			{
				if (profiler->IsScriptRecording())
				{
					profiler->ScriptStopRecording();
					console::Printf("cmd", "Stopping the recording\n");
				}
				else if (profiler->IsRecording())
				{
					profiler->StopRecording();
					console::Printf("cmd", "Stopped the recording\n");
				}
				else
				{
					console::Printf("cmd", "No active recording\n");
				}
			}
		}));

		static ConsoleCommand resourceCmd(profilerCtx.GetRef(), "resource", ExecuteOnThread<std::string, std::string>([](std::string resource, std::string arg)
		{
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			if (profiler->IsRecording())
			{
				console::Printf("cmd", "A recording is already taking place\n");
				return;
			}

			int frames = -1;
			if (arg.size() > 0) {
				try {
					frames = std::stoi(arg); // @TODO Improve. Lazy.
				}
				catch (const std::invalid_argument& )
				{
					console::Printf("cmd", "Expected frame integer value\n");
					return;
				}
			}

			profiler->StartRecording(frames, resource);
			console::Printf("cmd", "Started recording\n");
		}));

		static ConsoleCommand saveCmd(profilerCtx.GetRef(), "save", ExecuteOffThread<std::string>([](std::string path)
		{
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();

			std::string outFn = path;

#ifndef IS_FXSERVER
			outFn = "citizen:/" + outFn;
#endif

			auto vfsDevice = vfs::GetDevice(outFn);
			auto handle = vfsDevice->Create(outFn);

			vfs::Stream writeStream(vfsDevice, handle);

			struct WriteWrapper
			{
				WriteWrapper(vfs::Stream& stream)
					: stream(stream)
				{
				}

				void write(const char* data, size_t size)
				{
					stream.Write(data, size);
				}

				vfs::Stream& stream;
			} writeWrapper(writeStream);

			console::Printf("cmd", "Saving the recording to: %s.\n", path);
			msgpack::pack(writeWrapper, ConvertToStorage(profiler));
			console::Printf("cmd", "Save complete\n");
		}));

		static ConsoleCommand dumpCmd(profilerCtx.GetRef(), "dump", ExecuteOffThread([]() {
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			if (profiler->IsRecording())
			{
				console::Printf("cmd", "Cannot dump: profiler is active.\n");
				return;
			}

			PrintEvents(profiler->Get());
		}));

		static ConsoleCommand viewCmd0(profilerCtx.GetRef(), "view", ExecuteOffThread([]() {
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			if (profiler->IsRecording())
			{
				console::Printf("cmd", "Cannot view: profiler is active.\n");
				return;
			}

			console::Printf("cmd", "Building profile results\n");
			auto jsonData = ConvertToJSON(ConvertToStorage(profiler));

			ViewProfile(jsonData);
		}));

		static ConsoleCommand viewCmd1(profilerCtx.GetRef(), "view", ExecuteOffThread<std::string>([](std::string path) {
			std::string inFn = path;

#ifndef IS_FXSERVER
			inFn = "citizen:/" + inFn;
#endif

			auto stream = vfs::OpenRead(inFn);

			if (!stream.GetRef())
			{
				return;
			}

			auto data = stream->ReadToEnd();
			auto unpacked = msgpack::unpack(reinterpret_cast<char*>(data.data()), data.size());
			auto recording = unpacked.get().as<ProfilerRecording>();
			auto jsonData = ConvertToJSON(recording);

			ViewProfile(jsonData);
		}, false));
	}
	

	auto Execute(const ProgramArguments& args) {
		if (!profilerCtx.GetRef()) { Setup(); }

		if (args.Count() == 0)
		{
			profilerCtx->ExecuteSingleCommandDirect(ProgramArguments{ "help" });
		}
		else if (commandUsage.find(args[0]) == commandUsage.end())
		{
			console::Printf("cmd", "`profiler %s` not found. See `profiler help` for available commands\n", args[0]);
		}
		else
		{
			profilerCtx->ExecuteSingleCommandDirect(args);
		}
	};

	static InitFunction initFunction([]
	{
		static ConsoleCommand profilerCmd0("profiler", []()
		{
			Execute(ProgramArguments{});
		});
		static ConsoleCommand profilerCmd1("profiler", [](std::string subcmd)
		{
			Execute(ProgramArguments{ subcmd });
		});
		static ConsoleCommand profilerCmd2("profiler", [](std::string subcmd, std::string arg)
		{
			Execute(ProgramArguments{ subcmd, arg });
		});
		static ConsoleCommand profilerCmd3("profiler", [](std::string subcmd, std::string arg, std::string arg2)
		{
			Execute(ProgramArguments{ subcmd, arg, arg2 });
		});
	});
}

namespace fx {
	DLL_EXPORT bool g_recordProfilerTime;

#ifndef IS_FXSERVER
	void ProfilerComponent::SubmitScreenshot(const void* imageRgb, size_t width, size_t height)
	{
		if (!IsRecording())
		{
			return;
		}

		static thread_local uint8_t bbuf[256 * 1024];
		static thread_local size_t bbufIdx;

		bbufIdx = 0;

		if (TooJpeg::writeJpeg([](uint8_t c)
		{
			bbuf[(bbufIdx++) % sizeof(bbuf)] = c;
		}, imageRgb, width, height, true, 70, true))
		{
			m_screenshot = Botan::base64_encode(bbuf, bbufIdx % sizeof(bbuf));
		}
	}
#endif

	void ProfilerComponent::EnterResource(const std::string& resource, const std::string& cause)
	{
		PushEvent(TRACE_THREAD_BROWSER, ProfilerEventType::ENTER_RESOURCE, resource, cause);
	}
	void ProfilerComponent::ExitResource()
	{
		PushEvent(TRACE_THREAD_BROWSER, ProfilerEventType::EXIT_RESOURCE);
	}
	void ProfilerComponent::EnterScope(const std::string& scope, ProfilerEvent::memory_t memoryUsage)
	{
		PushEvent(TRACE_THREAD_BROWSER, ProfilerEventType::ENTER_SCOPE, scope, std::string{}, memoryUsage);
	}
	void ProfilerComponent::ExitScope(ProfilerEvent::memory_t memoryUsage)
	{
		PushEvent(TRACE_THREAD_BROWSER, ProfilerEventType::EXIT_SCOPE, memoryUsage);
	}
	void ProfilerComponent::BeginTick(ProfilerEvent::memory_t memoryUsage)
	{
		PushEvent(TRACE_THREAD_MAIN, fx::ProfilerEventType::BEGIN_TICK, memoryUsage);
		EnterScope("Resource Manager Tick", memoryUsage);

		if (--m_frames == 0) {
			ProfilerComponent::StopRecording();
			console::Printf("cmd", "Stopped the recording\n");
		}
	}
	void ProfilerComponent::EndTick(ProfilerEvent::memory_t memoryUsage)
	{
		ExitScope(memoryUsage);
#ifndef IS_FXSERVER
		PushEvent(TRACE_THREAD_MAIN, fx::ProfilerEventType::END_TICK, "", m_screenshot);
#else
		PushEvent(TRACE_THREAD_MAIN, fx::ProfilerEventType::END_TICK);
#endif
	}
	
	void ProfilerComponent::StartRecording(const int frames, const std::string& resource)
	{
		// Handle StartRecording being invoked outside of ConsoleCommand
		std::lock_guard<std::recursive_mutex> guard(m_eventsMutex);

		m_offset = fx::usec();
		m_frames = frames;
		m_recording = true;
		g_recordProfilerTime = true;
		m_events = {};
		m_resources.clear();

		m_script = resource != "";
		m_resource_name = resource;
	}

	void ProfilerComponent::StopRecording()
	{
		std::lock_guard<std::recursive_mutex> guard(m_eventsMutex);

		ShutdownScriptConnection();
		m_recording = false;
		m_script = false;
		m_shutdown_next = false;
		m_resource_name = "";
		g_recordProfilerTime = false;
	}

	auto ProfilerComponent::Get() -> const tbb::concurrent_vector<ProfilerEvent>&
	{
		return m_events;
	}
	bool ProfilerComponent::IsRecording()
	{
		return m_recording;
	}
	int ProfilerComponent::GetFrames()
	{
		return m_frames;
	}

	std::string ProfilerComponent::GetDevToolsURL()
	{
		return "https://chrome-devtools-frontend.appspot.com/serve_rev/@901bcc219d9204748f9c256ceca0f2cd68061006/inspector.html";
	}

	bool ProfilerComponent::IsScriptRecording()
	{
		return IsRecording() && m_script;
	}

	void ProfilerComponent::ScriptStopRecording()
	{
		m_shutdown_next = true;
	}

	bool ProfilerComponent::IsScriptStopping()
	{
		return IsScriptRecording() && m_shutdown_next;
	}

	const tbb::concurrent_unordered_map<std::string, std::tuple<fx::ProfilerEvent::thread_t, bool>>& ProfilerComponent::Threads()
	{
		return m_resources;
	}

	void ProfilerComponent::SetupScriptConnection(fx::Resource* resource)
	{
		if (m_script && m_resources.find(resource->GetIdentifier()) == m_resources.end())
		{
			// Ensure the resource has an associated identifier
			const fx::ProfilerEvent::thread_t tid = (fx::ProfilerEvent::thread_t)m_resources.size() + TRACE_THREAD_BROWSER + 1;
			const bool setupProfiler = m_resource_name == "*" || resource->GetName() == m_resource_name;
			m_resources.emplace(resource->GetIdentifier(), std::make_tuple(tid, setupProfiler));

			// Extended profiler API for a single resource (by name) or all: *
			if (setupProfiler)
			{
				fx::ProfilerComponent* p_this = this;

				// For each runtime: if it implements IScriptProfiler setup the
				// profiler prior to continuing the resource tick.
				auto scripting = resource->GetComponent<fx::ResourceScriptingComponent>();
				scripting->ForAllRuntimes([p_this, tid](fx::OMPtr<IScriptRuntime> scRt)
				{
					fx::OMPtr<IScriptProfiler> pRt;
					if (FX_SUCCEEDED(scRt.As<IScriptProfiler>(&pRt)))
					{
						pRt->SetupFxProfiler(static_cast<void*>(p_this), tid);
					}
				});
			}
		}
	}

	void ProfilerComponent::ShutdownScriptConnection()
	{
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		for (auto it = m_resources.begin(); it != m_resources.end(); ++it)
		{
			fwRefContainer<fx::Resource> resource = resourceManager->GetResource(it->first);
			if (!std::get<1>(it->second) || !resource.GetRef()) // Does not have extended profiler API
			{
				continue;
			}

			// For each runtime: if it implements IScriptProfiler, shut the profiler down.
			auto scripting = resource->GetComponent<fx::ResourceScriptingComponent>();
			scripting->ForAllRuntimes([](fx::OMPtr<IScriptRuntime> scRt)
			{
				fx::OMPtr<IScriptProfiler> pRt;
				if (FX_SUCCEEDED(scRt.As<IScriptProfiler>(&pRt)))
				{
					pRt->ShutdownFxProfiler();
				}
			});
		}
	}
}


static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("PROFILER_ENTER_SCOPE", [](fx::ScriptContext& ctx)
	{
		static auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		profiler->EnterScope(ctx.GetArgument<const char*>(0));
	});

	fx::ScriptEngine::RegisterNativeHandler("PROFILER_EXIT_SCOPE", [](fx::ScriptContext& ctx)
	{
		static auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		profiler->ExitScope();
	});

	fx::ScriptEngine::RegisterNativeHandler("PROFILER_IS_RECORDING", [](fx::ScriptContext& ctx)
	{
		static auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		ctx.SetResult<int>(profiler->IsRecording());
	});

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* res)
	{
		auto resname = res->GetName();
		auto profiler = res->GetManager()->GetComponent<fx::ProfilerComponent>();
		
		res->OnEnter.Connect([res, profiler, resname]()
		{
			profiler->EnterResource(resname, "tick");
			profiler->SetupScriptConnection(res);
		}, INT32_MIN);

		res->OnLeave.Connect([profiler, resname]()
		{
			profiler->ExitResource();
		}, INT32_MAX);

		res->OnStart.Connect([=]()
		{
			auto event = res->GetComponent<fx::ResourceEventComponent>();
			event->OnTriggerEvent.Connect([=](const std::string& evName, const std::string& evPayload, const std::string& evSrc, bool* evCanceled)
			{
				profiler->EnterResource(resname, fmt::sprintf("event:%s", evName));
				profiler->SetupScriptConnection(res);
			}, -10000001);
			event->OnTriggerEvent.Connect([=](const std::string& evName, const std::string& evPayload, const std::string& evSrc, bool* evCanceled)
			{
				profiler->ExitResource();
			}, 10000001);
		});
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* resman)
	{
		auto profiler = new fx::ProfilerComponent();
		resman->SetComponent<fx::ProfilerComponent>(profiler);
		resman->OnTick.Connect([profiler]() { profiler->BeginTick(); }, INT32_MIN);
		resman->OnTick.Connect([profiler]()
		{
			profiler->EndTick();
			if (profiler->IsScriptStopping())
			{
				profiler->StopRecording();
				console::Printf("cmd", "Stopped the recording\n");
			}
		},
		INT32_MAX);
	});
});
