#include <StdInc.h>
#include "MonoScriptEnvironment.h"
#include "fiDevice.h"
#include "NetLibrary.h"

struct FileIOReference
{
	rage::fiDevice* device;
	uint32_t handle;
};

void* GI_OpenFileCall(MonoString* str)
{
	std::string fileName = mono_string_to_utf8(str);

	rage::fiDevice* device = rage::fiDevice::GetDevice(fileName.c_str(), true);

	if (!device)
	{
		return nullptr;
	}

	auto ref = new FileIOReference;
	ref->device = device;
	ref->handle = device->open(fileName.c_str(), true);

	if (ref->handle == -1)
	{
		delete ref;

		return nullptr;
	}

	return ref;
}

int GI_ReadFileCall(FileIOReference* fileHandle, MonoArray* buffer, int offset, int length)
{
	int arrayLength = mono_array_length(buffer);

	if ((offset + length) > arrayLength)
	{
		return 0;
	}

	char* buf = mono_array_addr(buffer, char, offset);
	int len = fileHandle->device->read(fileHandle->handle, buf, length);

	return len;
}

void GI_CloseFileCall(FileIOReference* reference)
{
	reference->device->close(reference->handle);

	delete reference;
}

int GI_GetFileLengthCall(FileIOReference* reference)
{
	return reference->device->fileLength(reference->handle);
}

int GI_GetEnvironmentInfoCall(MonoString** resourceName, MonoString** resourcePath, MonoString** resourceAssembly, uint32_t* instanceId)
{
	auto env = dynamic_cast<MonoScriptEnvironment*>(BaseScriptEnvironment::GetCurrentEnvironment());

	if (!env)
	{
		*resourceName = mono_string_new(mono_domain_get(), "");
		*resourcePath = mono_string_new(mono_domain_get(), "");
		*resourceAssembly = mono_string_new(mono_domain_get(), "");

		return false;
	}
	
	*resourceName = mono_string_new(mono_domain_get(), env->GetResource()->GetName().c_str());
	*resourcePath = mono_string_new(mono_domain_get(), env->GetResource()->GetPath().c_str());

	auto& metaData = env->GetResource()->GetMetaData();
	auto it = metaData.find("assembly");

	if (it == metaData.end())
	{
		*resourceAssembly = mono_string_new(mono_domain_get(), "Assembly.dll");
	}
	else
	{
		*resourceAssembly = mono_string_new(mono_domain_get(), it->second.c_str());
	}

	*instanceId = env->GetInstanceId();

	return true;
}

void GI_PrintLogCall(MonoString* str)
{
	trace("%s", mono_string_to_utf8(str));
}

fwRefContainer<Resource> ValidateResourceAndRef(int reference, int instance, fwString resourceName)
{
	auto resource = TheResources.GetResource(resourceName);

	if (!resource.GetRef())
	{
		return nullptr;
	}

	if (!resource->HasRef(reference, instance))
	{
		return nullptr;
	}

	return resource;
}

MonoArray* GI_InvokeNativeReferenceCall(MonoString* resourceStr, uint32_t instance, uint32_t reference, MonoArray* inArgs)
{
	fwString resourceName = mono_string_to_utf8(resourceStr);
	auto resource = ValidateResourceAndRef(reference, instance, resourceName);

	if (!resource.GetRef())
	{
		return nullptr;
	}

	char* inArgsStart = mono_array_addr(inArgs, char, 0);
	uintptr_t inArgsLength = mono_array_length(inArgs);

	// prepare input args
	fwString argsSerialized(inArgsStart, inArgsLength);

	fwString returnArgs = resource->CallRef(reference, instance, argsSerialized);

	// prepare return array
	MonoArray* argsArray = mono_array_new(mono_domain_get(), mono_get_byte_class(), returnArgs.size());

	char* argsAddr = mono_array_addr(argsArray, char, 0);
	memcpy(argsAddr, returnArgs.c_str(), returnArgs.size());

	// and return said array
	return argsArray;
}

void GI_DeleteNativeReferenceCall(MonoString* resourceStr, uint32_t instance, uint32_t reference)
{
	fwString resourceName = mono_string_to_utf8(resourceStr);
	auto resource = ValidateResourceAndRef(reference, instance, resourceName);

	if (!resource.GetRef())
	{
		return;
	}

	resource->RemoveRef(reference, instance);
}

static NetLibrary* g_netLibrary;

void GI_TriggerEventCall(MonoString* eventNameStr, MonoArray* inArgs, int isRemote)
{
	fwString eventName = mono_string_to_utf8(eventNameStr);

	char* inArgsStart = mono_array_addr(inArgs, char, 0);
	uintptr_t inArgsLength = mono_array_length(inArgs);

	// prepare args
	fwString argsSerialized(inArgsStart, inArgsLength);

	if (isRemote)
	{
		g_netLibrary->SendNetEvent(eventName, argsSerialized, -2);
	}
	else
	{
		TheResources.TriggerEvent(eventName, argsSerialized);
	}
}

MonoArray* GI_InvokeResourceExportCall(MonoString* resourceStr, MonoString* exportStr, MonoArray* inArgs)
{
	fwString resourceName = mono_string_to_utf8(resourceStr);
	fwString exportName = mono_string_to_utf8(exportStr);

	auto resource = TheResources.GetResource(resourceName);

	if (!resource.GetRef())
	{
		return nullptr;
	}

	if (!resource->HasExport(exportName))
	{
		return nullptr;
	}

	char* inArgsStart = mono_array_addr(inArgs, char, 0);
	uintptr_t inArgsLength = mono_array_length(inArgs);

	// prepare input args
	fwString argsSerialized(inArgsStart, inArgsLength);

	fwString returnArgs = resource->CallExport(exportName, argsSerialized);

	// prepare return array
	MonoArray* argsArray = mono_array_new(mono_domain_get(), mono_get_byte_class(), returnArgs.size());

	char* argsAddr = mono_array_addr(argsArray, char, 0);
	memcpy(argsAddr, returnArgs.c_str(), returnArgs.size());

	// and return said array
	return argsArray;
}

void MonoAddInternalCalls()
{
	mono_add_internal_call("CitizenFX.Core.GameInterface::OpenFile", GI_OpenFileCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::ReadFile", GI_ReadFileCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::CloseFile", GI_CloseFileCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::GetFileLength", GI_GetFileLengthCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::GetEnvironmentInfo", GI_GetEnvironmentInfoCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::PrintLog", GI_PrintLogCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::InvokeNativeReference", GI_InvokeNativeReferenceCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::DeleteNativeReference", GI_DeleteNativeReferenceCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::TriggerEvent", GI_TriggerEventCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::InvokeResourceExport", GI_InvokeResourceExportCall);
}

static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* library)
	{
		g_netLibrary = library;
	});
});