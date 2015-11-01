function process_includedirs(list)
	local r = ""

	for _, v in ipairs(list) do
		r = r .. ' -I "' .. v .. '"'
	end

	return r
end

local old_files = files

function files(x)
	old_files(x)

	if type(x) == 'table' then
		for _, v in ipairs(x) do
			if v:endswith('.idl') then
				filter 'files:**.idl'

				local prj_root = '%{prj.location}/../../'

				if _OPTIONS['game'] == 'server' then
					prj_root = prj_root .. '../'
				end

				buildcommands {
					'python "' .. prj_root .. 'tools/idl/header.py" -o "%{file and file.directory or ""}/%{file and file.basename or ""}.h" %{process_includedirs(prj.includedirs)} %{file and file.relpath or ""}'
				}

				buildoutputs { '%{file.directory}/%{file.basename}.h' }

				filter {}

				break
			end
		end
	end

end