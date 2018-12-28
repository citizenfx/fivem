return {
	include = function()
		includedirs {
			"../vendor/utfcpp/source/"
		}
	end,
	
	run = function()
		targetname 'utfcpp_dummy'
		language 'C'
		kind 'StaticLib'
		
		files {
			'vendor/dummy.c'
		}
	end
}