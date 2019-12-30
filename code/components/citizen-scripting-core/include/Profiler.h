#pragma once

#include <fmt/format.h>
#include <tbb/concurrent_vector.h>

#include <ResourceManager.h>

#include <json.hpp>

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

	extern
#ifdef COMPILING_CITIZEN_SCRIPTING_CORE
		DLL_EXPORT
#else
		DLL_IMPORT
#endif
		bool g_recordProfilerTime;

	struct ProfilerEvent {

		inline ProfilerEvent(ProfilerEventType what, std::string where, std::string why)
			: what(what), where(where), why(why)
		{
			when = (g_recordProfilerTime) ? usec() : std::chrono::microseconds{ 0 };
		};
		inline ProfilerEvent(ProfilerEventType what)
			: what(what)
		{
			when = (g_recordProfilerTime) ? usec() : std::chrono::microseconds{ 0 };
		};
		std::chrono::microseconds when;
		ProfilerEventType what;
		std::string where;
		std::string why;
	};

	class
#ifdef COMPILING_CITIZEN_SCRIPTING_CORE
		DLL_EXPORT
#else
		DLL_IMPORT
#endif
		ProfilerComponent : public fwRefCountable {
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
		void EnterScope(const std::string& scope);
		void ExitScope();
		void BeginTick();
		void EndTick();

		void SubmitScreenshot(const void* imageRgb, size_t width, size_t height);

		auto IsRecording() -> bool;
		auto GetFrames() -> int;

		void StartRecording(int frames);
		void StopRecording();
		auto Get() -> const tbb::concurrent_vector<ProfilerEvent>&;

	public:
		fwEvent<const nlohmann::json&> OnRequestView;

	private:
		std::string m_screenshot;

		tbb::concurrent_vector<ProfilerEvent> m_events;
		bool m_recording = false;
		std::chrono::microseconds m_offset;
		int m_frames = 0;
	};
}

DECLARE_INSTANCE_TYPE(fx::ProfilerComponent);
