#pragma once

#include <optional>
#include <string_view>
#include <tl/expected.hpp>
#include <skyr/v1/percent_encoding/errors.hpp>
#include <skyr/v1/percent_encoding/percent_encoded_char.hpp>

namespace skyr
{
using encode_set = percent_encoding::encode_set;

std::optional<std::string> percent_encode(std::string_view input, skyr::encode_set set);
tl::expected<std::string, skyr::v1::percent_encoding::percent_encode_errc> percent_decode(std::string_view input);
}
