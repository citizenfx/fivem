return function()
	filter {}
	configuration {}

	if os.istarget('windows') then
		flags { "LinkTimeOptimization" }
	end
end