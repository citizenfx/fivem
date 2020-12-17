return {
	include = function()
		includedirs {
			'../vendor/hdiffpatch/libHDiffPatch/HPatch/'
		}
	end,

	run = function()
		language "C"
		kind "StaticLib"

		files_project '../vendor/hdiffpatch/' {
			'libHDiffPatch/HPatch/**.c',
			'libHDiffPatch/HPatch/**.h'
		}

	end
}