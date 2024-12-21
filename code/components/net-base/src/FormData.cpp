#include "StdInc.h"

#include "FormData.h"

/// <summary>
/// This function is decoding a POST form data into a map.
/// The posts strings have the following format key=value&key2=value2
/// </summary>
std::map<std::string, std::string> net::DecodeFormData(const std::string_view& data)
{
	std::map<std::string, std::string> postMap;

	const size_t postDataStringSize = data.size();
	for (size_t i = 0; i < postDataStringSize; i++)
	{
		std::string_view remainingData{data.data() + i, postDataStringSize - i};
		const size_t keyEndPos = remainingData.find('=');
		if (keyEndPos == std::string_view::npos || keyEndPos == 0)
		{
			// todo: return parsing error to close connection
			// invalid format, return what we have
			return postMap;
		}

		// save the key length for later
		const size_t keyIndex = i;
		// skip the key and the '=' character
		i += keyEndPos + 1;

		if (i >= postDataStringSize)
		{
			// todo: return parsing error to close connection
			// invalid format, because the value should never be empty, return what we have
			return postMap;
		}

		remainingData = {data.data() + i, postDataStringSize - i};
		size_t valueEndPos = remainingData.find('&');
		if (!valueEndPos)
		{
			// todo: return parsing error to close connection
			// invalid format, because '=&' is not valid, return what we have
			return postMap;
		}

		// unlike the other finds, this one does not need to exist, because there does not need to be a next value
		if (valueEndPos == std::string_view::npos)
		{
			valueEndPos = remainingData.size();
		}

		std::string_view key(data.data() + keyIndex, keyEndPos);
		std::string_view value(data.data() + i, valueEndPos);

		std::string keyDecoded;
		std::string valueDecoded;

		UrlDecode(key, keyDecoded);
		UrlDecode(value, valueDecoded);

		postMap[keyDecoded] = valueDecoded;

		// skip till the value end
		i += valueEndPos;
	}

	return postMap;
}

bool net::UrlDecode(const std::string_view& in, std::string& out, const bool replacePlus)
{
	const size_t inSize = in.size();
	out.clear();
	out.reserve(inSize);
	for (std::size_t i = 0; i < inSize; ++i)
	{
		if (in[i] == '%')
		{
			if (i + 3 <= inSize)
			{
				try
				{
					int value = std::stoi(std::string(in.substr(i + 1, 2)), nullptr, 16);
					out += static_cast<char>(value);
					i += 2;
				}
				catch (...)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else if (in[i] == '+' && replacePlus)
		{
			out += ' ';
		}
		else
		{
			out += in[i];
		}
	}

	return true;
}
