#include <StdInc.h>
#include <Hooking.h>

#include <rlNetBuffer.h>
#include <NetworkPlayerMgr.h>

#include <NetLibrary.h>
#include <CrossBuildRuntime.h>

#include <boost/preprocessor/repeat.hpp>

#include "ArrayHandler.h"
#include "ArrayUpdate.h"
#include "ArrayUpdatePacketHandler.h"

static hook::cdecl_stub<rage::netArrayHandlerBase*(rage::netArrayManager*, int, void*)> _getArrayHandler([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("48 8B 0D ? ? ? ? BA 06 00 00 00 45 33 C0", 0xF));
#elif IS_RDR3
	return hook::get_call((xbr::IsGameBuildOrGreater<1436>()) ? hook::get_pattern("48 8B 0D ? ? ? ? 41 8D 56 06 45 33 C0", 0xE) : hook::get_pattern("48 8B 0D ? ? ? ? BA 06 00 00 00 45 33 C0", 0xF));
#endif
});

static rage::netArrayManager** g_arrayManager;

namespace rage
{
netArrayHandlerBase* netArrayManager::GetArrayHandler(int index, void* identifier)
{
	return _getArrayHandler(this, index, identifier);
}
}

static std::array<std::unique_ptr<ArrayHandlerInfo>, 20> arrayHandlers;

static auto GetArrayHandlerInfo(int index, rage::netArrayHandlerBase* handler)
{
	if (!arrayHandlers[index])
	{
		arrayHandlers[index] = std::make_unique<ArrayHandlerInfo>();
		arrayHandlers[index]->hashes.resize(handler->m_count);
	}

	return arrayHandlers[index].get();
}

extern NetLibrary* g_netLibrary;

void ArrayManager_Update()
{
	if (!*g_arrayManager)
	{
		return;
	}

	static const int arrayHandlers[] = {
#ifdef GTA_FIVE
		4, // incidents
		7, // sticky bombs
#elif IS_RDR3
		16, // world doors
#endif
	};

	for (int arrayIndex : arrayHandlers)
	{
		auto arrayHandler = (*g_arrayManager)->GetArrayHandler(arrayIndex, nullptr);

		if (arrayHandler)
		{
			static thread_local std::vector<uint8_t> data(1024);

			auto info = GetArrayHandlerInfo(arrayIndex, arrayHandler);

			for (int elem = 0; elem < arrayHandler->m_count; elem++)
			{
				memset(data.data(), 0, data.size());

				net::Span<uint8_t> sv;

				if (!arrayHandler->IsElementEmpty(elem))
				{
					// write element
					rage::datBitBuffer buffer(data.data(), data.size());
					arrayHandler->WriteElement(buffer, elem, nullptr);

					sv = { data.data(), (buffer.m_curBit / 8) + 1 };
				}

				// check hash for equality
				auto thisHash = std::hash<std::string_view>()({reinterpret_cast<const char*>(sv.data()), sv.size()});

				if (thisHash != info->hashes[elem])
				{
					// write the array update
					net::packet::ClientArrayUpdatePacket clientArrayUpdatePacket;
					clientArrayUpdatePacket.data.handler = arrayIndex;
					clientArrayUpdatePacket.data.index = elem;
					clientArrayUpdatePacket.data.data = sv;
					g_netLibrary->SendNetPacket(clientArrayUpdatePacket);

					info->hashes[elem] = thisHash;
				}
			}
		}
	}
}

static HookFunction hookFunctionArray([]()
{
#ifdef GTA_FIVE
	g_arrayManager = hook::get_address<rage::netArrayManager**>(hook::get_pattern("48 8B 0D ? ? ? ? BA 06 00 00 00 45 33 C0", 3));
#elif IS_RDR3
	g_arrayManager = hook::get_address<rage::netArrayManager**>((xbr::IsGameBuildOrGreater<1436>()) ? hook::get_pattern("48 8B 0D ? ? ? ? 41 8D 56 06 45 33 C0", 3) : hook::get_pattern("48 8B 0D ? ? ? ? BA 06 00 00 00 45 33 C0", 3));
#endif
});

static InitFunction initFunctionArray([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		lib->AddPacketHandler<fx::ArrayUpdatePacketHandler>(false);
	});
});
