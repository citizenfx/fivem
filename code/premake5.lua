-- to work around slow init times due to packagesrv.com being down
premake.downloadModule = function()
	return false
end

_G._ROOTPATH = path.getabsolute('.')

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
load_privates('privates_config.lua')
dofile('vendor/config.lua')

component
{
	name = 'platform:' .. os.target(),
	rawName = os.target(),
	vendor = { dummy = true }
}

workspace "CitizenMP"
	configurations { "Debug", "Release" }

	symbols "Full"
	characterset "Unicode"

	flags { "No64BitChecks" }

	flags { "NoIncrementalLink", "NoMinimalRebuild" } -- this breaks our custom section ordering in citilaunch, and is kind of annoying otherwise

	editandcontinue 'Off'
	justmycode 'Off'

	includedirs {
		"shared/",
		"client/shared/",
		"../vendor/jitasm/",
		"../vendor/rapidjson/include/",
		"../vendor/fmtlib/include/",
		"deplibs/include/",
		os.getenv("BOOST_ROOT")
	}

	filter { 'language:C or language:C++'}
		defines { "GTEST_HAS_PTHREAD=0", "BOOST_ALL_NO_LIB" }

		defines { "_HAS_AUTO_PTR_ETC" } -- until boost gets fixed

	filter {}

	libdirs { "deplibs/lib/" }

	location ((_OPTIONS['builddir'] or "build/") .. _OPTIONS['game'])
	
	cppdialect "C++17"

	if os.istarget('windows') then
		if _OPTIONS['game'] ~= 'server' then
			buildoptions { '/await', '/d2FH4-' }
		end

		systemversion '10.0.22000.0'
	else
		vectorextensions 'SSSE3'
	end

	-- special build dirs for FXServer
	if _OPTIONS['game'] == 'server' then
		location ((_OPTIONS['builddir'] or "build/") .. "server/" .. os.target())
		architecture 'x64'
		defines 'IS_FXSERVER'
		startproject 'DuplicityMain'
	else
		startproject 'CitiLaunch'
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
	
	filter { 'action:vs*' }
		implibdir "$(IntDir)/lib/"
		symbolspath "$(TargetDir)dbg/$(TargetName).pdb"
		
	filter {}

	-- debug output
	filter { "configurations:Debug" }
		targetdir (binroot .. "/debug")

		-- allow one level of inlining
		if os.istarget('windows') then
			buildoptions { '/Ob1' }
		end

	-- this slows down the application a lot
	filter { 'configurations:Debug', 'language:C or language:C++'}
		defines { '_ITERATOR_DEBUG_LEVEL=0' }

	-- release output
	filter { "configurations:Release" }
		targetdir (binroot .. "/release")
		defines "NDEBUG"
		optimize "Speed"

	filter {}

	if _OPTIONS["game"] == "ny" then
		filter { "configurations:Release*", "kind:SharedLib or kind:ConsoleApp or kind:WindowedApp" }
			linkoptions "/SAFESEH:NO"

		filter {}
			defines "GTA_NY"

			architecture 'x86'
	elseif _OPTIONS["game"] == "five" then
		defines "GTA_FIVE"

		filter 'language:C or language:C++ or language:C#'
			architecture 'x64'
	elseif _OPTIONS["game"] == "rdr3" then
		defines "IS_RDR3"

		filter 'language:C or language:C++ or language:C#'
			architecture 'x64'
	elseif _OPTIONS["game"] == "launcher" then
		defines "IS_LAUNCHER"

		filter 'language:C or language:C++ or language:C#'
			architecture 'x64'
	end

	filter { "system:windows", 'language:C or language:C++' }
		links { "winmm" }

		buildoptions { '/Zc:__cplusplus', '/utf-8' }

	filter { 'system:not windows', 'language:C or language:C++' }
		architecture 'x64'

		links { 'stdc++' }

		buildoptions {
			"-fPIC", -- required to link on AMD64
			"-fvisibility=hidden", -- default visibility
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
	
	if os.istarget('windows') then
		include 'premake5_layout.lua'
	end

	-- TARGET: corert
	include 'client/citicore'

if _OPTIONS['game'] == 'rdr3' then
	include 'client/ipfsdl'
end
	
if _OPTIONS['game'] ~= 'server' then
	project "CitiGame"
		targetname "CitizenGame"
		language "C++"
		kind "SharedLib"

		includedirs { 'client/citicore/' }

		files
		{
			"client/common/Error.cpp",
			"client/citigame/Launcher.cpp",
			"client/citigame/NvCacheWorkaround.cpp",
			"client/common/StdInc.cpp"
		}

		links { "Shared", "CitiCore" }

		add_dependencies { 'vendor:nvapi' }

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

premake.override(premake.vstudio.vc2010, 'ignoreImportLibrary', function(base, cfg)
	if cfg.flags.NoImportLib then
		premake.vstudio.vc2010.element("IgnoreImportLibrary", nil, "true")
	end
end)

premake.override(premake.vstudio.vc2010, 'buildCommands', function(base, cfg, cond)
	base(cfg, cond)

	premake.vstudio.vc2010.element("BuildInParallel", cond, "true")
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
	
	local hasPreLink = false

	for cfg in premake.project.eachconfig(prj) do
		if cfg.prelinkcommands and #cfg.prelinkcommands > 0 then
			hasPreLink = true
			break
		end
	end

	if hasPreLink then
		_p(1, '<PropertyGroup>')
		_p(2, '<PreLinkEventUseInBuild>false</PreLinkEventUseInBuild>')
		_p(1, '</PropertyGroup>')
		-- DoLinkOutputFilesMatch is right before PreLinkEvent; so it won't evaluate the condition yet
		_p(1, '<Target Name="EnablePreLinkEvent" Inputs="@(Link)" Outputs="$(ProjectDir)/$(ProjectName).res" BeforeTargets="DoLinkOutputFilesMatch">')
		-- use CreateProperty task to set property based on skipped state
		_p(2, '<CreateProperty Value="true">')
		_p(3, '<Output TaskParameter="ValueSetByTask" PropertyName="PreLinkEventUseInBuild" />')
		_p(2, '</CreateProperty>')
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

premake.override(premake.vstudio.dotnetbase, "propertyGroup", function(base, cfg)
    if cfg.project.name == 'CitiMonoRef' then
        premake.push('<PropertyGroup %s>', premake.vstudio.dotnetbase.condition(cfg))
        return
    end

    base(cfg)
end)

premake.override(premake.vstudio.dotnetbase, "compilerProps", function(base, cfg)
    base(cfg)
    WriteDocumentationFileXml(premake, cfg)

    premake.w('<GenerateTargetFrameworkAttribute>false</GenerateTargetFrameworkAttribute>')
end)

premake.override(premake.vstudio.cs2005, "referencePath", function(base, prj)
    premake.w('<ReferencePath>' .. path.getabsolute("deplibs/libs") .. '</ReferencePath>')
end)

premake.override(premake.vstudio.cs2005, "targets", function(base, prj)
    base(prj)

    if prj.name == 'CitiMono' then
		_p(1, '<PropertyGroup>')
		_p(2, '<GenAPITargetDir>%s</GenAPITargetDir>', path.getabsolute("client/clrref/" .. _OPTIONS['game']))
		_p(2, '<GenAPITargetPath>$(GenAPITargetDir)\\$(TargetName).cs</GenAPITargetPath>')
		_p(2, '<GenAPIAdditionalParameters>%s</GenAPIAdditionalParameters>', ('--exclude-api-list "%s" --exclude-attributes-list "%s"'):format(
			path.getabsolute("client/clrref/exclude_list.txt"),
			path.getabsolute("client/clrref/exclude_attributes_list.txt")
		))
		_p(2, '<GenerateReferenceAssemblySource>true</GenerateReferenceAssemblySource>')
		_p(1, '</PropertyGroup>')
		
		_p(1, '<Import Project="$(ProjectDir)\\packages\\Microsoft.DotNet.GenAPI.6.0.0-beta.21063.5\\build\\Microsoft.DotNet.GenAPI.targets" />')

		_p(1, '<Target Name="CreateReferenceAssemblyDirectory" BeforeTargets="GenerateReferenceAssemblySource">')
		_p(2, '<MakeDir Directories="$(GenAPITargetDir)" />')
		_p(1, '</Target>')
    end

	if prj.name == 'CitiMonoRef' then
		_p(1, '<Import Project="%s" />', path.getabsolute("client/clrref/GenFacades.targets"))
	end
end)

premake.override(premake.vstudio.nuget2010, "supportsPackageReferences", function(base, prj)
	-- <PackageReference /> doesn't work for GenAPI (even if fixing `nuget.config` issue for source)
	return false
end)

premake.override(premake.vstudio.dotnetbase, "nuGetReferences", function(base, cfgOrPrj)
	-- and this'll fail as GenAPI doesn't have any lib/.../*.dll file
	local prj = cfgOrPrj

	-- this changed to be a config, not a project
	-- https://github.com/premake/premake-core/commit/cd276f8971008d1f19cf25e6a19a362884ae85d0
	if prj.project then
		prj = prj.project
	end

	if prj.name == 'CitiMono' then
		return
	end

	return base(cfgOrPrj)
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
		
		if _OPTIONS['game'] == 'five' then
			files { "client/clrcore/NativesFive.cs" }
		elseif _OPTIONS['game'] == 'rdr3' then
			files { "client/clrcore/NativesRDR3.cs" }
		elseif _OPTIONS['game'] == 'server' then
			files { "client/clrcore/NativesServer.cs" }
		elseif _OPTIONS['game'] == 'ny' then
			files { "client/clrcore/NativesNY.cs" }
		end

		if _OPTIONS['game'] ~= 'server' then
			defines { 'USE_HYPERDRIVE' }

			if _OPTIONS['game'] == 'five' then
				files { "client/clrcore/External/*.cs" }
			end
		else
			files { "client/clrcore/Server/*.cs" }
		end

		if os.istarget('windows') then
			nuget {
				"Microsoft.DotNet.GenAPI:6.0.0-beta.21063.5",
				"Microsoft.DotNet.GenFacades:6.0.0-beta.21063.5",
			}
			nugetsource "https://pkgs.dev.azure.com/dnceng/public/_packaging/dotnet-eng/nuget/v3/index.json"
		else
			postbuildcommands {
				("%s '%s' '%s' '%%{cfg.linktarget.abspath}'"):format(
					path.getabsolute("client/clrref/genapi.sh"),
					path.getabsolute("."),
					path.getabsolute("client/clrref/" .. _OPTIONS['game'])
				)
			}
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

		filter { "configurations:Debug" }
			targetdir (binroot .. '/debug/citizen/clr2/lib/mono/4.5/')

		filter { "configurations:Release" }
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

			filter { "configurations:Debug" }
				targetdir (binroot .. '/debug/citizen/clr2/lib/mono/4.5/')

			filter { "configurations:Release" }
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

			links {
				"System.dll",
				"System.Drawing.dll",
				"System.Core.dll",
			}
			
			files { "client/clrref/" .. _OPTIONS['game'] .. "/CitizenFX.Core.cs" }
			
			buildoptions '/debug:portable /langversion:7.3'
			targetdir (binroot .. '/%{cfg.name:lower()}/citizen/clr2/lib/mono/4.5/ref/')
	else
		project "CitiMonoRef"
			kind "Utility"

			dependson "CitiMono"

			files {
				"client/clrref/CitizenFX.Core.Server.csproj"
			}

			filter 'files:**.csproj'
				buildmessage 'Generating facades'
				buildinputs { "client/clrref/server/CitizenFX.Core.cs" }
				buildcommands {
					("dotnet msbuild '%%{file.abspath}' /p:Configuration=%%{cfg.name} /p:FacadeOutPath=%s /t:Restore"):format(
						path.getabsolute(binroot) .. '/%{cfg.name:lower()}/citizen/clr2/lib/mono/4.5/'
					),
					("dotnet msbuild '%%{file.abspath}' /p:Configuration=%%{cfg.name} /p:FacadeOutPath=%s"):format(
						path.getabsolute(binroot) .. '/%{cfg.name:lower()}/citizen/clr2/lib/mono/4.5/'
					),
					("rm -rf '%s'"):format(
						path.getabsolute(binroot) .. '/%{cfg.name:lower()}/citizen/clr2/lib/mono/4.5/ref/'
					)
				}
				buildoutputs { binroot .. '/%{cfg.name:lower()}/citizen/clr2/lib/mono/4.5/CitizenFX.Core.Server.dll' }
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

		defines { "USING_CEF_SHARED", "NOMINMAX", "WIN32", "WRAPPING_CEF_SHARED", "DCHECK_ALWAYS_ON" }

		flags { "NoIncrementalLink", "NoMinimalRebuild" }

		local cefRoot = "../vendor/cef/"

		if _OPTIONS['game'] == 'ny' then
			cefRoot = "../vendor/cef32/"
		end

		includedirs { ".", cefRoot }

		buildoptions "/MP"

		files_project(cefRoot)
		{
			"libcef_dll/**.cc",
			"libcef_dll/**.cpp",
			"libcef_dll/**.h"
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
