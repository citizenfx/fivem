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

		local minhook_dir = "../vendor/minhook/src/"

		files_project(minhook_dir) {
			"buffer.c",
			"hook.c",
			"trampoline.c",
		}

		filter { 'architecture:x86' }
			files_project(minhook_dir) {
				"HDE/hde32.c"
			}

		filter { 'architecture:x64' }
			files_project(minhook_dir) {
				"HDE/hde64.c"
			}
	end
}
