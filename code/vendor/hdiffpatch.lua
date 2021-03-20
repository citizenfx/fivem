local a = ...

return {
	include = function()
		includedirs {
			'../vendor/hdiffpatch/libHDiffPatch/HPatch/'
		}
	end,

	run = function()
		language "C"
		kind "StaticLib"

		if a then
			staticruntime 'On'
		end

		files_project '../vendor/hdiffpatch/' {
			'libHDiffPatch/HPatch/**.c',
			'libHDiffPatch/HPatch/**.h'
		}

	end
}