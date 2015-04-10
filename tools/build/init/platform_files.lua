-- final override for files to exclude platform-specific files
local origFiles = files

files = function(...)
	origFiles(...)

	filter "system:windows"
		excludes { "**/*.Posix.cpp" }

	filter "system:not windows"
		excludes { "**/*.Win32.cpp" }

	-- reset configuration
	filter {}
end