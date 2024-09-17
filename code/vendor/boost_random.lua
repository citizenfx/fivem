return {
	include = function()
		add_dependencies { 'vendor:boost_system' }
	end,

	run = function()
		language "C++"
		kind 'StaticLib'

		files_project('../vendor/boost-submodules/boost-random/src/')
		{
			'random_device.cpp'
		}
	end
}
