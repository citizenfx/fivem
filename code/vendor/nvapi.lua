return {
	include = function()
		includedirs {
			'../vendor/nvapi/'
		}

		filter 'architecture:x64'
			libdirs '../vendor/nvapi/amd64/'
			links 'nvapi64'

		filter 'architecture:x86'
			libdirs '../vendor/nvapi/x86/'
			links 'nvapi'

		filter {}
	end,

	run = function()
		language "C"
		kind "StaticLib"

		files {
			'../vendor/nvapi/*.h',
			'vendor/dummy.c'
		}
	end
}
