local a = ...

return {
	include = function()
		defines '__TBB_NO_IMPLICIT_LINKAGE=1'

		includedirs "../vendor/tbb/include/"
	end,

	run = function()
		language "C++"
		kind "StaticLib"

		if a then
			staticruntime 'On'
		end

		includedirs { "../vendor/tbb/src/", "../vendor/tbb/src/rml/include/", "../vendor/tbb/build/vs2013/" }

		defines { '__TBB_BUILD', '__TBB_DYNAMIC_LOAD_ENABLED=0', '_TBB_USE_DEBUG=0', '_HAS_STD_BYTE=0' }

		files_project "../vendor/tbb/" {
			"src/tbb/concurrent_hash_map.cpp",
			"src/tbb/concurrent_queue.cpp",
			"src/tbb/concurrent_vector.cpp",
			"src/tbb/dynamic_link.cpp",
			"src/tbb/itt_notify.cpp",
			"src/tbb/cache_aligned_allocator.cpp",
			"src/tbb/pipeline.cpp",
			"src/tbb/queuing_mutex.cpp",
			"src/tbb/queuing_rw_mutex.cpp",
			"src/tbb/reader_writer_lock.cpp",
			"src/tbb/spin_rw_mutex.cpp",
			"src/tbb/x86_rtm_rw_mutex.cpp",
			"src/tbb/spin_mutex.cpp",
			"src/tbb/critical_section.cpp",
			"src/tbb/mutex.cpp",
			"src/tbb/recursive_mutex.cpp",
			"src/tbb/condition_variable.cpp",
			"src/tbb/tbb_thread.cpp",
			"src/tbb/concurrent_monitor.cpp",
			"src/tbb/semaphore.cpp",
			"src/tbb/private_server.cpp",
			"src/rml/client/rml_tbb.cpp",
			"src/tbb/tbb_misc.cpp",
			"src/tbb/tbb_misc_ex.cpp",
			"src/tbb/task.cpp",
			"src/tbb/task_group_context.cpp",
			"src/tbb/governor.cpp",
			"src/tbb/market.cpp",
			"src/tbb/arena.cpp",
			"src/tbb/scheduler.cpp",
			"src/tbb/observer_proxy.cpp",
			"src/tbb/tbb_statistics.cpp",
			"src/tbb/tbb_main.cpp"
		}

		filter { 'system:not windows' }
			defines { 'USE_PTHREAD' }

		filter { 'system:windows' }
			defines { 'USE_WINTHREAD' }

			files_project "../vendor/tbb/" {
				'src/tbb/intel64-masm/intel64_misc.asm',
				'src/tbb/intel64-masm/atomic_support.asm',
			}
	end
}
