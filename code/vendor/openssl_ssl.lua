local a = ...

local openssl = require('./vendor/openssl_ext')
local openssl_cfg = require('./vendor/openssl_cfg')

return {
	include = function()
		if os.istarget('windows') then
			includedirs { "../vendor/openssl/include/" }
			includedirs { "vendor/openssl/" }
		else
			links { 'ssl' }
		end
	end,
	
	run = function()
		if not os.istarget('windows') then
			targetname 'ssl_dummy'
			language 'C'
			kind 'StaticLib'
			return
		end
		
		language "C"
		kind "StaticLib"
		
		buildoptions { '/MP' }
		
		if not a then
			add_dependencies 'vendor:openssl_crypto'
		else
			staticruntime 'On'
		
			add_dependencies 'vendor:openssl_crypto_crt'
		end
		
		openssl.ssl_project(openssl_cfg)
		
		defines {
			"OPENSSL_NO_KRB5", "OPENSSL_EXPORT_VAR_AS_FUNCTION"
		}
	end
}