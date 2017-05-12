return {
	run = function()
		language "C++"
		kind 'StaticLib'

		files_project(os.getenv("BOOST_ROOT") .. '/libs/system/src/')
		{
			'*.cpp'
		}
	end
}
