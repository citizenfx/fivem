-- override for GCC-style CXXFLAGS
local getcxxflags = premake.tools.gcc.getcxxflags;
function premake.tools.gcc.getcxxflags(cfg)
    local r = getcxxflags(cfg)

    table.insert(r, "-std=c++14")
    table.insert(r, "-stdlib=libc++")
    table.insert(r, "-Xclang") -- to enable the following option
    table.insert(r, "-fno-sized-deallocation") -- C++14 mode in clang causes sized deallocation operator delete to be referenced, but we don't want to depend on latest libc++

    return r
end

local config = premake.config

function premake.tools.gcc.ldflags.kind.SharedLib(cfg)
	local r = { iif(cfg.system == premake.MACOSX, "-dynamiclib", "-shared") }
	if cfg.system == "windows" and not cfg.flags.NoImportLib then
		table.insert(r, '-Wl,--out-implib="' .. cfg.linktarget.relpath .. '"')
	elseif cfg.system == premake.LINUX then
		table.insert(r, '-Wl,-soname="' .. cfg.linktarget.name .. '"')
	elseif cfg.system == premake.MACOSX then
		table.insert(r, '-Wl,-install_name,"@rpath/' .. cfg.linktarget.name .. '"')
	end
	return r
end

function premake.tools.gcc.getlinks(cfg, systemonly)
	local result = {}

	-- Don't use the -l form for sibling libraries, since they may have
	-- custom prefixes or extensions that will confuse the linker. Instead
	-- just list out the full relative path to the library.

	if not systemonly then
		local siblings = config.getlinks(cfg, "siblings", "fullpath")
		local sharedlibextension = ".so"
		sharedlibextension = iif(cfg.system == premake.WINDOWS, ".dll", sharedlibextension)
		sharedlibextension = iif(cfg.system == premake.MACOSX, ".dylib", sharedlibextension)
		for _, sibling in ipairs(siblings) do
			if path.getextension(sibling) == sharedlibextension then
				local fullpath = path.getabsolute(path.rebase(path.getdirectory(sibling), cfg.location, os.getcwd()))
				local rpath = path.getrelative(cfg.targetdir, fullpath)
				if cfg.system == premake.LINUX then
					rpath = iif(rpath == ".", "", "/" .. rpath)
					rpath = " -Wl,-rpath,'$$ORIGIN" .. rpath .. "'"
				elseif cfg.system == premake.MACOSX then
					rpath = " -Wl,-rpath,'@loader_path/" .. rpath .. "'"
				else
					rpath = ""
				end
				if (#rpath > 0) and not table.contains(result, rpath) then
					table.insert(result, rpath)
				end
			end

			table.insert(result, sibling)
		end
	end

	-- The "-l" flag is fine for system libraries

	local links = config.getlinks(cfg, "system", "fullpath")
	for _, link in ipairs(links) do
		if path.isframework(link) then
			table.insert(result, "-framework " .. path.getbasename(link))
		elseif path.isobjectfile(link) then
			table.insert(result, link)
		else
			table.insert(result, "-l" .. path.getbasename(link))
		end
	end

	return result
end