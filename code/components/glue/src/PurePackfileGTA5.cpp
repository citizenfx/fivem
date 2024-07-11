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

constexpr size_t shaHashSize = sizeof(cfx::puremode::Sha256Result);
constexpr size_t baseGameTableSize = sizeof(cfx::puremode::baseGameSafeHashesInit) / shaHashSize;
constexpr size_t updateTableSize = sizeof(cfx::puremode::updateSafeHashesInit) / shaHashSize;
constexpr size_t dlcTableSize = sizeof(cfx::puremode::dlcSafeHashesInit) / shaHashSize;
constexpr size_t manualTableSize = sizeof(cfx::puremode::manualSafeHashesInit) / shaHashSize;

static InitFunction initFunction([]
{
	if (fx::client::GetPureLevel() == 0)
	{
		return;
	}

	static std::unordered_set<cfx::puremode::Sha256Result> safeHashes;
	safeHashes.reserve(baseGameTableSize + updateTableSize + dlcTableSize + manualTableSize);
	for (int i = 0; i < baseGameTableSize; ++i)
	{
		safeHashes.insert(cfx::puremode::baseGameSafeHashesInit[i]);
	}

	for (int i = 0; i < updateTableSize; ++i)
	{
		safeHashes.insert(cfx::puremode::updateSafeHashesInit[i]);
	}

	for (int i = 0; i < dlcTableSize; ++i)
	{
		safeHashes.insert(cfx::puremode::dlcSafeHashesInit[i]);
	}

	for (int i = 0; i < manualTableSize; ++i)
	{
		safeHashes.insert(cfx::puremode::manualSafeHashesInit[i]);
	}

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

		cfx::puremode::Sha256Result out;
		{
			Botan::SHA_256 hash;
			hash.update(fixedHeader);
			hash.final(reinterpret_cast<uint8_t*>(&out));
		}

		return (safeHashes.find(out) != safeHashes.end());
	};

	SetPackfileValidationRoutine(validationCallback);
});
#endif
