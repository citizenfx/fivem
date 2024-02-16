return {
	run = function()
		language "C++"
		kind 'StaticLib'

		files_project('../vendor/boost-submodules/boost-system/src/')
		{
			'error_code.cpp'
		}
	end
}
