#include "StdInc.h"
#include <CoreConsole.h>
#include <Profiler.h>
#include <ResourceEventComponent.h>
#include <ResourceManager.h>
#include <ScriptEngine.h>
#include <VFSManager.h>

#include <msgpack.hpp>
#include <json.hpp>

#include <botan/base64.h>
#include <toojpeg.h>

using json = nlohmann::json;

MSGPACK_ADD_ENUM(fx::ProfilerEventType);

struct ProfilerRecordingEvent {
	uint64_t when;
	int what;
	std::string where;
	std::string why;

	MSGPACK_DEFINE_ARRAY(when, what, where, why)
};

struct ProfilerRecording {
	std::vector<uint64_t> ticks;
	std::vector<ProfilerRecordingEvent> events;

	MSGPACK_DEFINE_MAP(ticks, events)
};

template<typename TContainer>
auto ConvertToStorage(const TContainer& evs) -> ProfilerRecording
{
	std::vector<uint64_t> ticks;
	std::vector<ProfilerRecordingEvent> events;
	events.reserve(evs.size());
	auto start_of_tick = true;
	for (auto i = 0; i < evs.size(); i++)
	{
		const fx::ProfilerEvent& ev = evs[i];
		events.push_back({ (uint64_t)ev.when.count(), (int)ev.what, ev.where, ev.why });


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

	return {
		ticks,
		events
	};
}

auto ConvertToJSON(const ProfilerRecording& recording) -> json
{
	auto obj = json::object();
	auto traceEvents = json::array();

	traceEvents.push_back(json::object({
		{ "cat", "__metadata" },
		{ "name", "process_name" },
		{ "ph", "M" },
		{ "ts", 0 },
		{ "pid", 1 },
		{ "tid", 1 },
		{ "args", json::object({
			{ "name", "Browser" }
		}) }
	}));

	traceEvents.push_back(json::object({
		{ "cat", "__metadata" },
		{ "name", "thread_name" },
		{ "ph", "M" },
		{ "ts", 0 },
		{ "pid", 1 },
		{ "tid", 1 },
		{ "args", json::object({
			{ "name", "CrBrowserMain" }
		}) }
	}));

	traceEvents.push_back(json::object({
		{ "cat", "__metadata" },
		{ "name", "thread_name" },
		{ "ph", "M" },
		{ "ts", 0 },
		{ "pid", 1 },
		{ "tid", 2 },
		{ "args", json::object({
			{ "name", "CrRendererMain" }
		}) }
	}));

	// needed to make the devtools enable frames
	traceEvents.push_back(json::object({
		{ "cat", "disabled-by-default-devtools.timeline" },
		{ "name", "TracingStartedInBrowser" },
		{ "ph", "I" },
		{ "ts", 0 },
		{ "pid", 1 },
		{ "tid", 1 },
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

	std::stack<ProfilerRecordingEvent> eventStack;

	for (const auto& event : recording.events)
	{
		switch ((fx::ProfilerEventType)event.what)
		{
		case fx::ProfilerEventType::BEGIN_TICK:
			traceEvents.push_back(json::object({
				{ "cat", "disabled-by-default-devtools.timeline.frame" },
				{ "name", "BeginFrame" },
				{ "s", "t" },
				{ "ph", "I" },
				{ "ts", event.when },
				{ "pid", 1 },
				{ "tid", 1 },
				{ "args", json::object({
					{ "layerTreeId", nullptr }
				}) }
			}));

			break;
		case fx::ProfilerEventType::END_TICK:
			traceEvents.push_back(json::object({
				{ "cat", "disabled-by-default-devtools.timeline.frame" },
				{ "name", "ActivateLayerTree" },
				{ "s", "t" },
				{ "ph", "I" },
				{ "ts", event.when },
				{ "pid", 1 },
				{ "tid", 1 },
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
				{ "pid", 1 },
				{ "tid", 1 },
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
				{ "pid", 1 },
				{ "tid", 1 },
				{ "args", json::object({
					// TODO: screenshot if client?
					{ "snapshot", 
						(event.why.empty()) 
							? "/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAMCAgICAgMCAgIDAwMDBAYEBAQEBAgGBgUGCQgKCgkICQkKDA8MCgsOCwkJDRENDg8QEBEQCgwSExIQEw8QEBD/2wBDAQMDAwQDBAgEBAgQCwkLEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBD/wAARCAABAAEDASIAAhEBAxEB/8QAFQABAQAAAAAAAAAAAAAAAAAAAAj/xAAUEAEAAAAAAAAAAAAAAAAAAAAA/8QAFQEBAQAAAAAAAAAAAAAAAAAABwn/xAAUEQEAAAAAAAAAAAAAAAAAAAAA/9oADAMBAAIRAxEAPwCdAAYqm//Z"
							: event.why }
				}) }
			}));

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
				{ "pid", 1 },
				{ "tid", 2 },
			}));

			eventStack.push(event);

			break;
		}
		case fx::ProfilerEventType::EXIT_RESOURCE:
		case fx::ProfilerEventType::EXIT_SCOPE:
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
				{ "pid", 1 },
				{ "tid", 2 },
				}));

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

		static ConsoleCommand statusCmd(profilerCtx.GetRef(), "status", []()
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
		});

		static ConsoleCommand recordCmd(profilerCtx.GetRef(), "record", [](std::string arg)
		{
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			if (arg == "stop")
			{
				profiler->StopRecording();
				console::Printf("cmd", "Stopped the recording\n");
			}
			else
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
		});

		static ConsoleCommand saveCmd(profilerCtx.GetRef(), "save", [](std::string path)
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

			msgpack::pack(writeWrapper, ConvertToStorage(profiler->Get()));
		});

		static ConsoleCommand dumpCmd(profilerCtx.GetRef(), "dump", []() {
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			PrintEvents(profiler->Get());
		});

		static ConsoleCommand viewCmd0(profilerCtx.GetRef(), "view", []() {
			auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();
			auto jsonData = ConvertToJSON(ConvertToStorage(profiler->Get()));

			ViewProfile(jsonData);
		});

		static ConsoleCommand viewCmd1(profilerCtx.GetRef(), "view", [](std::string path) {
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
		});
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
}

namespace fx {
	DLL_EXPORT bool g_recordProfilerTime;

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

	void ProfilerComponent::EnterResource(const std::string& resource, const std::string& cause)
	{
		PushEvent(ProfilerEventType::ENTER_RESOURCE, resource, cause);
	}
	void ProfilerComponent::ExitResource()
	{
		PushEvent(ProfilerEventType::EXIT_RESOURCE);
	}
	void ProfilerComponent::EnterScope(const std::string& scope)
	{
		PushEvent(ProfilerEventType::ENTER_SCOPE, scope, std::string{});
	}
	void ProfilerComponent::ExitScope()
	{
		PushEvent(ProfilerEventType::EXIT_SCOPE);
	}
	void ProfilerComponent::BeginTick()
	{
		PushEvent(fx::ProfilerEventType::BEGIN_TICK);
		EnterScope("Resource Tick");

		if (--m_frames == 0) { ProfilerComponent::StopRecording(); }
	}
	void ProfilerComponent::EndTick()
	{
		ExitScope();
		PushEvent(fx::ProfilerEventType::END_TICK, "", m_screenshot);
	}
	
	void ProfilerComponent::StartRecording(const int frames)
	{
		m_offset = fx::usec();
		m_frames = frames;
		m_recording = true;
		g_recordProfilerTime = true;
		m_events = {};
	}
	void ProfilerComponent::StopRecording()
	{
		m_recording = false;
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
}


static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("PROFILER_ENTER_SCOPE", [](fx::ScriptContext& ctx)
	{
		auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		profiler->EnterScope(ctx.GetArgument<const char*>(0));
	});

	fx::ScriptEngine::RegisterNativeHandler("PROFILER_EXIT_SCOPE", [](fx::ScriptContext& ctx)
	{
		auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		profiler->ExitScope();
	});

	fx::ScriptEngine::RegisterNativeHandler("PROFILER_IS_RECORDING", [](fx::ScriptContext& ctx)
	{
		auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		ctx.SetResult<int>(profiler->IsRecording());
	});

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* res)
	{
		auto resname = res->GetName();
		auto profiler = res->GetManager()->GetComponent<fx::ProfilerComponent>();

		res->OnTick.Connect([profiler, resname]()
		{
			profiler->EnterResource(resname, "tick");
		}, INT32_MIN);
		res->OnTick.Connect([profiler, resname]()
		{
			profiler->ExitResource();
		}, INT32_MAX);

		res->OnStart.Connect([=]()
		{
			auto event = res->GetComponent<fx::ResourceEventComponent>();
			event->OnTriggerEvent.Connect([=](const std::string& evName, const std::string& evPayload, const std::string& evSrc, bool* evCanceled)
			{
				profiler->EnterResource(resname, fmt::sprintf("event:%s", evName));
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
		resman->OnTick.Connect([profiler]() { profiler->EndTick(); }, INT32_MAX);
	});
});
