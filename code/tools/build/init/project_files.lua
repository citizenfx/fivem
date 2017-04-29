-- files based on a root directory
function files_project(name)
	return function(f)
		local t = {}

		for k, file in ipairs(f) do
			table.insert(t, name .. file)
		end

		files(t)
	end
end