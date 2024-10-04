#pragma once

#include <StdInc.h>

#include <string>
#include <string_view>
#include <unordered_map>

#include <MonoComponentHostShared.h>

#include "MonoScriptRuntime.h"
#include "MonoMethods.h"

#include <mono/metadata/object.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/appdomain.h>

#ifndef IS_FXSERVER
#include <scrEngine.h>
typedef rage::scrEngine::NativeHandler NativeHandler;
typedef rage::scrNativeCallContext ScriptContext;
#else
#include <ScriptEngine.h>
typedef fx::TNativeHandler* NativeHandler;
typedef fx::ScriptContext ScriptContext;
#endif

namespace fx::mono
{
class MonoComponentHost : public MonoComponentHostShared
{
public:
	enum class AssemblyOverrideRule : uint8_t
	{
		FULL_VERSION_EQUAL,

		MAJOR_VERSION_EQUAL,
		MAJOR_VERSION_GREATER_OR_EQUAL,
		MAJOR_VERSION_LESSER_OR_EQUAL
	};

	typedef std::tuple<MonoAssembly*, std::array<uint16_t, 4>, AssemblyOverrideRule> AssemblyOverride;

private:
	static MonoDomain* s_rootDomain;
	static MonoAssembly* s_rootAssembly;
	static MonoImage* s_rootImage;

	static std::unordered_map<std::string, std::vector<AssemblyOverride>> s_overrideAssemblies;

	static fwRefContainer<fx::ProfilerComponent> GetProfiler()
	{
		static auto s_profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
		return s_profiler;
	}

public:
	// Initializes the v2 environment, must be called before making and using script runtimes
	static void Initialize();

	// Client only, loads /v2/ native wrapper and adds it to the overrides
	static void InitializeNativeWrapper(const std::string& platformFolder);

	// Ensures that Mono is attached to the current thread
	static void EnsureThreadAttached();

	// Get v2 root domain
	static MonoDomain* GetRootDomain();

#pragma region Assembly related

public:
	// Get v2 core assembly
	static MonoAssembly* GetRootAssembly();
	
	// Get v2 core image
	static MonoImage* GetRootImage();

	// Get all assembly overrides by the given name, will be nullptr if it's not present
	static const std::vector<AssemblyOverride>* GetAssemblyOverrides(std::string_view assemblyName);

private:
	// Load assembly contents, used for UGC scripts
	static bool ReadAssemblyUGC(MonoScriptRuntime* runtime, MonoString* name, MonoArray** assembly, MonoArray** symbols);

	/// Loads an assembly directly
	static MonoAssembly* LoadAssemblyDirect(std::string_view path);

	// Hook to override assembly loading
	static MonoAssembly* AssemblyResolve(MonoAssemblyName* assemblyName, void* userData);

	// Add an assembly that'll take precedence when mono tries to load them.
	// `version` and `rule` are present for future implementations, for now only version 2.x.x.x assemblies are checked without taking the `rule` into account
	static void AddAssemblyOverride(std::string_view assemblyName, MonoAssembly* assembly, const std::array<uint16_t, 4>& version, AssemblyOverrideRule rule);

#pragma endregion

#pragma region Profiler

public:
	static bool ProfilerIsRecording();

	static void ProfilerEnterScope(MonoString* name);

	static void ProfilerExitScope();

#pragma endregion

#pragma region Natives

private:
	static void InvokeNative(NativeHandler native, ScriptContext* context, uint64_t hash);

	static uintptr_t GetNative(uint64_t hash);

#pragma endregion

private:
	static MonoArray* CanonicalizeRef(const MonoScriptRuntime* runTime, int referenceId);

	static MonoArray* InvokeFunctionReference(const MonoScriptRuntime* runTime, MonoString* referenceId, MonoArray* argsSerialized);

private:
	static void InitializeMethods(MonoImage* image);
};

inline MonoDomain* MonoComponentHost::GetRootDomain()
{
	return s_rootDomain;
}

inline MonoAssembly* MonoComponentHost::GetRootAssembly()
{
	return s_rootAssembly;
}

inline MonoImage* MonoComponentHost::GetRootImage()
{
	return s_rootImage;
}

inline bool MonoComponentHost::ProfilerIsRecording()
{
	return GetProfiler()->IsRecording();
}

inline void MonoComponentHost::ProfilerEnterScope(MonoString* name)
{
	char* cName = mono_string_to_utf8(name);
	GetProfiler()->EnterScope(cName);
	mono_free(cName);
}

inline void MonoComponentHost::ProfilerExitScope()
{
	GetProfiler()->ExitScope();
}

inline bool MonoComponentHost::ReadAssemblyUGC(MonoScriptRuntime* runtime, MonoString* name, MonoArray** assembly, MonoArray** symbols)
{
	return runtime->ReadAssembly(name, assembly, symbols);
}

inline const std::vector<MonoComponentHost::AssemblyOverride>* MonoComponentHost::GetAssemblyOverrides(std::string_view assemblyName)
{
	auto found = s_overrideAssemblies.find(std::string(assemblyName));
	return found != s_overrideAssemblies.end() ? &found->second : nullptr;
}

inline void MonoComponentHost::AddAssemblyOverride(std::string_view assemblyName, MonoAssembly* assembly,
	const std::array<uint16_t, 4>& version, MonoComponentHost::AssemblyOverrideRule rule)
{
	s_overrideAssemblies[std::string(assemblyName)].emplace_back(assembly, version, rule);
}

inline MonoArray* MonoComponentHost::CanonicalizeRef(const MonoScriptRuntime* runTime, int referenceId)
{
	return runTime->CanonicalizeRef(referenceId);
}

inline MonoArray* MonoComponentHost::InvokeFunctionReference(const MonoScriptRuntime* runTime, MonoString* referenceId, MonoArray* argsSerialized)
{
	return runTime->InvokeFunctionReference(referenceId, argsSerialized);
}
}
