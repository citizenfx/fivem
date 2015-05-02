/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrEngine.h"

#ifdef GTA_NY
#include "CPlayerInfo.h"
#endif

#include "ResourceVFS.h"
#include "BaseResourceScripting.h"
#include "ScriptObjectRegistry.h"

ScriptObjectRegistry<fwRefContainer<VFSStream>> ScriptObjectRegistry<fwRefContainer<VFSStream>>::ms_instance;

static InitFunction initFunction([] ()
{
	using namespace rage;

#ifdef GTA_NY
	scrEngine::RegisterNativeHandler("GET_PLAYER_SERVER_ID", [] (rage::scrNativeCallContext* context)
	{
		int playerIdx = context->GetArgument<int>(0);
		CPlayerInfo* info = CPlayerInfo::GetPlayer(playerIdx);

		if (!info)
		{
			context->SetResult(0, -1);
		}
		else
		{
			context->SetResult(0, info->address.inaOnline.s_addr);
		}
	});
#endif

	scrEngine::RegisterNativeHandler("OPEN_FILE_FOR_READING", [] (rage::scrNativeCallContext* context)
	{
		const char* fileName = context->GetArgument<const char*>(0);
		auto baseEnvironment = BaseScriptEnvironment::GetCurrentEnvironment();

		auto vfsStream = ResourceVFS::OpenFileRead(fileName, ResourceIdentity(baseEnvironment->GetResource()->GetName()));

		if (!vfsStream.GetRef())
		{
			context->SetResult(0, -1);
			return;
		}

		context->SetResult(0, AddValueToRegistry(vfsStream));
	});

	scrEngine::RegisterNativeHandler("GET_LENGTH_OF_FILE", [] (rage::scrNativeCallContext* context)
	{
		uint16_t fileId = context->GetArgument<int>(0);

		auto ref = GetScriptRegistry<fwRefContainer<VFSStream>>()->GetValue(fileId);

		if (!ref.GetRef())
		{
			context->SetResult(0, 0);
			return;
		}

		context->SetResult(0, ref->GetLength());
	});

	static std::set<char*> g_validFileBuffers;

	scrEngine::RegisterNativeHandler("READ_FILE", [] (rage::scrNativeCallContext* context)
	{
		uint16_t fileId = context->GetArgument<int>(0);
		size_t length = context->GetArgument<size_t>(1);
		size_t* outRead = context->GetArgument<size_t*>(2);

		auto ref = GetScriptRegistry<fwRefContainer<VFSStream>>()->GetValue(fileId);

		if (!ref.GetRef())
		{
			context->SetResult(0, nullptr);
			return;
		}

		char* buffer = new char[length];
		size_t readLength = ref->Read(buffer, length);

		if (outRead)
		{
			*outRead = readLength;
		}

		g_validFileBuffers.insert(buffer);

		context->SetResult(0, buffer);
	});

	scrEngine::RegisterNativeHandler("FREE_FILE_BUFFER", [] (rage::scrNativeCallContext* context)
	{
		char* buffer = context->GetArgument<char*>(0);

		if (g_validFileBuffers.find(buffer) == g_validFileBuffers.end())
		{
			return;
		}

		g_validFileBuffers.erase(buffer);

		delete[] buffer;
	});

	scrEngine::RegisterNativeHandler("CLOSE_FILE", [] (rage::scrNativeCallContext* context)
	{
		uint16_t fileId = context->GetArgument<int>(0);

		auto registry = GetScriptRegistry<fwRefContainer<VFSStream>>();
		auto ref = registry->GetValue(fileId);

		if (!ref.GetRef())
		{
			context->SetResult(0, nullptr);
			return;
		}

		registry->RemoveValue(fileId);
	});
});