#pragma once

#include <fmt/format.h>
#include <tbb/concurrent_vector.h>

#include <ResourceManager.h>

#include <json.hpp>

#ifdef COMPILING_CITIZEN_SCRIPTING_CORE
	#define FX_PROFILER_LINKAGE DLL_EXPORT
#else
	#define FX_PROFILER_LINKAGE DLL_IMPORT
#endif

namespace fx {
	enum class ProfilerEventType {
		BEGIN_TICK,     // BEGIN_TICK(µs when)
		ENTER_RESOURCE, // ENTER_RESOURCE(µs when, string res, string cause)
		EXIT_RESOURCE,  // EXIT_RESOURCE(µs when)
		ENTER_SCOPE,    // ENTER_SCOPE(µs when, string scope)
		EXIT_SCOPE,     // EXIT_SCOPE(µs when)
		END_TICK,       // END_TICK(µs when)
	};
	
	inline auto usec()
	{
		using namespace std::chrono;
		return duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch());
	}

	extern FX_PROFILER_LINKAGE bool g_recordProfilerTime;

	struct ProfilerEvent {
		// DevTools supports negative HeapUsage values
		using memory_t = int64_t;

		inline ProfilerEvent(ProfilerEventType what, std::string where, std::string why, memory_t much)
			: what(what), where(where), why(why), much(much)
		{
			when = (g_recordProfilerTime) ? usec() : std::chrono::microseconds{ 0 };
		};

		inline ProfilerEvent(ProfilerEventType what, std::string where, std::string why)
			: what(what), where(where), why(why), much(0)
		{
			when = (g_recordProfilerTime) ? usec() : std::chrono::microseconds{ 0 };
		};

		inline ProfilerEvent(ProfilerEventType what, memory_t much)
			: what(what), much(much)
		{
			when = (g_recordProfilerTime) ? usec() : std::chrono::microseconds{ 0 };
		};

		inline ProfilerEvent(ProfilerEventType what)
			: what(what), much(0)
		{
			when = (g_recordProfilerTime) ? usec() : std::chrono::microseconds{ 0 };
		};

		std::chrono::microseconds when;
		ProfilerEventType what;
		std::string where;
		std::string why;
		memory_t much;  /* fxScripting::GetMemoryUsage() */
	};

	class FX_PROFILER_LINKAGE ProfilerComponent : public fwRefCountable {
	public:
		template<typename... TArgs>
		inline void PushEvent(TArgs&&... args)
		{
			if (m_recording)
			{
				ProfilerEvent ev{ std::forward<TArgs>(args)... };

				ev.when -= m_offset;
				m_events.push_back(ev);
			}
		}
		
		void EnterResource(const std::string& resource, const std::string& cause);
		void ExitResource();
		void EnterScope(const std::string& scope, ProfilerEvent::memory_t memoryUsage = 0);
		void ExitScope(ProfilerEvent::memory_t memoryUsage = 0);
		void BeginTick(ProfilerEvent::memory_t memoryUsage = 0);
		void EndTick(ProfilerEvent::memory_t memoryUsage = 0);

		void SubmitScreenshot(const void* imageRgb, size_t width, size_t height);

		auto IsRecording() -> bool;
		auto GetFrames() -> int;

		void StartRecording(int frames);
		void StopRecording();
		auto Get() -> const tbb::concurrent_vector<ProfilerEvent>&;

	public:
		fwEvent<const nlohmann::json&> OnRequestView;

	private:
#ifndef IS_FXSERVER
		std::string m_screenshot;
#endif

		tbb::concurrent_vector<ProfilerEvent> m_events;
		bool m_recording = false;
		std::chrono::microseconds m_offset;
		int m_frames = 0;
	};
}

DECLARE_INSTANCE_TYPE(fx::ProfilerComponent);
