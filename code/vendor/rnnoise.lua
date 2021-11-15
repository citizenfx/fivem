return {
	include = function()
		includedirs "../vendor/rnnoise/include/"
	end,

	run = function()
		language 'C'
		kind 'StaticLib'

		files_project '../vendor/rnnoise/src/'  {
			'**.c',
			'**.h',
		}
	end
}