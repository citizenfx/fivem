#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
/// <summary>
/// Request from the client to the server for requesting new object ids
/// </summary>
class ClientRequestObjectIdsPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgRequestObjectIds") };

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			type
		);
	}
};

/// <summary>
/// Response from the server to the client for the requested object ids
/// </summary>
template<bool BigMode>
class ServerObjectIds : public SerializableComponent
{
public:
	class IdEntry : public SerializableComponent
	{
	public:
		SerializableProperty<uint16_t> gap;
		SerializableProperty<uint16_t> size;

		IdEntry() = default;
		
		IdEntry(const uint16_t gapValue, const uint16_t sizeValue)
		{
			gap = gapValue;
			size = sizeValue;
		}

		template<typename T>
		bool Process(T& stream)
		{
			return ProcessPropertiesInOrder<T>(
				stream,
				gap,
				size
			);
		}
	};

	// todo: in future net version use StreamTail
	SerializableProperty<std::vector<IdEntry>, storage_type::ConstrainedBytesArray<0, BigMode ? 6 : 32>> ids;

	void SetIds(const std::vector<int>& idsToSet)
	{
		// compress and send
		// adapted from https://stackoverflow.com/a/1081776
		int last = -1;
		for (int i = 0; i < idsToSet.size(); )
		{
			int gap = idsToSet[i] - 2 - last;
			int size = 0;

			while (++i < idsToSet.size() && idsToSet[i] == idsToSet[i - 1] + 1) size++;

			last = idsToSet[i - 1];

			ids.GetValue().emplace_back(gap, size);
		}
	}

	void GetIds(std::vector<int>& idsToRead)
	{
		int last = 0;
		for (const auto& idEntry : ids.GetValue())
		{
			last += idEntry.gap + 1;

			for (int j = 0; j <= idEntry.size; j++)
			{
				int objectId = last++;
				idsToRead.push_back(objectId);
			}
		}
	}

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			ids
		);
	}
};

/// <summary>
/// Response from the server to the client for the requested object ids
/// </summary>
template<bool BigMode>
class ServerObjectIdsPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("msgObjectIds") };
	ServerObjectIds<BigMode> data;

	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			type,
			data
		);
	}
};
}
