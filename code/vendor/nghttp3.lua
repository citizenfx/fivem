return {
	include = function()
		includedirs { "../vendor/nghttp3/lib/includes/" }

		defines { '_SSIZE_T_DEFINED=1', '_U_=', 'NGHTTP3_STATICLIB' }
		
		if os.istarget('windows') then
			defines 'ssize_t=__int64'
		else
			-- check if this is a glibc distro
			local succ, status, code = os.execute('ldd --version | grep -c "Free Software Foundation"')
			
			if code ~= 0 then
				defines 'ssize_t=long'
			end
		end
	end,

	run = function()
		targetname "nghttp3"

		language "C"
		kind "StaticLib"

		flags "NoRuntimeChecks"

		files_project '../vendor/nghttp3/lib/'
		{
			'*.c'
		}
	end
}
