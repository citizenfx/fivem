﻿#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
/// <summary>
/// The client request data for requesting object ids is empty
/// </summary>
class ClientRequestObjectIds : public net::SerializableComponent
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream
		);
	}
};

/// <summary>
/// Request from the client to the server for requesting new object ids
/// </summary>
class ClientRequestObjectIdsPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{net::force_consteval<uint32_t, HashRageString("msgRequestObjectIds")>};

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
	SerializableProperty<Span<IdEntry>, storage_type::ConstrainedBytesArray<0, BigMode ? 6 : 64>> ids;

	void SetIds(const std::vector<uint16_t>& idsToSet, std::vector<IdEntry>& container)
	{
		// compress and send
		// adapted from https://stackoverflow.com/a/1081776
		int32_t last = -1;
		for (size_t i = 0; i < idsToSet.size();)
		{
			uint16_t gap = idsToSet[i] - 2 - last;
			uint16_t size = 0;

			while (++i < idsToSet.size())
			{
				if (idsToSet[i] != idsToSet[i - 1] + 1)
				{
					break;
				}

				size++;
			}

			last = idsToSet[i - 1];

			container.emplace_back(gap, size);
		}

		ids.SetValue({container.data(), container.size()});
	}

	void GetIds(std::vector<uint16_t>& idsToRead)
	{
		uint16_t last = 0;
		for (const auto& idEntry : ids.GetValue())
		{
			last += idEntry.gap + 1;

			for (uint16_t j = 0; j <= idEntry.size; j++)
			{
				uint16_t objectId = last++;
				idsToRead.push_back(objectId);
			}
		}
	}

	template<class Func>
	void ForEachId(const Func&& callback)
	{
		uint16_t last = 0;
		for (const auto& idEntry : ids.GetValue())
		{
			last += idEntry.gap + 1;

			for (uint16_t j = 0; j <= idEntry.size; j++)
			{
				uint16_t objectId = last++;
				callback(objectId);
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
	SerializableProperty<uint32_t> type{net::force_consteval<uint32_t, HashRageString("msgObjectIds")>};
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
