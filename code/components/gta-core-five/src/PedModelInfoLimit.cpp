#include <StdInc.h>

#include <fiDevice.h>
#include <PoolSizesState.h>

#include <filesystem>
#include <optional>
#include <regex>

//
// MaxExtraPedModelInfos is read from gameconfig.xml (CConfigModelInfo) instead of the PoolSizes list, so the
// GetSizeOfPool hook in PoolManagement.cpp never sees it. To let servers raise it through `increase_pool_size`,
// we write a copy of gameconfig.xml with the increased value and mount it over common:/ before the game reads it.
//

static constexpr const char* kPedModelInfosName = "MaxExtraPedModelInfos";

static bool PatchMaxExtraPedModelInfos(std::string& xml, uint32_t increase, uint32_t& newValue)
{
	static const std::regex valueRegex(R"re(<MaxExtraPedModelInfos\s+value="(\d{1,9})")re");

	std::smatch match;
	if (!std::regex_search(xml, match, valueRegex))
	{
		return false;
	}

	uint64_t value = std::stoull(match[1].str()) + increase;
	if (value > UINT32_MAX)
	{
		return false;
	}

	newValue = static_cast<uint32_t>(value);
	xml.replace(match.position(1), match.length(1), std::to_string(newValue));
	return true;
}

static std::optional<std::string> ReadGameFile(const char* path)
{
	auto device = rage::fiDevice::GetDevice(path, true);
	if (!device)
	{
		return std::nullopt;
	}

	auto handle = device->Open(path, true);
	if (handle == -1)
	{
		return std::nullopt;
	}

	int length = device->GetFileLength(handle);
	std::string data(std::max(length, 0), '\0');
	bool success = length > 0 && device->ReadFull(handle, data.data(), data.size());
	device->Close(handle);

	if (!success)
	{
		return std::nullopt;
	}

	return data;
}

static InitFunction initFunction([]()
{
	rage::fiDevice::OnInitialMount.Connect([]()
	{
		const auto& increaseRequest = fx::PoolSizeManager::GetIncreaseRequest();
		auto entry = increaseRequest.find(kPedModelInfosName);
		if (entry == increaseRequest.end() || entry->second == 0)
		{
			return;
		}

		auto xml = ReadGameFile("common:/data/gameconfig.xml");
		if (!xml)
		{
			trace("Could not read gameconfig.xml, %s size increase will not be applied.\n", kPedModelInfosName);
			return;
		}

		uint32_t newValue = 0;
		if (!PatchMaxExtraPedModelInfos(*xml, entry->second, newValue))
		{
			trace("Could not find %s in gameconfig.xml, size increase will not be applied.\n", kPedModelInfosName);
			return;
		}

		std::wstring overrideRoot = MakeRelativeCitPath(L"data/cache/gameconfig-override/");
		std::error_code ec;
		std::filesystem::create_directories(overrideRoot + L"data", ec);

		FILE* f = _wfopen((overrideRoot + L"data/gameconfig.xml").c_str(), L"wb");
		if (!f)
		{
			trace("Could not write gameconfig.xml override, %s size increase will not be applied.\n", kPedModelInfosName);
			return;
		}

		bool written = fwrite(xml->data(), 1, xml->size(), f) == xml->size();
		fclose(f);

		if (!written)
		{
			trace("Could not write gameconfig.xml override, %s size increase will not be applied.\n", kPedModelInfosName);
			return;
		}

		// mounted after the citizen/addon mounts, so this copy takes priority and other files fall through
		for (const char* mount : { "common:/", "commoncrc:/" })
		{
			rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
			device->SetPath(ToNarrow(overrideRoot).c_str(), true);
			device->Mount(mount);
		}

		trace("Increased %s by %d to %d.\n", kPedModelInfosName, entry->second, newValue);
	},
	1000);
});
