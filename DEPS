vars = {
	"citidev_root": "http://tohjo.ez.lv/citidev"
}

deps = {
	"vendor/luajit": Var("citidev_root") + "/luajit.git",
	"build/premake": "http://github.com/annulen/premake.git"
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
	}
]