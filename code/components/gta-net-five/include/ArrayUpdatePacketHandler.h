#pragma once
#include "ArrayHandler.h"
#include "ArrayUpdate.h"
#include "PacketHandler.h"

extern rage::netArrayManager** g_arrayManager;
extern std::unordered_map<uint16_t, CNetGamePlayer*> g_playersByNetId;
extern std::array<std::unique_ptr<ArrayHandlerInfo>, 20> arrayHandlers;

namespace fx
{
class ArrayUpdatePacketHandler : public net::PacketHandler<net::packet::ServerArrayUpdate, HashRageString("msgArrayUpdate")>
{
	static auto GetArrayHandlerInfo(const int index, rage::netArrayHandlerBase* handler)
	{
		if (!arrayHandlers[index])
		{
			arrayHandlers[index] = std::make_unique<ArrayHandlerInfo>();
			arrayHandlers[index]->hashes.resize(handler->m_count);
		}

		return arrayHandlers[index].get();
	}

public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerArrayUpdate& serverArrayUpdate)
		{
			if (!*g_arrayManager)
			{
				return;
			}

			uint8_t arrayIndex = serverArrayUpdate.handler;
			uint16_t player = serverArrayUpdate.ownerNetId;
			uint32_t element = serverArrayUpdate.index;
			size_t length = serverArrayUpdate.data.GetValue().size();

			auto arrayHandler = (*g_arrayManager)->GetArrayHandler(arrayIndex, nullptr);

			if (!arrayHandler)
			{
				return;
			}

			auto playerData = g_playersByNetId[player];

			if (!playerData)
			{
				return;
			}

			if (length)
			{
				rage::datBitBuffer buffer(serverArrayUpdate.data.GetValue().data(), serverArrayUpdate.data.GetValue().size());
				buffer.m_f1C = 1;

				arrayHandler->ReadElement(buffer, element, nullptr);

				// apply
				if (arrayHandler->CanApplyElementData(element, *playerData, true))
				{
					arrayHandler->ApplyElementData(element, *playerData);
				}
			}
			else
			{
				arrayHandler->SetElementEmpty(element);
			}

			arrayHandler->DoPostElementReadProcessing(element);
			arrayHandler->DoPostReadProcessing();

			// update hashes
			auto info = GetArrayHandlerInfo(arrayIndex, arrayHandler);

			{
				// write element
				static thread_local std::vector<uint8_t> data(1024);
				memset(data.data(), 0, data.size());

				rage::datBitBuffer buffer(data.data(), data.size());
				arrayHandler->WriteElement(buffer, element, nullptr);

				std::string_view sv{ reinterpret_cast<char*>(data.data()), (buffer.m_curBit / 8) + 1 };

				auto thisHash = std::hash<std::string_view>()(sv);
				info->hashes[element] = thisHash;
			}
		});
	}
};
}
