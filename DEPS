vars = {
	"citidev_root": "http://tohjo.eu/citidev"
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
	"vendor/protobuf": "https://github.com/google/protobuf.git",
	"vendor/libopus": "git://git.opus-codec.org/opus.git",
	"vendor/pash": "http://tohjo.eu/citidev/pash.git",
	"vendor/breakpad": "http://google-breakpad.googlecode.com/svn/trunk/",
	"vendor/udis86": "https://github.com/vmt/udis86.git",
	"vendor/tinyxml2": "https://github.com/leethomason/tinyxml2.git",
	"vendor/cpp-uri": "https://github.com/cpp-netlib/uri.git"
}

hooks = [
	{
		"name": "gen_udis_script",
		"pattern": "vendor/udis86/",
		"action": [ "citizenmp\prebuild_udis86.cmd" ]
	},
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
