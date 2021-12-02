local a = ...

return {
	include = function()
		includedirs { "../vendor/zstd/lib/" }
	end,

	run = function()
		language "C"
		kind "StaticLib"
		
		if not a then
			staticruntime "On"
		end

		files_project '../vendor/zstd/lib/' {
			'**.h',
			'common/*.c',
			'decompress/*.c',
		}

		defines { "ZSTD_MULTITHREAD" }
	end
}
