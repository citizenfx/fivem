#include <StdInc.h>

#include <Hooking.h>
#include <MinHook.h>

#include <CoreConsole.h>
#include <ConsoleHost.h>

#include <Pool.h>

#include <imgui.h>

#include <vector>
#include <algorithm>

#include <intrin.h>

struct HiddenPoolInfo
{
	uint32_t hash;
	void* pool;
	uint32_t count;
	uint32_t entrySize;
};

static std::vector<HiddenPoolInfo> g_hiddenPools;
static bool g_showHiddenPools;

static int GetHiddenPoolUsed(const HiddenPoolInfo& info)
{
	if (!info.pool || info.count == 0 || info.count > 65536)
	{
		return -1;
	}

	auto flags = *(uint8_t**)((char*)info.pool + 0x18);

	if (!flags || IsBadReadPtr(flags, info.count))
	{
		return -1;
	}

	int used = 0;

	for (uint32_t i = 0; i < info.count; i++)
	{
		if (flags[i] & 1)
		{
			used++;
		}
	}

	return used;
}

static uint32_t FindPoolHashFromCaller(const uint8_t* returnAddress)
{
	if (!returnAddress || IsBadReadPtr(returnAddress - 160, 160))
	{
		return 0;
	}

	for (int i = 5; i < 160; i++)
	{
		if (returnAddress[-i] == 0xBA)
		{
			uint32_t value = *(uint32_t*)(returnAddress - i + 1);

			if (value > 0xFFFF)
			{
				return value;
			}
		}
	}

	return 0;
}

static void* (*g_origInitMemoryPool)(void*, uint32_t, uint32_t, uint32_t);

static void* InitMemoryPool(void* pool, uint32_t count, uint32_t entrySize, uint32_t align)
{
	auto result = g_origInitMemoryPool(pool, count, entrySize, align);

	g_hiddenPools.push_back({ FindPoolHashFromCaller((const uint8_t*)_ReturnAddress()), pool, count, entrySize });

	return result;
}

static void DrawHiddenPools()
{
	if (!g_showHiddenPools)
	{
		return;
	}

	if (ImGui::Begin("Hidden Pools", &g_showHiddenPools))
	{
		static char search[96];
		ImGui::InputText("Search", search, IM_ARRAYSIZE(search));
		ImGui::Text("%d pools registered", (int)g_hiddenPools.size());
		ImGui::Separator();

		if (ImGui::BeginTable("hiddenPools", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY))
		{
			ImGui::TableSetupColumn("Pool");
			ImGui::TableSetupColumn("Used");
			ImGui::TableSetupColumn("Max");
			ImGui::TableSetupColumn("Entry size");
			ImGui::TableSetupColumn("Used %");
			ImGui::TableHeadersRow();

			for (const auto& info : g_hiddenPools)
			{
				auto name = rage::LookupPoolName(info.hash);
				auto label = name.c_str();

				if (search[0] && !strstr(label, search))
				{
					continue;
				}

				int used = GetHiddenPoolUsed(info);
				float pct = (used < 0 || info.count == 0) ? 0.0f : ((float)used / info.count * 100.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TextColored(pct >= 90.0f ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", label);
				ImGui::TableNextColumn();
				ImGui::Text(used < 0 ? "?" : "%d", used);
				ImGui::TableNextColumn();
				ImGui::Text("%u", info.count);
				ImGui::TableNextColumn();
				ImGui::Text("%u", info.entrySize);
				ImGui::TableNextColumn();
				ImGui::Text("%.1f%%", pct);
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();
}

static HookFunction hookFunction([]()
{
	MH_Initialize();

	auto initPattern = hook::pattern("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 30 48 83 39 00");

	if (initPattern.size() == 1)
	{
		MH_CreateHook(initPattern.get(0).get<void>(0), InitMemoryPool, (void**)&g_origInitMemoryPool);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	static ConVar<bool> hiddenPoolsVar("net_showHiddenPools", ConVar_Archive | ConVar_UserPref, false, &g_showHiddenPools);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || g_showHiddenPools;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		DrawHiddenPools();
	});
});
