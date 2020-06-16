return {
	include = function()
		includedirs { "../vendor/ngtcp2/lib/includes/", "../vendor/ngtcp2/crypto/includes" }

		defines { '_SSIZE_T_DEFINED=1', '_U_=', 'NGTCP2_STATICLIB' }
		
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
		targetname "ngtcp2"

		language "C"
		kind "StaticLib"

		flags "NoRuntimeChecks"
		
		includedirs { "../vendor/ngtcp2/lib/", "../vendor/ngtcp2/crypto/" }

		files_project '../vendor/ngtcp2/lib/'
		{
			'*.c'
		}
		
		files_project '../vendor/ngtcp2/crypto/'
		{
			'*.c',
			'openssl/*.c'
		}
		
		add_dependencies { 'vendor:openssl_ssl', 'vendor:openssl_crypto' }
	end
}
