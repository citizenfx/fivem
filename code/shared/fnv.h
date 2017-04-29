#pragma once

// inspired by/based upon http://create.stephan-brumme.com/fnv-hash/ and http://stackoverflow.com/questions/34597260/stdhash-value-on-char-value-and-not-on-memory-address/34597785

#include <stdint.h>

#define __tolower(c) ((c) - 'A' + 'a')

struct fnv1a_process_noop
{
	static constexpr inline uint8_t Process(const uint8_t value) noexcept
	{
		return value;
	}
};

struct fnv1a_process_tolower
{
	static constexpr inline uint8_t Process(const uint8_t value) noexcept
	{
		return __tolower(value);
	}
};

template<typename TInteger, TInteger Seed, TInteger Prime, typename ProcessByte = fnv1a_process_noop>
struct fnv1a_impl
{
	inline constexpr fnv1a_impl()
	{

	}

	inline TInteger operator()(const std::string& buffer) const noexcept
	{
		return ProcessUnrolled<sizeof(size_t)>(buffer.c_str(), buffer.length(), Seed);
	}

	inline TInteger operator()(const void* buffer, size_t length) const noexcept
	{
		return ProcessUnrolled<sizeof(size_t)>(buffer, length, Seed);
	}

	constexpr inline TInteger operator()(const char* const buffer) const noexcept
	{
		return Process(buffer, Seed);
	}

	constexpr static inline TInteger Hash(const char* const buffer) noexcept
	{
		return Process(buffer, Seed);
	}

private:
	inline static constexpr TInteger Process(const char* buffer, const TInteger hash_) noexcept
	{
		TInteger hash = hash_;

		while (*buffer)
		{
			hash = static_cast<TInteger>(1ULL * (ProcessByte::Process(*buffer++) ^ hash) * Prime);
		}

		return hash;
	}

	TInteger Process(const void* buffer, size_t length, TInteger hash) const noexcept
	{
		auto data = static_cast<const uint8_t*>(buffer);

		while (length--)
		{
			hash = (ProcessByte::Process(*data++) ^ hash) * Prime;
		}

		return hash;
	}

	template<size_t Unroll>
	TInteger ProcessUnrolled(const void* buffer, size_t length, TInteger hash) const noexcept
	{
		auto data = static_cast<const uint8_t*>(buffer);

		while (length >= Unroll)
		{
			hash = Process(data, Unroll, hash);
			data += Unroll;
			length -= Unroll;
		}

		return Process(data, length, hash);
	}

	template<>
	inline TInteger ProcessUnrolled<0>(const void* buffer, size_t length, TInteger hash) const noexcept
	{
		return Process(buffer, length, hash);
	}

	template<>
	inline TInteger ProcessUnrolled<1>(const void* buffer, size_t length, TInteger hash) const noexcept
	{
		return Process(buffer, length, hash);
	}
};

template<typename ProcessByte = fnv1a_process_noop>
using fnv1a_32_impl = fnv1a_impl<uint32_t, 0x811C9DC5, 0x1000193, ProcessByte>;

template<typename ProcessByte = fnv1a_process_noop>
using fnv1a_64_impl = fnv1a_impl<uint64_t, 0xCBF29CE484222325, 0x100000001B3, ProcessByte>;

template<size_t Bytes>
struct fnv1a;

template<>
struct fnv1a<4>
{
	using type = fnv1a_32_impl<>;
	using lowercase_type = fnv1a_32_impl<fnv1a_process_tolower>;
};

template<>
struct fnv1a<8>
{
	using type = fnv1a_64_impl<>;
	using lowercase_type = fnv1a_64_impl<fnv1a_process_tolower>;
};

template<size_t Bytes>
using fnv1a_t = typename fnv1a<Bytes>::type;

template<size_t Bytes>
using fnv1a_lower_t = typename fnv1a<Bytes>::lowercase_type;

using fnv1a_size_t = fnv1a_t<sizeof(size_t)>;
using fnv1a_size_lower_t = fnv1a_lower_t<sizeof(size_t)>;