return {
	include = function()
		includedirs { "../vendor/nng/src/" }
	end,
	
	run = function()
		targetname "nng"
		language "C"
		kind "SharedLib"
		
		defines { "NNG_HAVE_PULL0", "NNG_HAVE_PUSH0", "NNG_HAVE_REQ0", "NNG_HAVE_REP0", "NNG_TRANSPORT_INPROC", "NNG_TRANSPORT_IPC", "NNG_SHARED_LIB", "NNG_LITTLE_ENDIAN" }
		
		files_project '../vendor/nng/src/' {
			"nng.c",
			"nng.h",

			"core/defs.h",

			"core/aio.c",
			"core/aio.h",
			"core/clock.c",
			"core/clock.h",
			"core/device.c",
			"core/device.h",
			"core/endpt.c",
			"core/endpt.h",
			"core/file.c",
			"core/file.h",
			"core/idhash.c",
			"core/idhash.h",
			"core/init.c",
			"core/init.h",
			"core/list.c",
			"core/list.h",
			"core/message.c",
			"core/message.h",
			"core/msgqueue.c",
			"core/msgqueue.h",
			"core/nng_impl.h",
			"core/options.c",
			"core/options.h",
			--"core/pollable.c",
			--"core/pollable.h",
			"core/panic.c",
			"core/panic.h",
			"core/pipe.c",
			"core/pipe.h",
			"core/platform.h",
			"core/protocol.c",
			"core/protocol.h",
			"core/random.c",
			"core/random.h",
			"core/reap.c",
			"core/reap.h",
			"core/socket.c",
			"core/socket.h",
			"core/strs.c",
			"core/strs.h",
			"core/taskq.c",
			"core/taskq.h",
			"core/thread.c",
			"core/thread.h",
			"core/timer.c",
			"core/timer.h",
			"core/transport.c",
			"core/transport.h",
			"core/url.c",
			"core/url.h",
		}
		
		if os.istarget('windows') then
			defines { "NNG_PLATFORM_WINDOWS", "_CRT_RAND_S", "NNG_HAVE_SNPRINTF", "NNG_HAVE_CONDVAR" }
			
			links { 'ws2_32' }
		
			files_project '../vendor/nng/src/' {
				"platform/windows/win_impl.h",
				"platform/windows/win_clock.c",
				"platform/windows/win_debug.c",
				"platform/windows/win_file.c",
				"platform/windows/win_iocp.c",
				"platform/windows/win_ipc.c",
				"platform/windows/win_pipe.c",
				"platform/windows/win_rand.c",
				"platform/windows/win_resolv.c",
				"platform/windows/win_sockaddr.c",
				"platform/windows/win_tcp.c",
				"platform/windows/win_thread.c",
				"platform/windows/win_udp.c",
			}
		elseif os.istarget('linux') then
			defines { "NNG_PLATFORM_POSIX", "NNG_PLATFORM_LINUX", "NNG_USE_EVENTFD", "NNG_HAVE_EPOLL", "NNG_HAVE_SEMAPHORE_PTHREAD" }
		
			files_project '../vendor/nng/src/' {
				"platform/posix/posix_impl.h",
				"platform/posix/posix_config.h",
				"platform/posix/posix_aio.h",
				"platform/posix/posix_pollq.h",

				"platform/posix/posix_alloc.c",
				"platform/posix/posix_clock.c",
				"platform/posix/posix_debug.c",
				"platform/posix/posix_epdesc.c",
				"platform/posix/posix_file.c",
				"platform/posix/posix_ipc.c",
				"platform/posix/posix_pipe.c",
				"platform/posix/posix_pipedesc.c",
				"platform/posix/posix_rand.c",
				"platform/posix/posix_resolv_gai.c",
				"platform/posix/posix_sockaddr.c",
				"platform/posix/posix_tcp.c",
				"platform/posix/posix_thread.c",
				"platform/posix/posix_udp.c",
				
				"platform/posix/posix_pollq_epoll.c",
			}
		end
		
		files_project '../vendor/nng/src/' {
			"supplemental/util/platform.c",
			"supplemental/util/platform.h",
			"supplemental/util/options.c",
			"supplemental/util/options.h",
			
			"protocol/pipeline0/push.c",
			"protocol/pipeline0/pull.c",
			
			"protocol/pipeline0/push.h",
			"protocol/pipeline0/pull.h",

			"protocol/reqrep0/req.c",
			--"protocol/reqrep0/xreq.c",
			"protocol/reqrep0/rep.c",
			--"protocol/reqrep0/xrep.c",
			
			"protocol/reqrep0/req.h",
			"protocol/reqrep0/rep.h",
						
			"transport/inproc/inproc.c",
			"transport/inproc/inproc.h",
			
			"transport/ipc/ipc.c",
			"transport/ipc/ipc.h",
		}
	end
}