local gameBuilds = require("premake5_builds")

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

local PROGRAM_DETAILS = {
	['server']  = { publicName = 'Server',   cSharp = { nativesFile = 'NativesServer.cs', gameFiles = 'Server/*.cs' } },
	
	['five']    = { publicName = 'FiveM',    cSharp = { nativesFile = 'NativesFive.cs',   gameFiles = 'Client/FiveM/**.cs' } },
	['fivem']   = { publicName = 'FiveM',    cSharp = { nativesFile = 'NativesFive.cs',   gameFiles = 'Client/FiveM/**.cs' } },
	
	['rdr3']    = { publicName = 'RedM',     cSharp = { nativesFile = 'NativesRDR3.cs',   gameFiles = 'Client/RedM/*.cs' } },
	['ny']      = { publicName = 'LibertyM', cSharp = { nativesFile = 'NativesNY.cs',     gameFiles = 'Client/LibertyM/*.cs' } },
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
		"../vendor/boost-submodules/boost-context/include/",
		"../vendor/boost-submodules/boost-filesystem/include/",
		"../vendor/boost-submodules/boost-locale/include/",
		"../vendor/boost-submodules/boost-preprocessor/include/",
		"../vendor/boost-submodules/boost-program-options/include/",
		"../vendor/boost-submodules/boost-random/include/",
		"../vendor/boost-submodules/boost-system/include/",
		"../vendor/boost-submodules/boost-thread/include/",
		
		"../vendor/boost-submodules/boost-config/include/",
		"../vendor/boost-submodules/boost-any/include/",
		"../vendor/boost-submodules/boost-assert/include/",
		"../vendor/boost-submodules/boost-detail/include/",
		"../vendor/boost-submodules/boost-function/include/",
		"../vendor/boost-submodules/boost-smart-ptr/include/",
		"../vendor/boost-submodules/boost-static-assert/include/",
		"../vendor/boost-submodules/boost-throw-exception/include/",
		"../vendor/boost-submodules/boost-type-traits/include/",
		"../vendor/boost-submodules/boost-type-index/include/",
		"../vendor/boost-submodules/boost-predef/include/",
		"../vendor/boost-submodules/boost-core/include/",
		"../vendor/boost-submodules/boost-iterator/include/",
		"../vendor/boost-submodules/boost-move/include/",
		"../vendor/boost-submodules/boost-winapi/include/",
		"../vendor/boost-submodules/boost-mpl/include/",
		"../vendor/boost-submodules/boost-integer/include/",
		"../vendor/boost-submodules/boost-container-hash/include/",
		"../vendor/boost-submodules/boost-bind/include/",
		"../vendor/boost-submodules/boost-unordered/include/",
		"../vendor/boost-submodules/boost-lexical-cast/include/",
		"../vendor/boost-submodules/boost-io/include/",
		"../vendor/boost-submodules/boost-date-time/include/",
		"../vendor/boost-submodules/boost-range/include/",
		"../vendor/boost-submodules/boost-tuple/include/",
		"../vendor/boost-submodules/boost-concept-check/include/",
		"../vendor/boost-submodules/boost-utility/include/",
		"../vendor/boost-submodules/boost-numeric-conversion/include/",
		"../vendor/boost-submodules/boost-chrono/include/",
		"../vendor/boost-submodules/boost-array/include/",
		"../vendor/boost-submodules/boost-ratio/include/",
		"../vendor/boost-submodules/boost-container/include/",
		"../vendor/boost-submodules/boost-math/include/",
		"../vendor/boost-submodules/boost-tokenizer/include/",
		"../vendor/boost-submodules/boost-property-tree/include/",
		"../vendor/boost-submodules/boost-optional/include/",
		"../vendor/boost-submodules/boost-fusion/include/",
		"../vendor/boost-submodules/boost-function-types/include/",
		"../vendor/boost-submodules/boost-circular-buffer/include/",
		"../vendor/boost-submodules/boost-bimap/include/",
		"../vendor/boost-submodules/boost-algorithm/include/",
		"../vendor/boost-submodules/boost-variant/include/",
		"../vendor/boost-submodules/boost-serialization/include/",
		"../vendor/boost-submodules/boost-exception/include/",
		"../vendor/boost-submodules/boost-multi-index/include/",
		"../vendor/boost-submodules/boost-foreach/include/",
		"../vendor/boost-submodules/boost-uuid/include/",
		"../vendor/boost-submodules/boost-regex/include/",
		"../vendor/boost-submodules/boost-crc/include/",
		"../vendor/boost-submodules/boost-tti/include/",
		"../vendor/boost-submodules/boost-outcome/include/",
		"../vendor/boost-submodules/boost-coroutine/include/",
		"../vendor/boost-submodules/boost-asio/include/",
		"../vendor/boost-submodules/boost-atomic/include/",
		"../vendor/boost-submodules/boost-beast/include/",
		"../vendor/boost-submodules/boost-intrusive/include/",
		"../vendor/boost-submodules/boost-iostreams/include/",
		"../vendor/boost-submodules/boost-mp11/include/",
		"../vendor/boost-submodules/boost-logic/include/",
		"../vendor/boost-submodules/boost-endian/include/",
		"../vendor/boost-submodules/boost-describe/include/",
		"../vendor/boost-submodules/boost-scope/include/",
		"../vendor/boost-submodules/boost-align/include/",
		"../vendor/boost-submodules/boost-static-string/include/",
	}

	filter { 'language:C or language:C++'}
		defines { "GTEST_HAS_PTHREAD=0", "BOOST_ALL_NO_LIB" }
 		defines { "BOOST_NULLPTR=nullptr" }
		defines { "_HAS_AUTO_PTR_ETC" } -- until boost gets fixed
		defines { "_PPLTASK_ASYNC_LOGGING=0"}

	filter {}

	libdirs { "deplibs/lib/" }

	location ((_OPTIONS['builddir'] or "build/") .. _OPTIONS['game'])
	
	cppdialect "C++17"

	if os.istarget('windows') then
		if _OPTIONS['game'] ~= 'server' then
			buildoptions { '/await', '/d2FH4-' }
		end

		buildoptions { '/Zc:preprocessor' }

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


	local buildsDef = "GAME_BUILDS="
	local builds = gameBuilds[_OPTIONS["game"]]

	if builds ~= nil then
		local buildsOrdered = {}

		for n in pairs(builds) do table.insert(buildsOrdered, n) end
		table.sort(buildsOrdered)

		for _, build in ipairs(buildsOrdered) do
			buildsDef = buildsDef .. "(" .. string.sub(build, string.len("game_") + 1) .. ")"
		end

		filter 'language:C or language:C++'
			defines(buildsDef)
	end

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
			defines(buildsDef .. "(0)")
	else
		filter 'language:C or language:C++'
			defines(buildsDef .. "(0)")
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

	include 'tests'
	
	if os.istarget('windows') then
		include 'premake5_layout.lua'
	end

	-- TARGET: corert
	include 'client/citicore'

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
	
	if _cfg.project.name:find('^CitizenFX.') ~= nil then
		_premake.w('<DocumentationFile>$(OutputPath)ref\\' .. _cfg.targetname .. '.xml</DocumentationFile>')
	elseif _cfg.project.name == 'CitiMono' then
		_premake.w('<DocumentationFile>' .. string.gsub(_cfg.buildtarget.relpath, ".dll", ".xml") .. '</DocumentationFile>')
	end
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
		_p(2, '<GenAPIAdditionalParameters>--exclude-api-list "%s" --exclude-attributes-list "%s"</GenAPIAdditionalParameters>', 
			path.getabsolute("client/clrref/exclude_list.txt"),
			path.getabsolute("client/clrref/exclude_attributes_list.txt")
		)
		_p(2, '<GenerateReferenceAssemblySource>true</GenerateReferenceAssemblySource>')
		_p(1, '</PropertyGroup>')
		
		_p(1, '<Import Project="$(ProjectDir)\\packages\\Microsoft.DotNet.GenAPI.6.0.0-beta.21063.5\\build\\Microsoft.DotNet.GenAPI.targets" />')

		_p(1, '<Target Name="CreateReferenceAssemblyDirectory" BeforeTargets="GenerateReferenceAssemblySource">')
		_p(2, '<MakeDir Directories="$(GenAPITargetDir)" />')
		_p(1, '</Target>')
		
	elseif prj.name == 'CitiMonoRef' then
		_p(1, '<Import Project="%s" />', path.getabsolute("client/clrref/GenFacades.targets"))
		
	elseif prj.name:find('^CitizenFX.') ~= nil then
		_p(1, '<PropertyGroup>')
		_p(2, '<ProduceReferenceAssembly>true</ProduceReferenceAssembly>')
		_p(1, '</PropertyGroup>')
		_p(1, '<Target Name="CopyFilesToOutputDirectory">')
		_p(2, '<!-- .exe/.dll -->')
		_p(2, '<Copy SourceFiles="@(IntermediateAssembly)" DestinationFolder="$(OutDir)" SkipUnchangedFiles="$(SkipCopyUnchangedFiles)" OverwriteReadOnlyFiles="$(OverwriteReadOnlyFiles)" Retries="$(CopyRetryCount)" RetryDelayMilliseconds="$(CopyRetryDelayMilliseconds)" UseHardlinksIfPossible="$(CreateHardLinksForCopyFilesToOutputDirectoryIfPossible)" UseSymboliclinksIfPossible="$(CreateSymbolicLinksForCopyFilesToOutputDirectoryIfPossible)" ErrorIfLinkFails="$(ErrorIfLinkFailsForCopyFilesToOutputDirectory)" Condition="\'$(SkipCopyBuildProduct)\' != \'true\'">')
		_p(3, '<Output TaskParameter="DestinationFiles" ItemName="MainAssembly" />')
		_p(3, '<Output TaskParameter="DestinationFiles" ItemName="FileWrites" />')
		_p(2, '</Copy>')
		_p(2, '<!-- .pdb -->')
		_p(2, '<Copy SourceFiles="@(_DebugSymbolsIntermediatePath)" DestinationFiles="@(_DebugSymbolsOutputPath)" SkipUnchangedFiles="$(SkipCopyUnchangedFiles)" OverwriteReadOnlyFiles="$(OverwriteReadOnlyFiles)" Retries="$(CopyRetryCount)" RetryDelayMilliseconds="$(CopyRetryDelayMilliseconds)" UseHardlinksIfPossible="$(CreateHardLinksForCopyFilesToOutputDirectoryIfPossible)" UseSymboliclinksIfPossible="$(CreateSymbolicLinksForCopyFilesToOutputDirectoryIfPossible)" ErrorIfLinkFails="$(ErrorIfLinkFailsForCopyFilesToOutputDirectory)" Condition="\'$(_DebugSymbolsProduced)\'==\'true\' and \'$(SkipCopyingSymbolsToOutputDirectory)\' != \'true\'">')
		_p(3, '<Output TaskParameter="DestinationFiles" ItemName="FileWrites" />')
		_p(2, '</Copy>')
		_p(2, '<!-- reference assembly -->')
		_p(2, '<CopyRefAssembly SourcePath="@(IntermediateRefAssembly)" DestinationPath="$(OutDir)ref\\$(TargetName).dll" Condition="\'$(ProduceReferenceAssembly)\' == \'true\' and \'$(SkipCopyBuildProduct)\' != \'true\'">')
		_p(3, '<Output TaskParameter="DestinationPath" ItemName="ReferenceAssembly" />')
		_p(3, '<Output TaskParameter="DestinationPath" ItemName="FileWrites" />')
		_p(2, '</CopyRefAssembly>')
		_p(1, '</Target>')
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

	function csproject(projectName, targetName)
		do project(projectName)
			targetname(targetName or projectName)
			language 'C#'
			kind 'SharedLib'
			
			disablewarnings 'CS1591' -- Missing XML comment for publicly visible type or member
			dotnetframework '4.6'
			buildoptions '/debug:portable /langversion:7.3'
			csversion '7.3'
			
			links {
				'System.dll',
				'System.Core.dll',
				'Microsoft.CSharp.dll',
				'../data/client/citizen/clr2/lib/mono/4.5/System.Reflection.Metadata.dll',
				'../data/client/citizen/clr2/lib/mono/4.5/MsgPack.dll'
			}
			
			if os.istarget('windows') then
				defines { 'OS_WIN' }
			elseif os.istarget('linux') then
				defines { 'OS_LINUX' }
			end
		end
	end
	
	function cstargets(targetDirectory)
		filter { "configurations:Debug" }
			targetdir (binroot .. '/debug/citizen/clr2/lib/mono/4.5/'.. (targetDirectory or ''))
		
		filter { "configurations:Release" }
			targetdir (binroot .. '/release/citizen/clr2/lib/mono/4.5/'.. (targetDirectory or ''))
	end
			
	group "mono/v1"
	
	do csproject ("CitiMono", "CitizenFX.Core")
		clr 'Unsafe'
		files { 'client/clrcore/*.cs', 'client/clrcore/Math/*.cs' }
		files { '../vendor/ben-demystifier/src/Ben.Demystifier/**.cs' }
		
		if _OPTIONS['game'] == 'server' then
			files { 'client/clrcore/Server/*.cs' }
		else
			if _OPTIONS['game'] == 'five' then
				files { 'client/clrcore/External/*.cs' }
				files { 'client/clrcore-v2/Game/Shared/*.cs' }
			end
			
			defines { 'USE_HYPERDRIVE' }
		end
	
		if os.istarget('windows') then		
			dependson 'CfxPrebuild'
			nuget { 'Microsoft.DotNet.GenAPI:6.0.0-beta.21063.5', 'Microsoft.DotNet.GenFacades:6.0.0-beta.21063.5' }
			nugetsource 'https://pkgs.dev.azure.com/dnceng/public/_packaging/dotnet-eng/nuget/v3/index.json'
		elseif os.istarget('linux') then
			links { '/usr/lib/mono/4.5/Facades/System.Runtime.dll', '/usr/lib/mono/4.5/Facades/System.IO.dll' }
			postbuildcommands {
				("%s '%s' '%s' '%%{cfg.linktarget.abspath}'"):format(
					path.getabsolute("client/clrref/genapi.sh"),
					path.getabsolute("."),
					path.getabsolute("client/clrref/" .. _OPTIONS['game'])
				)
			}
		end
		
		links { '../data/client/citizen/clr2/lib/mono/4.5/System.Collections.Immutable.dll' }
		cstargets ''
	end
	
	local csharpCoreReferenceDllName = _OPTIONS['game'] == 'server' and "CitizenFX.Core.Server" or "CitizenFX.Core.Client"
	
	-- reference assembly project
	if os.istarget('windows') then
		do csproject ("CitiMonoRef", csharpCoreReferenceDllName)
			clr 'Unsafe'
			files { 'client/clrref/' .. _OPTIONS['game'] .. '/CitizenFX.Core.cs' }
			links { 'System.dll', 'System.Drawing.dll', 'System.Core.dll' }
			dependson 'CitiMono'
			cstargets 'ref/'
		end
	elseif os.istarget('linux') then
		do project "CitiMonoRef"
			kind 'Utility'
			files { 'client/clrref/CitizenFX.Core.Server.csproj' }
			dependson 'CitiMono'
			
			local absMonoRoot = path.getabsolute(binroot) .. '/%{cfg.name:lower()}/citizen/clr2/lib/mono/4.5/'
			
			filter 'files:**.csproj'
				buildmessage 'Generating facades'
				buildinputs { 'client/clrref/server/CitizenFX.Core.cs' }
				buildcommands {
					("dotnet msbuild '%%{file.abspath}' /p:Configuration=%%{cfg.name} /p:FacadeOutPath=%s /t:Restore"):format(
						absMonoRoot
					),
					("dotnet msbuild '%%{file.abspath}' /p:Configuration=%%{cfg.name} /p:FacadeOutPath=%s"):format(
						absMonoRoot
					),
					("rm -rf '%s'"):format(
						absMonoRoot .. 'ref/'
					)
				}
				buildoutputs { binroot .. '/%{cfg.name:lower()}/citizen/clr2/lib/mono/4.5/CitizenFX.Core.Server.dll' }
		end
	end
	
	if _OPTIONS['game'] ~= 'server' then
		do csproject ("CitiMonoSystemDrawingStub", "System.Drawing")
			files { 'client/clrref/System.Drawing.cs' }
			links { 'CitiMono' }
			cstargets ''
		end
	end
	
	group "mono/v2"
	
	local csharpCoreAssemblyName = "CitizenFX.Core"
	
	-- Get game name and files
	-- '.*' should not load any file, replace it with something else if it does
	local program = PROGRAM_DETAILS[_OPTIONS['game']]
	if not program then
		print('Program "'.._OPTIONS['game']..'" was not found in the PROGRAM_DETAILS list, you either provided an incorrect program or PROGRAM_DETAILS is missing this options, premake5.lua')
		program = { publicName = 'Unknown', cSharp = { nativesFile = '.*', gameFiles = '.*' } }
	end
	
	do csproject (csharpCoreAssemblyName)
		clr 'Unsafe'
		files {
			'client/clrcore-v2/*.cs',
			'client/clrcore-v2/Coroutine/**.cs',
			'client/clrcore-v2/Interop/**.cs',
			'client/clrcore-v2/Math/v2/**.cs',
			'client/clrcore-v2/Native/**.cs',
			'client/clrcore-v2/Shared/**.cs',
			'client/clrcore-v2/System/**.cs',
			
			-- Math, cherry pick from v1 files for now
			'client/clrcore-v2/Math/GameMath.cs',
			'client/clrcore-v2/Math/MathUtil.cs',
			'client/clrcore-v2/Math/Matrix.cs',
			'client/clrcore-v2/Math/Matrix3x3.cs',
			'client/clrcore-v2/Math/Quaternion.cs',
			'client/clrcore-v2/Math/Vector2.cs',
			'client/clrcore-v2/Math/Vector3.cs',
			'client/clrcore-v2/Math/Vector4.cs',
		}
		
		defines { 'MONO_V2' }
		
		do  -- prev. disabled on certain games
			defines { 'NATIVE_SHARED_INCLUDE' }
		end
		
		cstargets 'v2'
	end
	
	if _OPTIONS['game'] ~= 'server' then
		do csproject ("CitizenFX."..program.publicName..".NativeImpl")
			clr 'Unsafe'
			files { 'client/clrcore-v2/Native/'..program.cSharp.nativesFile }
			links { csharpCoreAssemblyName }
			defines { 'MONO_V2', 'NATIVE_WRAPPER_USE_VERSION', 'NATIVE_IMPL_INCLUDE' }
			cstargets 'v2'
		end
		
		do csproject ("CitizenFX."..program.publicName..".Native")
			files { 'client/clrcore-v2/Native/'..program.cSharp.nativesFile, 'client/clrcore-v2/Native/CustomNativeWrapper.cs' }
			links { csharpCoreAssemblyName, 'CitizenFX.'..program.publicName..'.NativeImpl' }
			defines { 'MONO_V2', 'NATIVE_WRAPPER_USE_VERSION', 'NATIVE_HASHES_INCLUDE', 'NATIVE_WRAPPERS_INCLUDE' }
			cstargets 'v2/Native/'
		end
	end
	
	do -- prev. disabled on certain games
		do csproject ("CitizenFX."..program.publicName)
			clr 'Unsafe'
			files { 'client/clrcore-v2/'..program.cSharp.gameFiles, 'client/clrcore-v2/Game/Shared/*.cs' }
			links { csharpCoreAssemblyName }
			defines { 'MONO_V2' }
			
			if _OPTIONS['game'] == 'server' then
				files { 'client/clrcore-v2/Native/'..program.cSharp.nativesFile, 'client/clrcore-v2/Native/CustomNativeWrapper.cs' }
				defines { 'NATIVE_IMPL_INCLUDE', 'NATIVE_HASHES_INCLUDE', 'NATIVE_WRAPPERS_INCLUDE' }
			else
				links { 'CitizenFX.'..program.publicName..'.Native' } -- isn't separated in server environments
			end
			
			cstargets 'v2'
		end
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
