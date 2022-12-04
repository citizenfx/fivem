#pragma once

// we don't want assertions even though we use assert.h
#define JSON_ASSERT(x)

// if RTTI is off, <any> is a no-op, but if C++17 is used and <any> does
// not define std::any, this leads to a compile error in the real json.hpp
//
// to fix this we force C++14 mode in this case
#if defined(_MSC_VER) && !_HAS_STATIC_RTTI
#define JSON_HAS_CPP_14
#define JSON_HAS_CPP_11
#endif

// include json_fwd.hpp so that we have the required defines for
// name-lookup order and defining our new input adapter
#include "../../vendor/nljson/single_include/nlohmann/json_fwd.hpp"

// input adapter for non-char istreams
NLOHMANN_JSON_NAMESPACE_BEGIN
/*!
Input adapter for a (caching) istream. Ignores a UFT Byte Order Mark at
beginning of input. Does not support changing the underlying std::streambuf
in mid-input. Maintains underlying std::istream and std::streambuf to support
subsequent use of standard std::istream operations to process any input
characters following those used in parsing the JSON input.  Clears the
std::istream flags; any input errors (e.g., EOF) will be detected by the first
subsequent call for input from the std::istream.
*/
template<typename TChar>
class input_stream_adapter_any
{
public:
	using char_type = TChar;

	~input_stream_adapter_any()
	{
		// clear stream flags; we use underlying streambuf I/O, do not
		// maintain ifstream flags, except eof
		if (is)
		{
			is->clear(is->rdstate() & std::ios::eofbit);
		}
	}

	explicit input_stream_adapter_any(std::basic_istream<TChar>& i)
		: is(&i), sb(i.rdbuf())
	{
	}

	// delete because of pointer members
	input_stream_adapter_any(const input_stream_adapter_any&) = delete;
	input_stream_adapter_any& operator=(input_stream_adapter_any&) = delete;
	input_stream_adapter_any& operator=(input_stream_adapter_any&& rhs) = delete;

	input_stream_adapter_any(input_stream_adapter_any&& rhs)
		: is(rhs.is), sb(rhs.sb)
	{
		rhs.is = nullptr;
		rhs.sb = nullptr;
	}

	// std::istream/std::streambuf use std::char_traits<char>::to_int_type, to
	// ensure that std::char_traits<char>::eof() and the character 0xFF do not
	// end up as the same value, eg. 0xFFFFFFFF.
	auto get_character()
	{
		auto res = sb->sbumpc();
		// set eof manually, as we don't use the istream interface.
		if (res == EOF)
		{
			is->clear(is->rdstate() | std::ios::eofbit);
		}
		return res;
	}

private:
	/// the associated input stream
	std::basic_istream<TChar>* is = nullptr;
	std::basic_streambuf<TChar>* sb = nullptr;
};

namespace detail
{
template<typename TChar>
inline input_stream_adapter_any<TChar> input_adapter(std::basic_istream<TChar>& stream)
{
	return input_stream_adapter_any<TChar>(stream);
}

template<typename TChar>
inline input_stream_adapter_any<TChar> input_adapter(std::basic_istream<TChar>&& stream)
{
	return input_stream_adapter_any<TChar>(stream);
}
}
NLOHMANN_JSON_NAMESPACE_END

// now that we have set our configuration and name-lookup overrides, we can include actual json.hpp
#include "../../vendor/nljson/single_include/nlohmann/json.hpp"
