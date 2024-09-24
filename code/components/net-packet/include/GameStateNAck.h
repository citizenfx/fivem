#pragma once

#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "SerializableStorageType.h"

namespace net::packet
{
class ClientGameStateNAck : public SerializableComponent
{
public:
	enum class Flags: uint8_t
	{
		IsMissingFrames = 1 << 0,
		HasIgnoreList = 1 << 1,
		HasRecreateList = 1 << 2,
		HasIgnoreListEntryResendFrame = 1 << 3,
	};

#ifdef _WIN32
#define PACK_STRUCT
#else
#define PACK_STRUCT __attribute__ ((packed))
#endif

#pragma pack(push, 1)
	struct PACK_STRUCT IgnoreListEntry
	{
		uint16_t entry;
		uint64_t lastFrame;

		IgnoreListEntry() = default;

		IgnoreListEntry(const uint16_t entry, const uint64_t last_frame)
			: entry(entry),
			  lastFrame(last_frame)
		{
		}
	};
#pragma pack(pop)

private:
	SerializableProperty<uint8_t> flags {static_cast<uint8_t>(Flags::HasIgnoreListEntryResendFrame)};
	SerializableProperty<uint64_t> frameIndex;
	SerializableProperty<uint64_t> firstMissingFrame;
	SerializableProperty<uint64_t> lastMissingFrame;
	SerializableProperty<Span<IgnoreListEntry>, storage_type::SmallBytesArray> ignoreList;
	SerializableProperty<Span<uint16_t>, storage_type::SmallBytesArray> recreateList;

public:
	template<typename T>
	bool Process(T& stream)
	{
		if (!flags.Process(stream))
		{
			return false;
		}

		if (!frameIndex.Process(stream))
		{
			return false;
		}

		if (net::force_consteval<bool, T::kType == DataStream::Type::Counter> || flags.GetValue() & static_cast<uint8_t>(Flags::IsMissingFrames))
		{
			if (!firstMissingFrame.Process(stream))
			{
				return false;
			}

			if (!lastMissingFrame.Process(stream))
			{
				return false;
			}
		}

		if (net::force_consteval<bool, T::kType == DataStream::Type::Counter> || flags.GetValue() & static_cast<uint8_t>(Flags::HasIgnoreList))
		{
			if (!ignoreList.Process(stream))
			{
				return false;
			}
		}

		if (net::force_consteval<bool, T::kType == DataStream::Type::Counter> || flags.GetValue() & static_cast<uint8_t>(Flags::HasRecreateList))
		{
			if (!recreateList.Process(stream))
			{
				return false;
			}
		}

		return true;
	}

	void SetFrameIndex(const uint64_t frameIndexValue)
	{
		frameIndex = frameIndexValue;
	}

	void SetIsMissingFrames(const bool isMissingFrames, const uint64_t firstMissingFrameValue, const uint64_t lastMissingFrameValue)
	{
		if (isMissingFrames)
		{
			flags.m_value |= static_cast<uint8_t>(Flags::IsMissingFrames);
			firstMissingFrame.SetValue(firstMissingFrameValue);
			lastMissingFrame.SetValue(lastMissingFrameValue);
		}
	}

	void SetIgnoreList(std::vector<IgnoreListEntry>& ignoreListValue)
	{
		if (!ignoreListValue.empty())
		{
			flags.m_value |= static_cast<uint8_t>(Flags::HasIgnoreList);
			ignoreList.SetValue({ ignoreListValue.data(), ignoreListValue.size() });
		}
	}

	void SetRecreateList(std::vector<uint16_t>& recreateListValue)
	{
		if (!recreateListValue.empty())
		{
			flags.m_value |= static_cast<uint8_t>(Flags::HasRecreateList);
			recreateList.SetValue({ recreateListValue.data(), recreateListValue.size() });
		}
	}

	uint8_t GetFlags() const
	{
		return flags;
	}

	Span<IgnoreListEntry>& GetIgnoreList()
	{
		return ignoreList.GetValue();
	}

	Span<uint16_t>& GetRecreateList()
	{
		return recreateList.GetValue();
	}

	uint64_t GetFirstMissingFrame() const
	{
		if (flags.GetValue() & static_cast<uint8_t>(Flags::IsMissingFrames))
		{
			return firstMissingFrame;
		}

		return 0;
	}

	uint64_t GetLastMissingFrame() const
	{
		if (flags.GetValue() & static_cast<uint8_t>(Flags::IsMissingFrames))
		{
			return lastMissingFrame;
		}

		return 0;
	}

	uint64_t GetFrameIndex() const
	{
		return frameIndex;
	}
};

/// <summary>
/// Information from the client to the server to acknowledge the game state packets.
/// Only used when OneSync is enabled.
/// </summary>
class ClientGameStateNAckPacket : public SerializableComponent
{
public:
	SerializableProperty<uint32_t> type{ HashRageString("gameStateNAck") };
	ClientGameStateNAck data;

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
