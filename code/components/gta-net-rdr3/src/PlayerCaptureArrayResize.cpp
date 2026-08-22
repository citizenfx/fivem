#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Patterns.h>

#include <Error.h>

#include <vector>

static constexpr int32_t kCaptureEntries = 160;
static constexpr int32_t kSlackEntries = 64;

static constexpr int32_t kPositionStride = 16;
static constexpr int32_t kMatrixStride = 144;

static constexpr int32_t kPositionArrayBytes = kCaptureEntries * kPositionStride;
static constexpr int32_t kMatrixArrayBytes = kCaptureEntries * kMatrixStride;

static constexpr int32_t kPositionBlockBytes = kPositionArrayBytes + 16 + (kSlackEntries * kPositionStride);
static constexpr int32_t kMatrixBlockBytes = kMatrixArrayBytes + 16 + (kSlackEntries * kMatrixStride);

static uint8_t* g_positions;
static uint8_t* g_matrices;

static int32_t* PositionCount()
{
	return reinterpret_cast<int32_t*>(g_positions + kPositionArrayBytes);
}

static int32_t* MatrixCount()
{
	return reinterpret_cast<int32_t*>(g_matrices + kMatrixArrayBytes);
}

static void* EraseCapturedRange(uint8_t* base, uint8_t* first, uint8_t* last, int32_t stride, int32_t* counter)
{
	uint8_t* end = base + (stride * (*counter));

	if (last != end)
	{
		memmove(first, last, size_t(end - last));
	}

	*counter -= int32_t((last - first) / stride);

	return first;
}

static void* ErasePositionRange(uint8_t* base, uint8_t* first, uint8_t* last)
{
	return EraseCapturedRange(base, first, last, kPositionStride, PositionCount());
}

static void* EraseMatrixRange(uint8_t* base, uint8_t* first, uint8_t* last)
{
	return EraseCapturedRange(base, first, last, kMatrixStride, MatrixCount());
}

static int (*g_origGetNetworkPlayerCount)();

static int GetCappedNetworkPlayerCount()
{
	return std::min(g_origGetNetworkPlayerCount(), kCaptureEntries - 1);
}

enum class CaptureTarget
{
	Positions,
	PositionCount,
	Matrices,
	MatrixCount,
	PositionCountFromArray,
	MatrixCountFromArray,
	PositionsScaled
};

struct PendingPatch
{
	uint8_t* location;
	CaptureTarget target;
};

static void CollectSites(std::vector<PendingPatch>& out, std::string_view pattern, int count, std::initializer_list<std::pair<int, CaptureTarget>> fields)
{
	auto matches = hook::pattern(pattern).count(count);

	for (int i = 0; i < count; i++)
	{
		auto base = matches.get(i).get<uint8_t>(0);

		for (auto& [offset, target] : fields)
		{
			out.push_back({ base + offset, target });
		}
	}
}

static HookFunction hookFunction([]()
{
	std::vector<PendingPatch> patches;

	auto structBase = hook::get_address<uint8_t*>(hook::get_pattern("48 83 EC ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 33 D2 38 15", 7));

	auto clearPositionSite = hook::get_pattern<uint8_t>("0F 11 83 E0 30 00 00 4C 63 81 00 02 00 00 49 C1 E0 04 4C 03 C1 E8");
	auto clearMatrixSite = hook::get_pattern<uint8_t>("48 8B D1 4C 8D 04 C0 49 C1 E0 04 4C 03 C1 48 83 C4 ? 5B E9");
	auto playerCountSite = hook::get_pattern<uint8_t>("48 8B F1 0F 84 ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 8B F8 85 DB", 9);

	CollectSites(patches, "48 63 96 00 33 00 00", 1, { { 3, CaptureTarget::PositionCount } });
	CollectSites(patches, "48 63 8E 00 33 00 00", 1, { { 3, CaptureTarget::PositionCount } });
	CollectSites(patches, "89 86 00 33 00 00", 2, { { 2, CaptureTarget::PositionCount } });
	CollectSites(patches, "0F 11 8C C6 00 31 00 00", 2, { { 4, CaptureTarget::Positions } });
	CollectSites(patches, "48 8D 96 10 33 00 00", 2, { { 3, CaptureTarget::Matrices } });
	CollectSites(patches, "48 63 8A 00 12 00 00", 2, { { 3, CaptureTarget::MatrixCountFromArray } });
	CollectSites(patches, "89 82 00 12 00 00", 2, { { 2, CaptureTarget::MatrixCountFromArray } });

	CollectSites(patches, "48 8D 8B 00 31 00 00", 1, { { 3, CaptureTarget::Positions } });
	CollectSites(patches, "48 8D 8B 10 33 00 00 48 63 81 00 12 00 00", 1, { { 3, CaptureTarget::Matrices }, { 10, CaptureTarget::MatrixCountFromArray } });

	CollectSites(patches, "89 81 00 33 00 00 89 81 10 45 00 00", 1, { { 2, CaptureTarget::PositionCount }, { 8, CaptureTarget::MatrixCount } });

	CollectSites(patches, "39 99 00 33 00 00", 2, { { 2, CaptureTarget::PositionCount } });
	CollectSites(patches, "3B 9F 00 33 00 00", 2, { { 2, CaptureTarget::PositionCount } });
	CollectSites(patches, "8B C3 48 8D 4C 24 ? 48 05 10 03 00 00 0F 28 CE", 1, { { 9, CaptureTarget::PositionsScaled } });
	CollectSites(patches, "48 81 C2 10 33 00 00", 1, { { 3, CaptureTarget::Matrices } });

	patches.push_back({ clearPositionSite + 10, CaptureTarget::PositionCountFromArray });

	auto blockRaw = reinterpret_cast<uintptr_t>(hook::AllocateStubMemory(kPositionBlockBytes + kMatrixBlockBytes + 16));

	if (!blockRaw)
	{
		FatalError("Couldn't allocate %d bytes of stub memory for the player capture arrays.", kPositionBlockBytes + kMatrixBlockBytes + 16);
		return;
	}

	auto block = reinterpret_cast<uint8_t*>((blockRaw + 15) & ~uintptr_t(15));

	memset(block, 0, kPositionBlockBytes + kMatrixBlockBytes);

	g_positions = block;
	g_matrices = block + kPositionBlockBytes;

	const int64_t positionDelta = g_positions - structBase;
	const int64_t matrixCountDelta = (g_matrices + kMatrixArrayBytes) - structBase;

	if (positionDelta != int32_t(positionDelta) || matrixCountDelta != int32_t(matrixCountDelta))
	{
		FatalError("Player capture arrays landed %lld bytes from the owning structure, out of reach of the game's 32-bit displacements.", positionDelta);
		return;
	}

	if ((positionDelta % kPositionStride) != 0)
	{
		FatalError("Player capture position array is not 16-byte aligned relative to the owning structure.");
		return;
	}

	for (auto& patch : patches)
	{
		int32_t value = 0;

		switch (patch.target)
		{
			case CaptureTarget::Positions:
				value = int32_t(positionDelta);
				break;
			case CaptureTarget::PositionCount:
				value = int32_t(positionDelta) + kPositionArrayBytes;
				break;
			case CaptureTarget::Matrices:
				value = int32_t(g_matrices - structBase);
				break;
			case CaptureTarget::MatrixCount:
				value = int32_t(matrixCountDelta);
				break;
			case CaptureTarget::PositionCountFromArray:
				value = kPositionArrayBytes;
				break;
			case CaptureTarget::MatrixCountFromArray:
				value = kMatrixArrayBytes;
				break;
			case CaptureTarget::PositionsScaled:
				value = int32_t(positionDelta) / kPositionStride;
				break;
		}

		hook::put<int32_t>(patch.location, value);
	}

	hook::call(clearPositionSite + 21, ErasePositionRange);
	hook::jump(clearMatrixSite + 19, EraseMatrixRange);

	hook::set_call(&g_origGetNetworkPlayerCount, playerCountSite);
	hook::call(playerCountSite, GetCappedNetworkPlayerCount);
});
