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
	"vendor/gmock": "http://googlemock.googlecode.com/svn/trunk/",
	"vendor/gtest": "http://googletest.googlecode.com/svn/trunk/",
	"vendor/protobuf": "http://protobuf.googlecode.com/svn/trunk/",
	"vendor/libopus": "git://git.opus-codec.org/opus.git"
}

hooks = [
	{
		"name": "premake_premake_win",
		"pattern": "build/premake/",
		"action": [ "citizenmp\prebuild_premake.cmd" ]
	},
	{
		"name": "build_premake_win",
		"pattern": "build/premake/",
		"action": [ "msbuild", "build/premake/Premake5.sln", "/p:configuration=release" ]
	},
	{
		"name": "build_luajit_win",
		"pattern": "vendor/luajit/src/",
		"action": ["cd", "vendor/luajit/src", "&&", "msvcbuild", "static" ]
	}
]
