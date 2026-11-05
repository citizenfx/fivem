#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace game
{
static constexpr size_t kMinResourceLabelLength = 2;
static constexpr size_t kMaxResourceLabelLength = 32;

// Trims a resource's `ui_label` and collapses runs of spaces in it. Returns an empty string if the result isn't valid UTF-8 of
// kMinResourceLabelLength to kMaxResourceLabelLength characters, or has control, invisible, non-ASCII space or private use
// characters, or game text markup (~ < > &) that could make it look like something else.
inline std::string SanitizeResourceLabel(std::string_view label)
{
	// C1 controls, Unicode spaces, default ignorable code points, blank fillers, private use and noncharacters
	static constexpr std::pair<uint32_t, uint32_t> invisibleRanges[] = {
		{ 0x7F, 0xA0 }, { 0xAD, 0xAD }, { 0x34F, 0x34F }, { 0x61C, 0x61C }, { 0x115F, 0x1160 }, { 0x1680, 0x1680 },
		{ 0x17B4, 0x17B5 }, { 0x180B, 0x180F }, { 0x2000, 0x200F }, { 0x2028, 0x202F }, { 0x205F, 0x206F },
		{ 0x2800, 0x2800 }, { 0x3000, 0x3000 }, { 0x3164, 0x3164 }, { 0xE000, 0xF8FF }, { 0xFDD0, 0xFDEF },
		{ 0xFE00, 0xFE0F }, { 0xFEFF, 0xFEFF }, { 0xFFA0, 0xFFA0 }, { 0xFFF0, 0xFFFF }, { 0x1BCA0, 0x1BCA3 },
		{ 0x1D173, 0x1D17A }, { 0xE0000, 0xE0FFF }, { 0xF0000, 0x10FFFF },
	};

	auto first = label.find_first_not_of(" \t\r\n\v\f");

	if (first == std::string_view::npos)
	{
		return {};
	}

	label = label.substr(first, label.find_last_not_of(" \t\r\n\v\f") - first + 1);

	std::string sanitized;
	size_t length = 0;

	for (size_t i = 0; i < label.size();)
	{
		auto lead = static_cast<uint8_t>(label[i]);
		size_t size = (lead < 0x80) ? 1 : (lead < 0xC2) ? 0 : (lead < 0xE0) ? 2 : (lead < 0xF0) ? 3 : (lead < 0xF5) ? 4 : 0;

		if (size == 0 || label.size() - i < size)
		{
			return {};
		}

		uint32_t c = (size == 1) ? lead : (lead & (0x7F >> size));

		for (size_t j = 1; j < size; j++)
		{
			auto next = static_cast<uint8_t>(label[i + j]);

			if ((next & 0xC0) != 0x80)
			{
				return {};
			}

			c = (c << 6) | (next & 0x3F);
		}

		// overlong encodings, surrogates and code points past U+10FFFF
		if ((size == 3 && c < 0x800) || (size == 4 && (c < 0x10000 || c > 0x10FFFF)) || (c >= 0xD800 && c <= 0xDFFF))
		{
			return {};
		}

		if (c < 0x20 || c == '~' || c == '<' || c == '>' || c == '&')
		{
			return {};
		}

		for (const auto& [low, high] : invisibleRanges)
		{
			if (c >= low && c <= high)
			{
				return {};
			}
		}

		if (c != ' ' || sanitized.back() != ' ')
		{
			sanitized.append(label.substr(i, size));
			length++;
		}

		i += size;
	}

	if (length < kMinResourceLabelLength || length > kMaxResourceLabelLength)
	{
		return {};
	}

	return sanitized;
}

// Returns the label `getLabel` gives resource `name`, or `name` itself if it has none or another resource in `names` has
// the label as its name or label (ignoring case), so no resource can pass for another.
template<typename TNames, typename TGetLabel>
inline std::string GetResourceDisplayName(const std::string& name, const TNames& names, const TGetLabel& getLabel)
{
	auto equals = [](std::string_view left, std::string_view right)
	{
		return std::equal(left.begin(), left.end(), right.begin(), right.end(), [](char a, char b)
		{
			return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
		});
	};

	std::string label = getLabel(name);

	if (label.empty())
	{
		return name;
	}

	for (const auto& other : names)
	{
		if (other != name && (equals(other, label) || equals(getLabel(other), label)))
		{
			return name;
		}
	}

	return label;
}
}
