/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#include <shared_mutex>

#include "MonoComponentHost.h"

#include <om/OMComponent.h>
#include <ResourceManager.h>
#include <fxScripting.h>

#include <CoreConsole.h>
#include <Error.h>
#include <Profiler.h>

#include <msgpack.hpp>

#include <mono/metadata/exception.h>
#include <mono/metadata/threads.h>

#define CITIZENFX_CORE "CitizenFX.Core"
#define CITIZENFX_GAME_NATIVE "CitizenFX." PRODUCT_NAME ".Native"

namespace fx::mono
{
MonoDomain* MonoComponentHost::s_rootDomain = nullptr;
MonoAssembly* MonoComponentHost::s_rootAssembly = nullptr;
MonoImage* MonoComponentHost::s_rootImage = nullptr;

decltype(MonoComponentHost::s_overrideAssemblies) MonoComponentHost::s_overrideAssemblies;

void MonoComponentHost::Initialize()
{
	MonoComponentHostShared::Initialize();
	mono_install_assembly_search_hook(AssemblyResolve, nullptr);

	mono_thread_attach(mono_get_root_domain());
	s_rootDomain = mono_domain_create_appdomain(const_cast<char*>("Mono-V2"), const_cast<char*>("cfx.config")); // should've been const qualified

	// TODO: PERF: see which of these can be replaced by mono_dangerous_add_raw_internal_call("full method name", func*) and/or made suitable for,
	// ^ requires newer version of mono, which client isn't on

	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::CFree", free);
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::Print", MonoComponentHost::Print);

	//Method::AddInternalCall("CitizenFX.Core.ScriptInterface::GetMemoryUsage", MonoComponentHost::GetMemoryUsage);
	//Method::AddInternalCall("CitizenFX.Core.ScriptInterface::WalkStackBoundary", MonoComponentHostShared::WalkStackBoundary);
	//Method::AddInternalCall("CitizenFX.Core.ScriptInterface::SnapshotStackBoundary", MonoComponentHostShared::SnapshotStackBoundary);

	// profiling
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::ProfilerIsRecording", MonoComponentHost::ProfilerIsRecording);
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::ProfilerEnterScope", MonoComponentHost::ProfilerEnterScope);
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::ProfilerExitScope", MonoComponentHost::ProfilerExitScope);

	// game native invoking for scripts
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::GetNative", MonoComponentHost::GetNative);
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::InvokeNative", MonoComponentHost::InvokeNative);

	// reference functions
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::CanonicalizeRef", MonoComponentHost::CanonicalizeRef);
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::InvokeFunctionReference", MonoComponentHost::InvokeFunctionReference);

	// assemblies
	Method::AddInternalCall("CitizenFX.Core.ScriptInterface::ReadAssembly", MonoComponentHost::ReadAssemblyUGC);

	// load core assembly
	std::string platformFolder = MakeRelativeNarrowPath("citizen/clr2/lib/mono/4.5/v2/");
	auto scriptManagerAssembly = mono_domain_assembly_open(s_rootDomain, (platformFolder + CITIZENFX_CORE ".dll").c_str());
	if (!scriptManagerAssembly)
		FatalError("Could not load v2/" CITIZENFX_CORE ".dll.\n");

	s_rootAssembly = scriptManagerAssembly;
	MonoImage* rootImage = s_rootImage = mono_assembly_get_image(scriptManagerAssembly);
	AddAssemblyOverride(CITIZENFX_CORE, scriptManagerAssembly, { 2, 0, 0, 0 }, MonoComponentHost::AssemblyOverrideRule::MAJOR_VERSION_EQUAL);

	InitializeNativeWrapper(platformFolder);

	// all functions required for this host to interface with C#, may also include vice-versa functions
	const char* requiredMethods[] {
		"CitizenFX.Core.ScriptInterface:Initialize",
		"CitizenFX.Core.ScriptInterface:LoadAssembly",
		"CitizenFX.Core.ScriptInterface:Tick",
		"CitizenFX.Core.ScriptInterface:TriggerEvent",

		"CitizenFX.Core.ScriptInterface:CallRef",
		"CitizenFX.Core.ScriptInterface:DuplicateRef",
		"CitizenFX.Core.ScriptInterface:RemoveRef"
	};

	// make sure all methods we'll be using are present for later, else we'll early out
	for (const char* method : requiredMethods)
	{
		if (!Method::Find(rootImage, method))
		{
			FatalError(CITIZENFX_CORE " method missing\nCouldn't find one or more methods, Try and update.\n\nMissing: %s", method);
			return;
		}
	}

	mono_domain_set_internal(mono_get_root_domain());

	// BACK_TO_PREV_DOMAIN
}

void MonoComponentHost::InitializeNativeWrapper(const std::string& platformFolder)
{
#ifndef IS_FXSERVER
	// native implementation
	{
		std::string nativeAssemblyPath = platformFolder + CITIZENFX_GAME_NATIVE "Impl.dll";
		auto nativeWrappersAssembly = mono_domain_assembly_open(s_rootDomain, nativeAssemblyPath.c_str());
		if (!nativeWrappersAssembly)
			FatalError("Could not load v2/" CITIZENFX_GAME_NATIVE "Impl.dll.\n");

		AddAssemblyOverride(CITIZENFX_GAME_NATIVE "Impl", nativeWrappersAssembly, { 2, 0, 0, 0 }, MonoComponentHost::AssemblyOverrideRule::MAJOR_VERSION_EQUAL);
	}

	// native wrappers
	{
	std::string nativeAssemblyPath = platformFolder + "Native/" CITIZENFX_GAME_NATIVE ".dll";
		auto nativeWrappersAssembly = mono_domain_assembly_open(s_rootDomain, nativeAssemblyPath.c_str());
		if (!nativeWrappersAssembly)
			FatalError("Could not load v2/Native/" CITIZENFX_GAME_NATIVE ".dll.\n");

		MonoAssemblyName* name = mono_assembly_get_name(nativeWrappersAssembly);
		std::array<uint16_t, 4> version;
		version[0] = mono_assembly_name_get_version(name, &version[1], &version[2], &version[3]);

		AddAssemblyOverride(CITIZENFX_GAME_NATIVE, nativeWrappersAssembly, version, MonoComponentHost::AssemblyOverrideRule::FULL_VERSION_EQUAL);

		// used for fallback natives
		AddAssemblyOverride(CITIZENFX_GAME_NATIVE, nativeWrappersAssembly, { 2, 65535, 65535, 65535 }, MonoComponentHost::AssemblyOverrideRule::FULL_VERSION_EQUAL);
	}
#endif
}

MonoAssembly* MonoComponentHost::AssemblyResolve(MonoAssemblyName* assemblyName, void* userData)
{
	std::array<uint16_t, 4> version;
	version[0] = mono_assembly_name_get_version(assemblyName, &version[1], &version[2], &version[3]);
	const char* name = mono_assembly_name_get_name(assemblyName);

	MonoDomain* curDomain = mono_domain_get();

	//trace("Searching for assembly: %s (%d.%d.%d.%d)\n", name, version[0], version[1], version[2], version[3]);

	// only checking major version 2 assemblies and "CitizenFX.*" names for an early out, until more are required
	if (version[0] == uint16_t(2u) && strncmp(name, "CitizenFX.", 10) == 0)
	{
		std::string strAssemblyName(name);
		auto found = s_overrideAssemblies.find(strAssemblyName);
		if (found != s_overrideAssemblies.end())
		{
			for (auto& overrideAssembly : found->second)
			{
				// already loaded, check if we should return it
				const auto& loadedAssembly = std::get<0>(overrideAssembly);
				const auto& loadedVersion = std::get<1>(overrideAssembly);
				const auto& condition = std::get<2>(overrideAssembly);

				switch (condition)
				{
					case AssemblyOverrideRule::FULL_VERSION_EQUAL:
						if (loadedVersion == version)
							return loadedAssembly;
						break;
					case AssemblyOverrideRule::MAJOR_VERSION_EQUAL:
						if (loadedVersion[0] == version[0])
							return loadedAssembly;
						break;
					case AssemblyOverrideRule::MAJOR_VERSION_GREATER_OR_EQUAL:
						if (loadedVersion[0] >= version[0])
							return loadedAssembly;
						break;
					case AssemblyOverrideRule::MAJOR_VERSION_LESSER_OR_EQUAL:
						if (loadedVersion[0] <= version[0])
							return loadedAssembly;
						break;
				}
			}
		}
	}

	// non found, let C# Assembly resolver figure it out
	return nullptr;
}

MonoAssembly* MonoComponentHost::LoadAssemblyDirect(std::string_view path)
{
	MonoImageOpenStatus status;
	MonoImage* image = mono_image_open(path.data(), &status);
	if (image)
	{
		if (status == MonoImageOpenStatus::MONO_IMAGE_OK)
		{
			MonoAssembly* assembly = mono_assembly_load_from(image, path.data(), &status);
			// MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status); // TODO: figure out if we need to use this variant instead
			if (assembly)
				return assembly;
		}

		mono_image_close(image);
	}

	return nullptr;
}

void MonoComponentHost::InvokeNative(NativeHandler native, ScriptContext* context, uint64_t hash)
{
	try
	{
#ifdef IS_FXSERVER
		(*native)(*context);
#else
		native(context);
#endif
	}
	catch (const std::exception& e)
	{
		std::string error = fmt::sprintf("Error executing native 0x%016llx at address %p, exception: %s", hash, (void*)native, e.what());
		mono_raise_exception(mono_get_exception_invalid_operation(error.c_str()));
	}
}

uintptr_t MonoComponentHost::GetNative(uint64_t hash)
{
#ifdef IS_FXSERVER
	auto* native = fx::ScriptEngine::GetNativeHandlerPtr(hash);
	if (native != nullptr)
		return reinterpret_cast<uintptr_t>(native);

	std::string error = fmt::sprintf("Error acquiring native 0x%016llx, no such native found", hash);
	mono_raise_exception(mono_get_exception_invalid_operation(error.c_str()));

	return 0;
#else
	return reinterpret_cast<uintptr_t>(rage::scrEngine::GetNativeHandler(hash));
#endif
}

void MonoComponentHost::EnsureThreadAttached()
{
	struct MonoAttachment
	{
		MonoThread* thread;

		MonoAttachment()
			: thread(nullptr)
		{
			if (!mono_domain_get())
			{
				thread = mono_thread_attach(s_rootDomain);
			}
		}

		~MonoAttachment()
		{
			if (thread)
			{
				mono_thread_detach(thread);
				thread = nullptr;
			}
		}
	};

	if (MonoComponentHost::s_rootDomain)
	{
		static thread_local MonoAttachment attachment;
	}
}
}

static InitFunction initFunction([] ()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* instance)
	{
		instance->OnTick.Connect(fx::mono::MonoComponentHost::EnsureThreadAttached, INT32_MIN);
	});

	fx::mono::MonoComponentHost::Initialize();
});
