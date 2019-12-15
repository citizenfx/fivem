-- to work around slow init times due to packagesrv.com being down
premake.downloadModule = function()
	return false
end

xpcall(function()
newoption {
	trigger 	= "with-asan",
	value       = "libpath",
	description = "Use asan for Windows."
}

newoption {
	trigger     = "builddir",
	value       = "path",
	description = "Output directory for build/ files."
}

newoption {
	trigger     = "bindir",
	value       = "path",
	description = "Root directory for bin/ files."
}

dofile('tools/build/init.lua')

-- perform component processing
if _OPTIONS['game'] == 'dummy' then
	return
end

root_cwd = os.getcwd()

-- initialize components
dofile('components/config.lua')
dofile('vendor/config.lua')

load_privates('privates_config.lua')

component
{
	name = 'platform:' .. os.target(),
	rawName = os.target(),
	vendor = { dummy = true }
}

workspace "CitizenMP"
	configurations { "Debug", "Release" }

	symbols "On"
	characterset "Unicode"

	flags { "No64BitChecks" }

	flags { "NoIncrementalLink", "NoMinimalRebuild" } -- this breaks our custom section ordering in citilaunch, and is kind of annoying otherwise

	editandcontinue 'Off'

	includedirs {
		"shared/",
		"client/shared/",
		"../vendor/jitasm/",
		"../vendor/rapidjson/include/",
		"../vendor/fmtlib/include/",
		"deplibs/include/",
		os.getenv("BOOST_ROOT")
	}

	defines { "GTEST_HAS_PTHREAD=0", "BOOST_ALL_NO_LIB" }

	defines { "_HAS_AUTO_PTR_ETC" } -- until boost gets fixed

	libdirs { "deplibs/lib/" }

	location ((_OPTIONS['builddir'] or "build/") .. _OPTIONS['game'])

	if os.istarget('windows') then
		buildoptions '/std:c++17'
		
		if _OPTIONS['game'] ~= 'server' then
			buildoptions '/await'
		end

		systemversion '10.0.18362.0'
	end

	-- special build dirs for FXServer
	if _OPTIONS['game'] == 'server' then
		location ((_OPTIONS['builddir'] or "build/") .. "server/" .. os.target())
		architecture 'x64'
		defines 'IS_FXSERVER'
	end

	local binroot = ((_OPTIONS['bindir'] or "bin/") .. _OPTIONS['game']) .. '/'

	if _OPTIONS['game'] == 'server' then
		binroot = (_OPTIONS['bindir'] or "bin/") .. 'server/' .. os.target() .. '/'
	end
	
	if _OPTIONS['with-asan'] then
		toolset 'msc-llvm'
		
		libdirs { _OPTIONS['with-asan'] }
		
		links { 'clang_rt.asan_dynamic-x86_64', 'clang_rt.asan_dynamic_runtime_thunk-x86_64' }
		
		filter 'language:C or language:C++'
			buildoptions '-mpclmul -maes -mssse3 -mavx2 -mrtm'
			buildoptions '-fsanitize=address -fsanitize-recover=address'
	end

	-- debug output
	configuration "Debug*"
		targetdir (binroot .. "/debug")
		defines "NDEBUG"

		-- this slows down the application a lot
		defines { '_ITERATOR_DEBUG_LEVEL=0' }

		-- allow one level of inlining
		if os.istarget('windows') then
			buildoptions '/Ob1'
		end

	-- release output
	configuration "Release*"
		targetdir (binroot .. "/release")
		defines "NDEBUG"
		optimize "Speed"

	configuration "game=five"
		defines "GTA_FIVE"

		filter 'language:C or language:C++'
			architecture 'x64'
			
	configuration "game=rdr3"
		defines "IS_RDR3"

		filter 'language:C or language:C++'
			architecture 'x64'
			
	configuration "game=launcher"
		defines "IS_LAUNCHER"

		filter 'language:C or language:C++'
			architecture 'x64'

	configuration "windows"
		links { "winmm" }

	filter { 'system:not windows', 'language:C or language:C++' }
		architecture 'x64'
		
		links { 'stdc++' }

		buildoptions {
			"-fPIC", -- required to link on AMD64
		}

	-- TARGET: launcher
	if _OPTIONS['game'] ~= 'server' then
		-- game launcher
		include 'client/launcher'
		include 'client/console'
		include 'client/diag'
	else
		include 'server/launcher'
	end

	-- TARGET: corert
	include 'client/citicore'

if _OPTIONS['game'] ~= 'server' then
	include 'client/ipfsdl'

	project "CitiGame"
		targetname "CitizenGame"
		language "C++"
		kind "SharedLib"

		includedirs { 'client/citicore/' }

		files
		{
			"client/common/Error.cpp",
			"client/citigame/Launcher.cpp",
			"client/common/StdInc.cpp"
		}

		links { "Shared", "citicore" }

		defines "COMPILING_GAME"

		pchsource "client/common/StdInc.cpp"
		pchheader "StdInc.h"
end

require 'vstudio'

premake.override(premake.vstudio.dotnetbase, 'debugProps', function(base, cfg)
	if cfg.symbols == premake.ON then
		_p(2,'<DebugSymbols>true</DebugSymbols>')
	end
	_p(2,'<DebugType>portable</DebugType>')
	_p(2,'<Optimize>%s</Optimize>', iif(premake.config.isOptimizedBuild(cfg), "true", "false"))
end)

premake.override(premake.vstudio.vc2010, 'importLanguageTargets', function(base, prj)
	base(prj)

	local hasPostBuild = false

	for cfg in premake.project.eachconfig(prj) do
		if cfg.postbuildcommands and #cfg.postbuildcommands > 0 then
			hasPostBuild = true
			break
		end
	end

	if hasPostBuild then
		_p(1, '<Target Name="DisablePostBuildEvent" AfterTargets="Link" BeforeTargets="PostBuildEvent">')
		_p(2, '<PropertyGroup>')
		_p(3, '<PostBuildEventUseInBuild Condition="\'$(LinkSkippedExecution)\' == \'True\'">false</PostBuildEventUseInBuild>')
		_p(2, '</PropertyGroup>')
		_p(1, '</Target>')
	end
end)

local function WriteDocumentationFileXml(_premake, _cfg)
	if _cfg.project.name == 'CitiMonoSystemDrawingStub' then
		_premake.w(('<AssemblyOriginatorKeyFile>%s</AssemblyOriginatorKeyFile>'):format(
			path.getabsolute("client/clrref/msft.snk")
		))
		_premake.w('<SignAssembly>true</SignAssembly>')
		_premake.w('<DelaySign>true</DelaySign>')
	
		return
	end

	if _cfg.project.name ~= 'CitiMono' then
		return
	end
	
    _premake.w('<DocumentationFile>' .. string.gsub(_cfg.buildtarget.relpath, ".dll", ".xml") .. '</DocumentationFile>')
end

premake.override(premake.vstudio.dotnetbase, "compilerProps", function(base, cfg)
    base(cfg)
    WriteDocumentationFileXml(premake, cfg)

    premake.w('<GenerateTargetFrameworkAttribute>false</GenerateTargetFrameworkAttribute>')
end)

premake.override(premake.vstudio.cs2005, "targets", function(base, prj)
    base(prj)
    
    if prj.name == 'CitiMono' then
		_p(1, '<PropertyGroup>')
		_p(2, '<GenAPITargetDir>%s/</GenAPITargetDir>', path.getabsolute("client/clrref/"))
		_p(2, '<GenAPIAdditionalParameters>%s</GenAPIAdditionalParameters>', ('-excludeApiList:"%s" -excludeAttributesList:"%s"'):format(
			path.getabsolute("client/clrref/exclude_list.txt"),
			path.getabsolute("client/clrref/exclude_attributes_list.txt")
		))
		_p(2, '<GenerateReferenceAssemblySources>true</GenerateReferenceAssemblySources>')
		_p(1, '</PropertyGroup>')
		
		_p(1, '<Import Project="%s" />', path.getabsolute("client/clrcore/GenAPI.targets"))
    end
end)

premake.override(premake.vstudio.nuget2010, "supportsPackageReferences", function(base, prj)
	-- <PackageReference /> doesn't work for GenAPI (even if fixing `nuget.config` issue for source)
	return false
end)

premake.override(premake.vstudio.dotnetbase, "nuGetReferences", function(base, prj)
	-- and this'll fail as GenAPI doesn't have any lib/.../*.dll file
	if prj.name == 'CitiMono' then
		return
	end
	
	return base(prj)
end)

if _OPTIONS['game'] ~= 'launcher' then
	project "CitiMono"
		targetname "CitizenFX.Core"
		language "C#"
		kind "SharedLib"

		-- Missing XML comment for publicly visible type or member
		disablewarnings 'CS1591'
		
		dotnetframework '4.6'
		
		clr 'Unsafe'
		
		csversion '7.3'

		files { "client/clrcore/*.cs", "client/clrcore/Math/*.cs" }
		
		files { "../vendor/ben-demystifier/src/Ben.Demystifier/**.cs" }
		
		if _OPTIONS['game'] ~= 'server' then
			defines { 'USE_HYPERDRIVE' }
			
			if _OPTIONS['game'] == 'five' then
				files { "client/clrcore/External/*.cs" }
			end
		else
			files { "client/clrcore/Server/*.cs" }
		end
		
		if os.istarget('windows') then
			nuget { "Microsoft.DotNet.BuildTools.GenAPI:3.0.0-preview1-03805-01", "Microsoft.DotNet.BuildTools.GenFacades:3.0.0-preview1-03805-01" }
			nugetsource "https://dotnet.myget.org/F/dotnet-buildtools/api/v3/index.json"
			
			
		end

		links {
			"System.dll",
			"Microsoft.CSharp.dll",
			"System.Core.dll",
			"../data/client/citizen/clr2/lib/mono/4.5/System.Reflection.Metadata.dll",
			"../data/client/citizen/clr2/lib/mono/4.5/System.Collections.Immutable.dll",
			"../data/client/citizen/clr2/lib/mono/4.5/MsgPack.dll"
		}

		if os.istarget('linux') then
			links {
				"/usr/lib/mono/4.5/Facades/System.Runtime.dll",
				"/usr/lib/mono/4.5/Facades/System.IO.dll"
			}
		end

		buildoptions '/debug:portable /langversion:7.3'

		configuration "Debug*"
			targetdir (binroot .. '/debug/citizen/clr2/lib/mono/4.5/')

		configuration "Release*"
			targetdir (binroot .. '/release/citizen/clr2/lib/mono/4.5/')
			
	if _OPTIONS['game'] ~= 'server' then
		project "CitiMonoSystemDrawingStub"
			targetname "System.Drawing"
			language "C#"
			kind "SharedLib"
			
			links { "CitiMono" }
			
			files {
				"client/clrref/System.Drawing.cs"
			}
			
			configuration "Debug*"
				targetdir (binroot .. '/debug/citizen/clr2/lib/mono/4.5/')

			configuration "Release*"
				targetdir (binroot .. '/release/citizen/clr2/lib/mono/4.5/')
	end
			
	if os.istarget('windows') then
		project "CitiMonoRef"
			if _OPTIONS['game'] == 'server' then
				targetname "CitizenFX.Core.Server"
			else
				targetname "CitizenFX.Core.Client"
			end
			
			language "C#"
			kind "SharedLib"
			
			dependson "CitiMono"
			
			dotnetframework '4.6'
			clr 'Unsafe'
			csversion '7.3'
			
			links { "System.dll", "System.Drawing.dll" }
			
			files { "client/clrref/CitizenFX.Core.cs" }
			
			buildoptions '/debug:portable /langversion:7.3'
			
			postbuildcommands {
				('copy /y "%s" "%s"'):format(
					"$(TargetDir)..\\CitizenFX.Core.xml",
					"$(TargetDir)$(TargetName).xml"
				),				
				('"%s" -facadePath:"%s" -seeds:"%s" -contracts:"%s"'):format(
					"$(SolutionDir)\\packages\\Microsoft.DotNet.BuildTools.GenFacades.3.0.0-preview1-03805-01\\tools\\GenFacades.exe",
					"$(TargetDir)..",
					"$(TargetDir)..\\CitizenFX.Core.dll",
					"$(TargetPath)"
				)
			}
			
			configuration "Debug*"
				targetdir (binroot .. '/debug/citizen/clr2/lib/mono/4.5/ref/')

			configuration "Release*"
				targetdir (binroot .. '/release/citizen/clr2/lib/mono/4.5/ref/')
	end
end
	group ""

	-- TARGET: shared component
	include "client/shared"

	group "vendor"

if _OPTIONS['game'] ~= 'server' then
	include "tools/dbg"

	project "libcef_dll"
		targetname "libcef_dll_wrapper"
		language "C++"
		kind "StaticLib"

		defines { "USING_CEF_SHARED", "NOMINMAX", "WIN32", "WRAPPING_CEF_SHARED" }

		flags { "NoIncrementalLink", "NoMinimalRebuild" }

		includedirs { ".", "../vendor/cef" }

		buildoptions "/MP"

		files
		{
			"../vendor/cef/libcef_dll/**.cc",
			"../vendor/cef/libcef_dll/**.cpp",
			"../vendor/cef/libcef_dll/**.h"
		}
end

	-- run components
	group "components"
		do_components()

	-- vendor is last so it can trigger even if it were merely tagged
	group "vendor"
		do_vendor()
end, function(e)
	print(e)
	print(debug.traceback())

	os.exit(1)
end)
