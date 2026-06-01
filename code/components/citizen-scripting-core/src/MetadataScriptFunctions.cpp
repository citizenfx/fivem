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

#include "FilesystemPermissions.h"

#ifdef IS_FXSERVER
#include <filesystem>
#endif

#if __has_include(<CrossBuildRuntime.h>) && defined(_WIN32)
#include <CrossBuildRuntime.h>

static inline auto GetGameBuild()
{
	return xbr::GetRequestedGameBuild();
}
#else
static inline auto GetGameBuild()
{
	return 0;
}
#endif

#ifdef IS_FXSERVER
static bool IsPathWithinResourceRoot(const std::filesystem::path& rootPath, const std::filesystem::path& candidatePath)
{
#ifdef _WIN32
	auto equalsPathPart = [](const std::filesystem::path& lhs, const std::filesystem::path& rhs)
	{
		auto lhsString = lhs.wstring();
		auto rhsString = rhs.wstring();

		if (lhsString.size() != rhsString.size())
		{
			return false;
		}

		for (size_t i = 0; i < lhsString.size(); ++i)
		{
			if (towlower(lhsString[i]) != towlower(rhsString[i]))
			{
				return false;
			}
		}

		return true;
	};
#else
	auto equalsPathPart = [](const std::filesystem::path& lhs, const std::filesystem::path& rhs)
	{
		return lhs == rhs;
	};
#endif

	auto rootIt = rootPath.begin();
	auto candidateIt = candidatePath.begin();

	for (; rootIt != rootPath.end(); ++rootIt, ++candidateIt)
	{
		if (candidateIt == candidatePath.end() || !equalsPathPart(*rootIt, *candidateIt))
		{
			return false;
		}
	}

	return true;
}
#endif

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_RESOURCE_METADATA", [] (fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(0);
			return;
		}

		// get the metadata component
		fwRefContainer<fx::ResourceMetaDataComponent> component = resource->GetComponent<fx::ResourceMetaDataComponent>();

		// find all matching entries
		auto entries = component->GetEntries(context.CheckArgument<const char*>(1));

		// and count them
		auto numEntries = std::distance(entries.begin(), entries.end());

		context.SetResult(numEntries);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_METADATA", [] (fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		// get the metadata component
		fwRefContainer<fx::ResourceMetaDataComponent> component = resource->GetComponent<fx::ResourceMetaDataComponent>();

		// find matching entries
		auto entries = component->GetEntries(context.CheckArgument<const char*>(1));
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
		const char* requestedFileName = context.CheckArgument<const char*>(1);

		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		const auto& rootPath = resource->GetPath();

		// an empty root path would read from `/...`
		if (rootPath.empty())
		{
			context.SetResult(nullptr);
			return;
		}

#ifndef IS_FXSERVER
		// only load from `resources:/` for client (see CachedResourceMounter)
		if (rootPath.find("resources:/") != 0) // find != 0 is equivalent to a !starts_with
		{
			context.SetResult(nullptr);
			return;
		}
#else
		try
		{
			const std::filesystem::path resourceRoot = std::filesystem::weakly_canonical(std::filesystem::absolute(std::filesystem::u8path(rootPath)));
			const std::filesystem::path requestedPath = std::filesystem::u8path(requestedFileName);

			if (requestedPath.is_absolute() || requestedPath.has_root_name())
			{
				context.SetResult(nullptr);
				return;
			}

			const std::filesystem::path absoluteRequestedPath = std::filesystem::weakly_canonical(std::filesystem::absolute(resourceRoot / requestedPath));

			if (!IsPathWithinResourceRoot(resourceRoot, absoluteRequestedPath))
			{
				context.SetResult(nullptr);
				return;
			}
		}
		catch (const std::filesystem::filesystem_error&)
		{
			context.SetResult(nullptr);
			return;
		}
#endif

		// try opening the file from the resource's home directory
		fwRefContainer<vfs::Stream> stream = vfs::OpenRead(rootPath + "/" + requestedFileName);

		if (!stream.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		// static, so it will persist until the next call
		static std::vector<uint8_t> returnedArray;
		returnedArray = stream->ReadToEnd();
		returnedArray.push_back(0); // zero-terminate

		struct scrString
		{
			const void* str;
			size_t len;
			uint32_t magic;

			scrString(const void* str, size_t len)
				: str(str), len(len), magic(0xFEED1212)
			{
			}
		};

		context.SetResult(scrString{ &returnedArray[0], returnedArray.size() - 1 });
	});

#ifdef IS_FXSERVER
	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_PATH", [](fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

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
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));
	
		if (!resource.GetRef())
		{
			context.SetResult(nullptr);
			return;
		}

		if (!fx::ScriptingFilesystemAllowWrite("@" + resource->GetName() + "/" + context.CheckArgument<const char*>(1)))
		{
			context.SetResult(nullptr);
			return;
		}

		// try opening a writable file in the resource's home directory
		const std::string filePath = resource->GetPath() + "/" + context.CheckArgument<const char*>(1);

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

	fx::ScriptEngine::RegisterNativeHandler("GET_GAME_NAME", [](fx::ScriptContext& context)
	{
		const char* gameName =
#ifdef IS_FXSERVER
		"fxserver"
#elif defined(GTA_FIVE)
		"fivem"
#elif defined(GTA_NY)
		"libertym"
#elif defined(IS_RDR3)
		"redm"
#else
		"unknown"
#endif
		;

		context.SetResult<const char*>(gameName);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_GAME_BUILD_NUMBER", [](fx::ScriptContext& context)
	{
		context.SetResult<int64_t>(GetGameBuild());
	});
});
