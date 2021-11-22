return function()
	filter {}

	if os.istarget('windows') then
		flags { "LinkTimeOptimization" }
	end
end
