local tbl = dofile('cpp-url.lua')

local oldRun = tbl.run

tbl.run = function()
	oldRun()
	
	filter {}
		rtti 'Off'
end

return tbl
