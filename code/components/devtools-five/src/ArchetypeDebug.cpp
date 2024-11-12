#include <StdInc.h>
#include <CoreConsole.h>
#include <ConsoleHost.h>
#include <imgui.h>
#include <atHashMap.h>
#include <string>
#include <Hooking.h>
#include <EntitySystem.h>
#include <Streaming.h>

template<typename T>
class fwArchetypeDynamicFactory : public fwFactoryBase<T>
{
public:
	atMultiHashMap<T> data;
};

struct FactoryInfo
{
	FactoryInfo(const char* factoryName) : name(factoryName), count(0), usage(0.0f) {}

	std::string name;

	size_t count;

	float usage;
};

struct FactoryBucket
{
	FactoryBucket() : count(0) {}

	std::string name;

	size_t count;
};

static void* rage__fwArchetypeManager__ms_ArchetypePool;

// Taken from CloneExperiments.cpp
static hook::cdecl_stub<const char* (int, uint32_t)> rage__atHashStringNamespaceSupport__GetString([]
{
	return hook::get_pattern("85 D2 75 04 33 C0 EB 61", -0x18);
});

static HookFunction hookFunction([]()
{
	auto location = hook::get_call(hook::get_pattern<char>("E8 ? ? ? ? EB 0B 45 33 C0 48 8B CB"));
	rage__fwArchetypeManager__ms_ArchetypePool = hook::get_address<void*>(location + 0x2B);
});

static std::string ResolveHashString(uint32_t hash)
{
	const char* resolvedName = rage__atHashStringNamespaceSupport__GetString(1, hash);
	if (resolvedName)
	{
		return resolvedName;
	}

	std::string fileName = streaming::GetStreamingBaseNameForHash(hash);
	if (!fileName.empty())
	{
		return fileName;
	}

	return fmt::sprintf("Unknown 0x%08X", hash);
}

static std::string GetFriendlyNameFromBucketHash(uint32_t hash)
{
	if (hash == 0xFFFF)
	{
		return "Base Game Content";
	}
	else if (hash == 0xF000)
	{
		return "Other Game Content";
	}
	else if (hash < 0xFFFF)
	{
		static auto mapTypesStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytyp");
		void* mapTypesContent = mapTypesStore->GetPtr(hash);
		if (mapTypesContent)
		{
			hash = *(uint32_t*)((char*)mapTypesContent + 0x8);
		}
	}
	return ResolveHashString(hash);
}

template<typename T>
static int SortCompare(const T& lhs, const T& rhs)
{
	if (lhs < rhs)
	{
		return -1;
	}
	else if (lhs > rhs)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static InitFunction initFunction([]()
{
	static bool archetypeListEnabled;
	static ConVar<bool> archetypeListVar("archetypelist", ConVar_Archive | ConVar_UserPref, false, &archetypeListEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || archetypeListEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!archetypeListEnabled)
		{
			return;
		}
		
		size_t archetypePoolSize = *(uint32_t*)(((char*)rage__fwArchetypeManager__ms_ArchetypePool) + 0x8);
		size_t archetypePoolFree = *(uint32_t*)(((char*)rage__fwArchetypeManager__ms_ArchetypePool) + 0x10);

		// Wait for some archetypes to exist
		if (archetypePoolFree == archetypePoolSize)
		{
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(500.0f, 300.0f), ImGuiCond_FirstUseEver);

		std::vector<FactoryInfo> factoryInfos = {
			"CBaseModelInfo",
			"CMloModelInfo",
			"CTimeModelInfo",
			"CWeaponModelInfo",
			"CVehicleModelInfo",
			"CPedModelInfo",
			"CCompEntityModelInfo",
		};

		std::unordered_map<uint32_t, FactoryBucket> factoryBucketsLookup;

		for (int i = 0; i < factoryInfos.size(); i++)
		{
			auto& factoryInfo = factoryInfos[i];
			auto factory = reinterpret_cast<fwArchetypeDynamicFactory<fwArchetype>*>(g_archetypeFactories->Get(i + 1));

			factory->data.ForAllEntriesWithHash([&](uint32_t hash, atArray<fwArchetype>* entries)
			{
				factoryInfo.count += entries->GetCount();
				auto& factoryBucket = factoryBucketsLookup[hash];
				factoryBucket.name = GetFriendlyNameFromBucketHash(hash);
				factoryBucket.count += entries->GetCount();
			});

			factoryInfo.usage = (float(factoryInfo.count) / float(archetypePoolSize)) * 100.0f;
		}

		if (ImGui::Begin("Archetype List", &archetypeListEnabled))
		{
			ImGui::Text("Available Archetypes: %d / %d", archetypePoolFree, archetypePoolSize);

			ImGui::Text("Registered Archetypes: %d (%.02f%%)", archetypePoolSize - archetypePoolFree, (1.0 - (float(archetypePoolFree) / float(archetypePoolSize))) * 100.0f);

			if (ImGui::BeginTable("##archetypedistribution", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Factory Type");
				ImGui::TableSetupColumn("Num Archetypes Registered", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_DefaultSort);
				ImGui::TableSetupColumn("Total Contribution", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableHeadersRow();

				auto sortSpecs = ImGui::TableGetSortSpecs();

				if (sortSpecs && sortSpecs->SpecsCount > 0)
				{
					std::sort(factoryInfos.begin(), factoryInfos.end(), [sortSpecs](const FactoryInfo& left, const FactoryInfo& right)
					{
						for (int n = 0; n < sortSpecs->SpecsCount; n++)
						{
							const ImGuiTableColumnSortSpecs* sortSpec = &sortSpecs->Specs[n];
							int delta = 0;
							switch (sortSpec->ColumnIndex)
							{
							case 0:
								delta = SortCompare(left.name, right.name);
								break;
							case 1:
								delta = SortCompare(left.count, right.count);
								break;
							case 2:
								delta = SortCompare(left.usage, right.usage);
								break;
							}
							if (delta > 0)
							{
								return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
							}
							else if (delta < 0)
							{
								return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
							}
						}

						return left.count > right.count;
					});
				}

				for (const auto& factoryInfo : factoryInfos)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(factoryInfo.name.c_str());

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("%d", factoryInfo.count);

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%.02f%%", factoryInfo.usage);
				}

				ImGui::EndTable();
			}

			static char filterText[512];
			ImGui::InputText("Filter", filterText, sizeof(filterText));

			std::vector<FactoryBucket> factoryBuckets;
			for (const auto& factoryBucketKvp : factoryBucketsLookup)
			{
				factoryBuckets.push_back(factoryBucketKvp.second);
			}

			if (ImGui::BeginTable("##archetypebuckets", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Bucket Name");
				ImGui::TableSetupColumn("Num Archetypes Defined", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_DefaultSort);
				ImGui::TableHeadersRow();

				auto sortSpecs = ImGui::TableGetSortSpecs();

				if (sortSpecs && sortSpecs->SpecsCount > 0)
				{
					std::sort(factoryBuckets.begin(), factoryBuckets.end(), [sortSpecs](const FactoryBucket& left, const FactoryBucket& right)
					{
						for (int n = 0; n < sortSpecs->SpecsCount; n++)
						{
							const ImGuiTableColumnSortSpecs* sortSpec = &sortSpecs->Specs[n];
							int delta = 0;
							switch (sortSpec->ColumnIndex)
							{
							case 0:
								delta = SortCompare(left.name, right.name);
								break;
							case 1:
								delta = SortCompare(left.count, right.count);
								break;
							}
							if (delta > 0)
							{
								return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
							}
							else if (delta < 0)
							{
								return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
							}
						}

						return left.count > right.count;
					});
				}

				for (const auto& factoryBucket : factoryBuckets)
				{
					if (factoryBucket.name.find(filterText) != std::string::npos)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text(factoryBucket.name.c_str());

						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%d", factoryBucket.count);
					}
				}

				ImGui::EndTable();
			}

		}

		ImGui::End();
	});
});
