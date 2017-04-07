vars = {
	"citidev_root": "http://tohjo.eu/citidev"
}

deps = {
	"vendor/luajit": Var("citidev_root") + "/luajit.git",
	"build/premake": "https://github.com/premake/premake-core.git",
	"vendor/jitasm": "http://tohjo.eu/citidev/jitasm.git",
	"vendor/yaml-cpp": "https://github.com/jbeder/yaml-cpp.git@11607eb5bf1258641d80f7051e7cf09e317b4746",
	"vendor/msgpack-c": "https://github.com/msgpack/msgpack-c.git@e183efcce27437bf7f9107ec1862f032d0b8a00e",
	"vendor/zlib": "https://github.com/madler/zlib.git@v1.2.11",
	"vendor/protobuf": "https://github.com/google/protobuf.git@5eb73dfcce20bdfe421620cb31b7b98a0c5eec88",
	"vendor/breakpad": "https://chromium.googlesource.com/breakpad/breakpad@704f41ec901c419f8c321742114b415e6f5ceacc",
	"vendor/udis86": "https://github.com/vmt/udis86.git",
	"vendor/tinyxml2": "https://github.com/leethomason/tinyxml2.git@b840b7e6736bca889ae9c9ce6d7b3ed9d85bbc64",
	"vendor/cpp-uri": "https://github.com/cpp-netlib/uri.git@094e114255bb228341163803976a9f55b2551371",
	"vendor/picohttpparser": "https://github.com/h2o/picohttpparser.git@98bcc1c3b431d05d4584af66082da48e4638a675",
	"vendor/libssh": "https://github.com/citizenfx/libssh.git",
	"vendor/xz": "http://git.tukaani.org/xz.git@v5.2.3",
	"vendor/curl": "https://github.com/bagder/curl.git@curl-7_54_0",
	"vendor/leveldb": "https://github.com/citizenfx/leveldb.git",
	"vendor/minhook": "https://github.com/TsudaKageyu/minhook.git",
	"vendor/rapidjson": "https://github.com/miloyip/rapidjson.git@e6d7247ed9a13242e7f683d0a6425aa04a1b232c",
	"vendor/libuv": "https://github.com/libuv/libuv.git@7639dd510ec13dd577677dce2a673f5046432903",
	"vendor/fmtlib": "https://github.com/fmtlib/fmt.git@d49f2061838ec4c0ee9951d917d06448c0aa65a2",
	"vendor/nghttp2": "https://github.com/nghttp2/nghttp2@v1.20.0",
	"vendor/cpr": "https://github.com/whoshuu/cpr.git@52d498788be27828dc9c33740f728b6be775d83f",
	"vendor/enet": "https://github.com/freeminer/enet.git@ipv6",
	"vendor/imgui": "https://github.com/ocornut/imgui.git@670775ac22e4fb93be2335d9c4ceb50f5f9ab056"
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
