-- define functions for private component repos
local private_repos = {}

function private_repo(repoPath)
	-- add to list
	table.insert(private_repos, path.getabsolute(repoPath))

	-- load component config
	dofile(repoPath .. '/components/config.lua')
end

function all_private_repos()
	return ipairs(private_repos)
end

function load_privates(file)
	-- check if the file exists
	local f = io.open(file)

	if f then
		f:close()
	else
		return
	end

	-- load the file
	dofile(file)
end