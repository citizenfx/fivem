local a = ...

local openssl = require('./vendor/openssl_ext')
local openssl_cfg = require('./vendor/openssl_cfg')

return {
	include = function()
		if os.istarget('windows') then
			includedirs { "vendor/openssl/" }
			includedirs { "../vendor/openssl/include/" }
		else
			links { 'crypto' }
		end
		
		defines { 'OPENSSL_NO_KRB5' }
	end,
	
	run = function()
		if not os.istarget('windows') then
			targetname 'crypto_dummy'
			language 'C'
			kind 'StaticLib'
			return
		end
		
		language "C"
		kind "StaticLib"
		
		if a then
			staticruntime 'On'
		end
		
		buildoptions { '/MP' }
		
		links {'ws2_32'}
		
		openssl.crypto_project(openssl_cfg)
		
		defines { "OPENSSL_EXPORT_VAR_AS_FUNCTION" }
	end
}