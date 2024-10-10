#pragma once

#include <memory>
#include <cstdint>

#include "ByteReader.h"
#include "SerializableComponent.h"
#include "SerializableProperty.h"
#include "Span.h"

namespace net
{
	class StreamByteReader
	{
		uint8_t* m_data;
		uint64_t m_capacity;
		uint64_t m_offset = 0;

		bool RequireSpan(Span<uint8_t>& data, const size_t size)
		{
			if (data.size() < size)
			{
				// when we can't require the expected size we try to read as much as possible
				if (!data.empty())
				{
					ReadSpan(data, data.size());
				}

				return false;
			}

			ReadSpan(data, size);
			return true;
		}

		void ReadSpan(Span<uint8_t>& data, const size_t size)
		{
			memcpy(m_data + m_offset, data.data(), size);
			data = Span(data.data() + size, data.size() - size);
			m_offset += size;
		}
	public:
		StreamByteReader(uint8_t* data, const uint64_t capacity): m_data(data), m_capacity(capacity)
		{
		}

		StreamByteReader(const StreamByteReader&) = delete;

		uint64_t GetRemainingDataSize() const
		{
			return m_offset;
		}

		uint64_t GetCapacity() const
		{
			return m_capacity;
		}

		const uint8_t* GetData() const
		{
			return m_data;
		}

		/// <summary>
		/// Pushes data to read.
		/// The incomplete data will be saved to the buffer.
		/// </summary>
		/// <param name="data">The new data to read.</param>
		/// <param name="completeCallback">Is called for every T type inside the stream.</param>
		/// <returns>Returns false if the stream is broken, otherwise true.</returns>
		template <typename T, class CompleteCallback>
		bool Push(Span<uint8_t>& data, const CompleteCallback&& completeCallback)
		{
			if (data.empty())
			{
				return false;
			}

			static size_t kMinSize = SerializableComponent::GetMinSize<T>();
			// check if remaining data is left in the stream
			if (m_offset > 0)
			{
				// make sure the minimum size is at least in the stream before reading a message from it
				if (m_offset < kMinSize)
				{
					if (!RequireSpan(data, kMinSize - m_offset))
					{
						return true;
					}
				}
				
				T message;
				ByteReader reader(m_data, m_offset);
				SerializableResult result = message.Process(reader);
				if (result != SerializableResult::Incomplete)
				{
					// stream is broken, because it should always be incomplete, otherwise would not be left in the stream
					return false;
				}

				const size_t requiredSize = SerializableComponent::GetSize(message);

				// should only be false if the message only requires the minimum size
				if (m_offset < requiredSize)
				{
					if (!RequireSpan(data, requiredSize - m_offset))
					{
						return true;
					}
				}
			
				ByteReader increasedReader(m_data, m_offset);
				SerializableResult increasedResult = message.Process(increasedReader);
				if (increasedResult != SerializableResult::Success)
				{
					// stream broken, data should have been complete
					return false;
				}

				m_offset -= increasedReader.GetOffset();
				completeCallback(message);
			}

			while (data.size() >= kMinSize)
			{
				T message;
				ByteReader reader(data.data(), data.size());
				const SerializableResult result = message.Process(reader);
				if (result == SerializableResult::Incomplete)
				{
					// stream requires more data, safe the remaining data
					break;
				}

				if (result == SerializableResult::Success)
				{
					completeCallback(message);
					data = Span(data.data() + reader.GetOffset(), data.size() - reader.GetOffset());
				}
				else
				{
					// stream broken, error
					return false;
				}
			}

			if (!data.empty())
			{
				ReadSpan(data, data.size());
			}

			return true;
		}
	};
}
