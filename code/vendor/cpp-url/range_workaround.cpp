#include "skyr/percent_encode.hpp"
#include <skyr/v1/percent_encoding/percent_decode.hpp>
#include <skyr/v1/percent_encoding/percent_encode.hpp>

namespace skyr
{
tl::expected<std::string, skyr::v1::percent_encoding::percent_encode_errc> percent_decode(std::string_view input)
{
	return skyr::v1::percent_decode(input);
}

std::optional<std::string> percent_encode(std::string_view input, skyr::encode_set set)
{
	using percent_encoding::percent_encoded_char;

	auto encode = [set](auto byte)
	{
		if ((byte == '\x2a') || (byte == '\x2d') || (byte == '\x2e') || ((byte >= '\x30') && (byte <= '\x39')) || ((byte >= '\x41') && (byte <= '\x5a')) || (byte == '\x5f') || ((byte >= '\x61') && (byte <= '\x7a')))
		{
			return percent_encoded_char(
			byte, percent_encoded_char::no_encode());
		}
		return percent_encode_byte(byte, set);
	};

	auto result = std::string{};
	for (auto encoded : input | ranges::views::transform(encode))
	{
		result += std::string(std::cbegin(encoded), std::cend(encoded));
	}
	return result;
}
}
