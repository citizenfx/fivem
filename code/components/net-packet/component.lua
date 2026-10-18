return function()
	filter {}

	if os.istarget('windows') then
		linktimeoptimization "On"
	end
end

