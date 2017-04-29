/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <Error.h>
#include "ConsoleHost.h"
#include "ConsoleHostImpl.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>

#include <mutex>
#include <queue>

#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

static bool g_conHostInitialized = false;
static MonoDomain* g_rootDomain;
static MonoMethod* g_sendMessageMethod;

static std::mutex g_traceBufferMutex;
static std::stringstream g_traceBuffer;

HANDLE g_keyWaitSemaphore;

void OutputExceptionDetails(MonoObject* exc)
{
	MonoClass* eclass = mono_object_get_class(exc);

	if (eclass)
	{
		/*MonoProperty* prop = mono_class_get_property_from_name(eclass, "Message");
		MonoMethod* getter = mono_property_get_get_method(prop);
		MonoString* msg = (MonoString*)mono_runtime_invoke(getter, exc, NULL, NULL);*/

		MonoString* msg = (MonoString*)mono_object_to_string(exc, nullptr);

		GlobalError("Unhandled exception in console host: %s", mono_string_to_utf8(msg));
	}
}

void ConHost_Run()
{
	g_keyWaitSemaphore = CreateSemaphore(nullptr, 0, INT_MAX, nullptr);

	// init Mono
	mono_set_dirs("citizen/clr/lib/", "citizen/clr/cfg/");

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::string clrPath = converter.to_bytes(MakeRelativeCitPath(L"citizen/clr/lib/"));

	std::wstring citizenClrLibPath = MakeRelativeCitPath(L"citizen/clr/lib/mono/4.5/");

	SetEnvironmentVariable(L"MONO_PATH", citizenClrLibPath.c_str());

	mono_assembly_setrootdir(clrPath.c_str());

	g_rootDomain = mono_jit_init_version("ConHost", "v4.0.30319");

	char* args[1];
	args[0] = "--soft-breakpoints";

	mono_jit_parse_options(1, args);

	ConHost_AddInternalCalls();

	// load the main assembly
	std::string filePath = converter.to_bytes(MakeRelativeCitPath(L"bin/System.Management.Automation.dll"));

	MonoAssembly* monoAssembly = mono_domain_assembly_open(g_rootDomain, filePath.c_str());

	if (!monoAssembly)
	{
		return;
	}

	MonoImage* monoImage = mono_assembly_get_image(monoAssembly);

	// load a side assembly
	filePath = converter.to_bytes(MakeRelativeCitPath(L"bin/Microsoft.PowerShell.Commands.Utility.dll"));

	mono_domain_assembly_open(g_rootDomain, filePath.c_str());

	// find the main method
	bool methodSearchSuccess = true;
	MonoMethodDesc* description;

#define method_search(name, method) description = mono_method_desc_new(name, 1); \
			method = mono_method_desc_search_in_image(description, monoImage); \
			mono_method_desc_free(description); \
			methodSearchSuccess = methodSearchSuccess && method != NULL

	MonoMethod* runMethod;
	method_search("CitizenFX.UI.Application:Run", runMethod);
	method_search("CitizenFX.UI.Application:SendMessage", g_sendMessageMethod);

	if (!methodSearchSuccess)
	{
		trace("Couldn't find one or more CitizenFX.UI methods.\n");
		return;
	}

	g_conHostInitialized = true;

	void* runArgs[1];

	{
		std::unique_lock<std::mutex> lock(g_traceBufferMutex);

		std::string runString = g_traceBuffer.str();
		g_traceBuffer.str("");

		runArgs[0] = mono_string_new_len(g_rootDomain, runString.data(), runString.size());
	}

	MonoObject* exc = nullptr;
	mono_runtime_invoke(runMethod, nullptr, runArgs, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);
	}
}

struct ConsoleKeyEvent
{
	uint32_t vKey;
	wchar_t character;
	ConsoleModifiers modifiers;
};

std::mutex g_keyUseMutex;
std::queue<ConsoleKeyEvent> g_keyEventQueue;

void ConHost_KeyEnter(uint32_t vKey, wchar_t character, ConsoleModifiers modifiers)
{
	ConsoleKeyEvent event;
	event.vKey = vKey;
	event.character = character;
	event.modifiers = modifiers;

	g_keyUseMutex.lock();
	g_keyEventQueue.push(event);
	g_keyUseMutex.unlock();

	ReleaseSemaphore(g_keyWaitSemaphore, 1, nullptr);
}

void ConHost_WaitForKey(uint32_t& vKey, wchar_t& character, ConsoleModifiers& modifiers)
{
	WaitForSingleObject(g_keyWaitSemaphore, INFINITE);

	g_keyUseMutex.lock();

	auto event = g_keyEventQueue.front();
	g_keyEventQueue.pop();

	g_keyUseMutex.unlock();

	vKey = event.vKey;
	character = event.character;
	modifiers = event.modifiers;
}

struct MonoAttachment
{
	MonoThread* thread;

	MonoAttachment()
		: thread(nullptr)
	{
		if (!mono_domain_get())
		{
			thread = mono_thread_attach(g_rootDomain);
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

void ConHost_SendMessage(MonoString* string)
{
	static thread_local MonoAttachment monoAttachment;
	
	void* runArgs[1];
	runArgs[0] = string;

	MonoObject* exc = nullptr;
	mono_runtime_invoke(g_sendMessageMethod, nullptr, runArgs, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);
	}
}

void SendPrintMessage(const std::string& message)
{
	ConHost_SendMessage(mono_string_new_len(g_rootDomain, message.c_str(), message.size()));
}

void ConHost::Print(int channel, const std::string& message)
{
	if (!g_conHostInitialized)
	{
		std::unique_lock<std::mutex> lock(g_traceBufferMutex);
		g_traceBuffer << message;

		return;
	}

	SendPrintMessage(message);
}