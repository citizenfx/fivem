vars = {
	"citidev_root": "http://tohjo.eu/citidev"
}

deps = {
	"vendor/luajit": Var("citidev_root") + "/luajit.git",
	"build/premake": "http://tohjo.eu/citidev/premake-core.git",
	"vendor/jitasm": "http://tohjo.eu/citidev/jitasm.git",
	"vendor/yaml-cpp": "https://github.com/jbeder/yaml-cpp.git",
	"vendor/msgpack-c": "https://github.com/msgpack/msgpack-c.git@e183efcce27437bf7f9107ec1862f032d0b8a00e",
	"vendor/zlib": "https://github.com/madler/zlib.git",
	"vendor/gtest": "http://github.com/google/googletest.git",
	"vendor/protobuf": "https://github.com/google/protobuf.git@5eb73dfcce20bdfe421620cb31b7b98a0c5eec88",
	"vendor/libopus": "http://git.opus-codec.org/opus.git",
	"vendor/pash": "ssh://git@git.internal.fivem.net/cfx/pash.git",
	"vendor/breakpad": "https://chromium.googlesource.com/breakpad/breakpad@704f41ec901c419f8c321742114b415e6f5ceacc",
	"vendor/udis86": "https://github.com/vmt/udis86.git",
	"vendor/tinyxml2": "https://github.com/leethomason/tinyxml2.git@9c8582c7c3a7a55798940dca3bfbb78d4d97fa05",
	"vendor/cpp-uri": "https://github.com/cpp-netlib/uri.git@094e114255bb228341163803976a9f55b2551371",
	"vendor/picohttpparser": "https://github.com/h2o/picohttpparser.git@98bcc1c3b431d05d4584af66082da48e4638a675",
	"vendor/libssh": "http://tohjo.eu/citidev/libssh.git",
	"vendor/xz": "http://git.tukaani.org/xz.git",
	"vendor/curl": "https://github.com/bagder/curl.git@curl-7_52_1",
	"vendor/leveldb": "http://tohjo.eu/citidev/leveldb.git",
	"vendor/minhook": "https://github.com/TsudaKageyu/minhook.git",
	"vendor/rapidjson": "https://github.com/miloyip/rapidjson.git",
	"vendor/libuv": "https://github.com/libuv/libuv.git@7639dd510ec13dd577677dce2a673f5046432903",
	"vendor/fmtlib": "https://github.com/fmtlib/fmt.git",
	"vendor/cpr": "https://github.com/whoshuu/cpr.git@master"
}

hooks = [
	{
		"name": "gen_udis_script",
		"pattern": "vendor/udis86/",
		"action": [ "citizenmp\prebuild_udis86.cmd" ]
	},
	{
		"name": "build_premake_win",
		"pattern": "build/premake/",
		"action": [ "citizenmp\prebuild_premake.cmd" ]
	},
	{
		"name": "generic_prebuild_win",
		"action": [ "citizenmp\prebuild_misc.cmd" ]
	}
]
