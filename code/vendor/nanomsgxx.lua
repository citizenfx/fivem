return {
	include = function()
		includedirs '../vendor/nanomsgxx/src/'
	end,
	
	run = function()
		targetname 'nanomsgxx'
		language 'C++'
		kind 'StaticLib'
		
		add_dependencies 'vendor:nanomsg'
		
		files_project '../vendor/nanomsgxx/src/' {
			'**.c',
			'**.cpp'
		}
	end
}