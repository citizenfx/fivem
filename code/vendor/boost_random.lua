return {
	include = function()
		add_dependencies { 'vendor:boost_system' }
	end,

	run = function()
		language "C++"
		kind 'StaticLib'

		files_project(os.getenv("BOOST_ROOT") .. '/libs/random/src/')
		{
			'*.cpp'
		}
	end
}
