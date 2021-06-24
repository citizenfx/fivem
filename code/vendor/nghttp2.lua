local a = ...

return {
	include = function()
		includedirs { "../vendor/nghttp2/lib/includes/" }

		defines { '_SSIZE_T_DEFINED=1', '_U_=', 'NGHTTP2_STATICLIB' }
		
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
		language "C"
		kind "StaticLib"

		if a then
			staticruntime 'On'
		end

		flags "NoRuntimeChecks"

		files_project '../vendor/nghttp2/lib/'
		{
			'*.c'
		}
	end
}
