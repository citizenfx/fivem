return {
	include = function()
		includedirs "../vendor/enet/include/"

		if os.istarget('windows') then
			defines { '_WIN32_WINNT=0x0A00' }

			links { 'ws2_32', 'winmm' }
		end
	end,

	run = function()
		language "C"
		kind "StaticLib"

		defines { "HAS_INET_NTOP", "HAS_INET_PTON" }

		if not os.istarget('windows') then
			defines { "HAS_SOCKLEN_T" }
		end

		files_project "../vendor/enet/" {
			"*.c"
		}

		if _OPTIONS['game'] == 'server' then
			removefiles '../vendor/enet/win32.c'
			removefiles '../vendor/enet/unix.c'
		end
	end
}
