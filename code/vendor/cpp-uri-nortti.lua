local tbl = dofile('cpp-uri.lua')

local oldRun = tbl.run

tbl.run = function()
	oldRun()
	
	configuration {}
		rtti 'Off'
end

return tbl