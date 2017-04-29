return {
	include = function()
		includedirs { "../vendor/libuv/include/" }
	end,

	run = function()
		language "C"
		kind "SharedLib"

		includedirs { "../vendor/libuv/src/" }

		files_project '../vendor/libuv/'
		{
			'include/uv.h',
			'include/tree.h',
			'include/uv-errno.h',
			'include/uv-threadpool.h',
			'include/uv-version.h',
			'src/fs-poll.c',
			'src/heap-inl.h',
			'src/inet.c',
			'src/queue.h',
			'src/threadpool.c',
			'src/uv-common.c',
			'src/uv-common.h',
			'src/version.c'
		}

		defines { 'BUILDING_UV_SHARED=1' }

		filter "system:not windows"
			defines { "_LARGEFILE_SOURCE", "_FILEOFFSET_BITS=64" }

			links { 'm' }

			linkoptions { '-pthread' }

			files_project '../vendor/libuv/'
			{
				'include/uv-unix.h',
				'include/uv-linux.h',
				'include/uv-sunos.h',
				'include/uv-darwin.h',
				'include/uv-bsd.h',
				'include/uv-aix.h',
				'src/unix/async.c',
				'src/unix/atomic-ops.h',
				'src/unix/core.c',
				'src/unix/dl.c',
				'src/unix/fs.c',
				'src/unix/getaddrinfo.c',
				'src/unix/getnameinfo.c',
				'src/unix/internal.h',
				'src/unix/loop.c',
				'src/unix/loop-watcher.c',
				'src/unix/pipe.c',
				'src/unix/poll.c',
				'src/unix/process.c',
				'src/unix/signal.c',
				'src/unix/spinlock.h',
				'src/unix/stream.c',
				'src/unix/tcp.c',
				'src/unix/thread.c',
				'src/unix/timer.c',
				'src/unix/tty.c',
				'src/unix/udp.c',

				-- linux
				'src/unix/linux-core.c',
				'src/unix/linux-inotify.c',
				'src/unix/linux-syscalls.c',
				'src/unix/linux-syscalls.h',
			}

		filter "system:windows"
			defines { "_GNU_SOURCE" }

			links { 'advapi32', 'iphlpapi', 'psapi', 'shell32', 'ws2_32', 'userenv' }

			files_project '../vendor/libuv/'
			{
				'src/win/async.c',
				'src/win/atomicops-inl.h',
				'src/win/core.c',
				'src/win/detect-wakeup.c',
				'src/win/dl.c',
				'src/win/error.c',
				'src/win/fs.c',
				'src/win/fs-event.c',
				'src/win/getaddrinfo.c',
				'src/win/getnameinfo.c',
				'src/win/handle.c',
				'src/win/handle-inl.h',
				'src/win/internal.h',
				'src/win/loop-watcher.c',
				'src/win/pipe.c',
				'src/win/thread.c',
				'src/win/poll.c',
				'src/win/process.c',
				'src/win/process-stdio.c',
				'src/win/req.c',
				'src/win/req-inl.h',
				'src/win/signal.c',
				'src/win/stream.c',
				'src/win/stream-inl.h',
				'src/win/tcp.c',
				'src/win/tty.c',
				'src/win/timer.c',
				'src/win/udp.c',
				'src/win/util.c',
				'src/win/winapi.c',
				'src/win/winapi.h',
				'src/win/winsock.c',
				'src/win/winsock.h',
			}
	end
}
