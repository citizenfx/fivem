/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>

#include <VFSManager.h>

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_RESOURCE_METADATA", [] (fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(0);
			return;
		}

		// get the metadata component
		fwRefContainer<fx::ResourceMetaDataComponent> component = resource->GetComponent<fx::ResourceMetaDataComponent>();

		// find all matching entries
		auto entries = component->GetEntries(context.GetArgument<const char*>(1));

		// and count them
		auto numEntries = std::distance(entries.begin(), entries.end());

		context.SetResult(numEntries);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_METADATA", [] (fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		// get the metadata component
		fwRefContainer<fx::ResourceMetaDataComponent> component = resource->GetComponent<fx::ResourceMetaDataComponent>();

		// find matching entries
		auto entries = component->GetEntries(context.GetArgument<const char*>(1));
		int entryIndex = context.GetArgument<int>(2);

		// and loop over the entries to see if we find anything
		int i = 0;
		const char* retval = nullptr;

		for (auto& entry : entries)
		{
			if (entryIndex == i)
			{
				retval = entry.second.c_str();
				break;
			}

			i++;
		}

		context.SetResult(retval);
	});

	fx::ScriptEngine::RegisterNativeHandler("LOAD_RESOURCE_FILE", [] (fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		// try opening the file from the resource's home directory
		fwRefContainer<vfs::Stream> stream = vfs::OpenRead(resource->GetPath() + "/" + context.GetArgument<const char*>(1));

		if (!stream.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		// static, so it will persist until the next call
		static std::vector<uint8_t> returnedArray;
		returnedArray = stream->ReadToEnd();
		returnedArray.push_back(0); // zero-terminate

		context.SetResult(&returnedArray[0]);
	});

#ifdef IS_FXSERVER
	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_PATH", [](fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		context.SetResult(resource->GetPath().c_str());
	});

	fx::ScriptEngine::RegisterNativeHandler("SAVE_RESOURCE_FILE", [](fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		// try opening a writable file in the resource's home directory
		std::string filePath = resource->GetPath() + "/" + context.GetArgument<const char*>(1);

		fwRefContainer<vfs::Device> device = vfs::GetDevice(filePath);
		auto handle = device->Create(filePath);

		if (handle == vfs::Device::InvalidHandle)
		{
			context.SetResult(false);
			return;
		}

		// create a stream
		fwRefContainer<vfs::Stream> stream(new vfs::Stream(device, handle));

		// get data + length
		const char* data = context.CheckArgument<const char*>(2);
		int length = context.GetArgument<int>(3);

		if (length == 0 || length == -1)
		{
			length = strlen(data);
		}

		stream->Write(data, length);

		context.SetResult(true);
	});
#endif
});
