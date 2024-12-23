#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <FormData.h>

#include "TestUtils.h"

TEST_CASE("Form data decode")
{
	std::string data;
	constexpr size_t valueCount = 100;
	std::map<std::string, std::string> generatedData {};
	for (size_t i = 0; i < valueCount; i++)
	{
		std::string key = fx::TestUtils::asciiRandom(20);
		std::string value = fx::TestUtils::asciiRandom(20);
		if (i == valueCount - 1)
		{
			data += key + "=" + value;
		}
		else
		{
			data += key + "=" + value + "&";
		}

		generatedData[key] = value;
	}

	std::map<std::string, std::string> decodedData = net::DecodeFormData(data);

	REQUIRE(generatedData == decodedData);
}

TEST_CASE("Form data url decode")
{
	WHEN ("%20 is given in")
	{
		std::string value = "%20";
		std::string out;
		REQUIRE(net::UrlDecode(value, out) == true);
		THEN("it is replaced by a space")
		{
			REQUIRE(out == " ");
		}
	}
	WHEN ("+ is given in")
	{
		std::string value = "+";
		std::string out;
		REQUIRE(net::UrlDecode(value, out) == true);
		THEN("it is replaced by a space")
		{
			REQUIRE(out == " ");
		}
	}
	WHEN ("+ is given in and replace + is false")
	{
		std::string value = "+";
		std::string out;
		REQUIRE(net::UrlDecode(value, out, false) == true);
		THEN("it is not replaced by a space")
		{
			REQUIRE(out == "+");
		}
	}
}
