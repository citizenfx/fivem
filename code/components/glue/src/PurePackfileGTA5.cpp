#include <StdInc.h>

#ifdef GTA_FIVE
#include <unordered_set>

#include <PureModeState.h>
#include <botan/sha2_32.h>

#include <BaseGameRpfHeaderHashes.h>
#include <UpdateRpfHeaderHashes.h>
#include <DlcRpfHeaderHashes.h>
#include <ManualRpfHeaderHashes.h>

DLL_IMPORT void SetPackfileValidationRoutine(bool (*routine)(const char*, const uint8_t*, size_t));

static InitFunction initFunction([]
{
	if (fx::client::GetPureLevel() == 0)
	{
		return;
	}

	static std::unordered_set<std::string> safeHashes = ([]
	{
		std::unordered_set<std::string> list;
		const auto addToHashList = [&list](const char* const arr[], size_t count)
		{
			for (int i = 0; i < count; ++i)
			{
				const char* hash = arr[i];
				uint8_t hashBytes[256 / 8] = { 0 };
				for (size_t i = 0; i < sizeof(hashBytes); i++)
				{
					char str[3] = { hash[i * 2], hash[i * 2 + 1], 0 };
					hashBytes[i] = uint8_t(strtol(str, nullptr, 16));
				}

				list.emplace((const char*)hashBytes, sizeof(hashBytes));
			}
		};

		addToHashList(baseGameSafeHashesInit, sizeof(baseGameSafeHashesInit) / 8);
		addToHashList(updateSafeHashesInit, sizeof(updateSafeHashesInit) / 8);
		addToHashList(dlcSafeHashesInit, sizeof(dlcSafeHashesInit) / 8);
		addToHashList(manualSafeHashesInit, sizeof(manualSafeHashesInit) / 8);
		return list;
	})();

	auto validationCallback = [](const char* path, const uint8_t* header, size_t headerLength)
	{
		if (fx::client::GetPureLevel() == 1)
		{
			std::string_view pathTest{ path };
			if (pathTest == "x64/audio/sfx/RESIDENT.rpf" || pathTest == "x64/audio/sfx/WEAPONS_PLAYER.rpf" || pathTest.find("x64/audio/sfx/STREAMED_VEHICLES") == 0 || pathTest.find("x64/audio/sfx/RADIO") == 0)
			{
				return true;
			}
		}

		std::vector<uint8_t> fixedHeader(header, header + headerLength);

		for (size_t i = 0; i < fixedHeader.size(); i += 16)
		{
			auto x = *(uint32_t*)(&fixedHeader[i + 4]);

			if (x != 0x7fffff00 && (x & 0x80000000) == 0)
			{
				auto y = (uint32_t*)(&fixedHeader[i + 12]);
				if (*y)
				{
					*y = 1;
				}
			}
		}

		uint8_t out[256 / 8] = { 0 };

		{
			Botan::SHA_256 hash;
			hash.update(fixedHeader);
			hash.final(out);
		}

		std::string str{
			(const char*)out,
			sizeof(out)
		};

		return (safeHashes.find(str) != safeHashes.end());
	};

	SetPackfileValidationRoutine(validationCallback);
});
#endif
