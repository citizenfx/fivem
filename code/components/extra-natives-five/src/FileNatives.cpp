#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>
#include <fiDevice.h>

#include <botan/sha2_32.h>
#include <boost/algorithm/string.hpp>
#include <concurrent_unordered_map.h>
#include <atomic>

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

struct HashResult
{
	std::string hash;
	bool isComplete;
};

static concurrency::concurrent_unordered_map<std::string, HashResult> g_fileHashes;
static std::atomic<bool> g_isFetching(false);

static DWORD WINAPI CalculateFileHash(LPVOID lpParameter)
{
	std::unique_ptr<std::string> pathStr(static_cast<std::string*>(lpParameter));
	std::string result = "missing";

	const char* path = pathStr->c_str();
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

	g_fileHashes[*pathStr] = { result, true };
	g_isFetching.store(false, std::memory_order_release);

	return 0;
}

static HookFunction hookFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("FETCH_MOUNTED_FILE_HASH", [](fx::ScriptContext& context)
	{
		bool expected = false;
		if (!g_isFetching.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
		{
			context.SetResult(false);
			return;
		}

		const std::string path = context.CheckArgument<const char*>(0);

		auto it = g_fileHashes.find(path);
		if (it == g_fileHashes.end())
		{
			if (IsPathWhitelisted(path))
			{
				g_fileHashes[path] = { "", false };

				auto pathCopy = new std::string(path);
				QueueUserWorkItem(CalculateFileHash, pathCopy, 0);
			}
			else
			{
				// Mark non-whitelisted paths as complete with "missing" hash
				g_fileHashes[path] = { "missing", true };
			}
		}

		context.SetResult(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_MOUNTED_FILE_HASH", [](fx::ScriptContext& context)
	{
		const std::string path = context.CheckArgument<const char*>(0);

		auto it = g_fileHashes.find(path);
		if (it != g_fileHashes.end() && it->second.isComplete)
		{
			context.SetResult(it->second.hash.c_str());
		}
		else
		{
			context.SetResult<const char*>("missing");
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_MOUNTED_FILE_HASH_READY", [](fx::ScriptContext& context)
	{
		const std::string path = context.CheckArgument<const char*>(0);

		auto it = g_fileHashes.find(path);
		context.SetResult(it != g_fileHashes.end() && it->second.isComplete);
	});
});
