vars = {
	"citidev_root": "http://tohjo.ez.lv/citidev"
}

deps = {
	"vendor/luajit": Var("citidev_root") + "/luajit.git",
	"build/premake": "http://github.com/annulen/premake.git",
	"vendor/jitasm": "http://jitasm.googlecode.com/svn/trunk/",
	"vendor/yaml-cpp": "https://github.com/bminor/yaml-cpp.git",
	"vendor/msgpack-c": "https://github.com/msgpack/msgpack-c.git",
	"vendor/zlib": "https://github.com/madler/zlib.git",
	"vendor/gtest": "http://googletest.googlecode.com/svn/trunk/"
}

hooks = [
	{
		"name": "premake_premake_win",
		"pattern": "build/premake/",
		"action": [ "build\premake5.exe", "--file=build/premake/premake5.lua", "--to=project", "vs2013" ]
	},
	{
		"name": "premake_premake_win",
		"pattern": "build/premake/",
		"action": [ "build\premake5.exe", "--file=build/premake/premake5.lua", "embed" ]
	},
	{
		"name": "build_premake_win",
		"pattern": "build/premake/",
		"action": [ "msbuild", "build/premake/project/Premake5.sln", "/p:configuration=release" ]
	},
	{
		"name": "build_luajit_win",
		"pattern": "vendor/luajit/src/",
		"action": ["cd", "vendor/luajit/src", "&&", "msvcbuild", "static" ]
	}
]