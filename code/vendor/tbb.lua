local a = ...

return {
	include = function()
		defines '__TBB_NO_IMPLICIT_LINKAGE=1'

		includedirs "../vendor/tbb/include/"

		if os.istarget('linux') then
			defines { 'TBB_USE_GLIBCXX_VERSION=90200', '__TBB_RESUMABLE_TASKS_USE_THREADS' }
		end
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		if a then
			staticruntime 'On'
		end

		includedirs { "../vendor/tbb/src/", "../vendor/tbb/src/rml/include/", "../vendor/tbb/build/vs2013/" }

		defines { '__TBB_BUILD', '__TBB_DYNAMIC_LOAD_ENABLED=0', '_TBB_USE_DEBUG=0', '_HAS_STD_BYTE=0' }

		files_project "../vendor/tbb/src/tbb/" {
			"address_waiter.cpp",
			"allocator.cpp",
			"arena.cpp",
			"arena_slot.cpp",
			"concurrent_bounded_queue.cpp",
			"dynamic_link.cpp",
			"exception.cpp",
			"governor.cpp",
			"global_control.cpp",
			"itt_notify.cpp",
			"main.cpp",
			"market.cpp",
			"misc.cpp",
			"misc_ex.cpp",
			"observer_proxy.cpp",
			"parallel_pipeline.cpp",
			"private_server.cpp",
			"profiling.cpp",
			"rml_tbb.cpp",
			"rtm_mutex.cpp",
			"rtm_rw_mutex.cpp",
			"semaphore.cpp",
			"small_object_pool.cpp",
			"task.cpp",
			"task_dispatcher.cpp",
			"task_group_context.cpp",
			"version.cpp",
			"queuing_rw_mutex.cpp",
		}

		filter { 'system:not windows' }
			defines { 'USE_PTHREAD' }

			buildoptions { '-mrtm', '-mwaitpkg' }

		filter { 'system:windows' }
			defines { 'USE_WINTHREAD', '_WIN32_WINNT=0x0601' }
	end
}
