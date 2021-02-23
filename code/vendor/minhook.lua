local staticRuntime = ...

return {
	include = function()
		includedirs "../vendor/minhook/include/"
	end,

	run = function()
		language "C"
		kind "StaticLib"
		
		if staticRuntime then
			staticruntime "On"
		end

		-- TODO: 32-bit stuffies
		files_project "../vendor/minhook/src/" {
			"buffer.c",
			"HDE/hde64.c",
			"hook.c",
			"trampoline.c",
		}
	end
}