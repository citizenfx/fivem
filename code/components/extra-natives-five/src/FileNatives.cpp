#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>
#include <fiDevice.h>

#include <botan/sha2_32.h>
#include <boost/algorithm/string.hpp>

static const std::array<std::string_view, 7> kWhitelistedPaths = {
	"platform:/",
	"common:/",
	"update:/",
	"update2:/",
	"audio:/",
	"commoncrc:/",
	"platformcrc:/"
};

static bool IsPathWhitelisted(std::string_view path)
{
	return std::any_of(kWhitelistedPaths.begin(), kWhitelistedPaths.end(),
	[&path](const auto& prefix)
	{
		return boost::algorithm::starts_with(path, prefix);
	});
}

static std::string ToHexString(const uint8_t* data, size_t length)
{
	static const char hex_chars[] = "0123456789ABCDEF";
	std::string result(length * 2, ' ');

	for (size_t i = 0; i < length; ++i)
	{
		result[2 * i] = hex_chars[data[i] >> 4];
		result[2 * i + 1] = hex_chars[data[i] & 0xF];
	}

	return result;
};

static HookFunction hookFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_MOUNTED_FILE_HASH", [](fx::ScriptContext& context)
	{
		const char* path = context.CheckArgument<const char*>(0);
		static std::string result = "missing";

		if (IsPathWhitelisted(path))
		{
			rage::fiDevice* device = rage::fiDevice::GetDevice(path, true);

			if (device)
			{
				auto handle = device->Open(path, true);

				if (handle != -1)
				{
					auto len = device->GetFileLength(handle);
					std::vector<char> data(len);

					device->Read(handle, data.data(), len);
					device->Close(handle);

					Botan::SHA_256 hashFunction;
					hashFunction.update(reinterpret_cast<const uint8_t*>(data.data()), data.size());
					std::vector<uint8_t> hash(hashFunction.output_length());
					hashFunction.final(hash.data());

					result = ToHexString(hash.data(), hash.size());
				}
			}
		}

		context.SetResult(result.c_str());
	});
});
