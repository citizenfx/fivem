#include "StdInc.h"
#include <ConsoleHost.h>

#include <imgui.h>

#include <Streaming.h>

#include <CoreConsole.h>

#include <imguilistview.h>

class StreamingListView : public ImGui::ListViewBase
{
public:
	inline StreamingListView(streaming::Manager* streaming)
		: m_streaming(streaming)
	{

	}

	inline void updateCount()
	{
		size_t num = 0;

		for (int i = 0; i < m_streaming->numEntries; i++)
		{
			if (m_streaming->Entries[i].handle && m_streaming->Entries[i].flags & 3)
			{
				m_indexMap[num] = i;
				num++;
			}
		}
	}

	virtual size_t getNumColumns() const override;
	virtual size_t getNumRows() const override;

	virtual void getHeaderData(size_t column, HeaderData& headerDataOut) const override;
	virtual void getCellData(size_t row, size_t column, CellData& cellDataOut) const override;

private:
	streaming::Manager* m_streaming;

	std::unordered_map<size_t, size_t> m_indexMap;
};

void StreamingListView::getCellData(size_t row, size_t column, CellData& cellDataOut) const
{
	// ok, got i
	auto it = m_indexMap.find(row);
	auto num = it->second;

	auto& entry = m_streaming->Entries[num];
	auto strModule = m_streaming->moduleMgr.GetStreamingModule(num);
	auto relativeIndex = (num - strModule->baseIdx);

	switch (column)
	{
	case 0:
	{
		// icon
		// ^ not yet

		auto type = std::string(typeid(*strModule).name());

		cellDataOut.customText = va("%s", type.substr(6));
		break;
	}
	case 1:
	{
		const auto& entryName = streaming::GetStreamingNameForIndex(num);

		cellDataOut.customText = entryName.c_str();
		break;
	}
	case 2:
		cellDataOut.fieldPtr = &entry.flags;
		break;
	case 3:
	{
		static int refCount;
		refCount = strModule->GetNumRefs(relativeIndex);

		cellDataOut.fieldPtr = &refCount;
	}
	}

	static bool dummy;
	cellDataOut.selectedRowPtr = &dummy;
}

void StreamingListView::getHeaderData(size_t column, HeaderData& headerDataOut) const
{
	headerDataOut.editing.editable = false;

	switch (column)
	{
	case 0:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "Type";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_CUSTOM;
		break;
	case 1:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "Name";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_CUSTOM;
		break;
	case 2:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "State";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_ENUM;
		headerDataOut.type.textFromEnumFunctionPointer = [](void*, int type, const char** out)
		{
			switch (type & 3)
			{
			case 0:
				*out = "UNLOADED";
				return true;
			case 1:
				*out = "LOADED";
				return true;
			case 2:
				*out = "REQUESTED";
				return true;
			case 3:
				*out = "LOADING";
				return true;
			}

			return false;
		};
		break;
	case 3:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "Refs";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_INT;
		break;
	}
}

size_t StreamingListView::getNumColumns() const
{
	return 4;
}

size_t StreamingListView::getNumRows() const
{
	return m_indexMap.size();
}

size_t CountDependencyMemory(streaming::Manager* streaming, uint32_t strIdx)
{
	auto thisModule = streaming->moduleMgr.GetStreamingModule(strIdx);

	size_t memory = streaming->Entries[strIdx].ComputeVirtualSize(strIdx, nullptr, 0);

	uint32_t depList[32];
	int numDeps = thisModule->GetDependencies(strIdx - thisModule->baseIdx, depList, sizeof(depList) / sizeof(*depList));

	for (int dep = 0; dep < numDeps; dep++)
	{
		memory += CountDependencyMemory(streaming, depList[dep]);
	}

	return memory;
}

static InitFunction initFunction([]()
{
	static bool streamingDebugEnabled;
	static bool streamingListEnabled;
	static bool streamingMemoryEnabled;

	static ConVar<bool> streamingDebugVar("strdbg", ConVar_Archive, false, &streamingDebugEnabled);
	static ConVar<bool> streamingListVar("strlist", ConVar_Archive, false, &streamingListEnabled);
	static ConVar<bool> streamingMemoryVar("strmem", ConVar_Archive, false, &streamingMemoryEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || streamingDebugEnabled || streamingListEnabled || streamingMemoryEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!streamingMemoryEnabled)
		{
			return;
		}

		static bool streamingMemoryOpen;

		auto streaming = streaming::Manager::GetInstance();
		auto streamingAllocator = rage::strStreamingAllocator::GetInstance();

		ImGui::SetNextWindowSize(ImVec2(500.0f, 300.0f), ImGuiCond_FirstUseEver);

		struct StreamingMemoryInfo : ImGui::ListView::ItemBase
		{
			uint32_t idx;
			size_t physicalMemory;
			size_t virtualMemory;

			virtual const char* getCustomText(size_t column) const
			{
				switch (column)
				{
				case 0:
					return streaming::GetStreamingNameForIndex(idx).c_str();
				case 1:
					return va("%.2f MiB", virtualMemory / 1024.0 / 1024.0);
				case 2:
					return va("%.2f MiB", physicalMemory / 1024.0 / 1024.0);
				}
			}
			
			virtual const void* getDataPtr(size_t column) const
			{
				switch (column)
				{
				case 0:
					return &idx;
				case 1:
					return &virtualMemory;
				case 2:
					return &physicalMemory;
				}
			}
		};

		auto avMem = 0x40000000;//streamingAllocator->GetMemoryAvailable()

		if (ImGui::Begin("Streaming Memory", &streamingMemoryOpen))
		{
			static std::vector<StreamingMemoryInfo> entryList(streaming->numEntries);
			entryList.resize(streaming->numEntries);

			int entryIdx = 0;
			size_t lockedMem = 0;
			size_t flag10Mem = 0;
			size_t usedMem = 0;
			size_t usedPhys = 0;

			for (int i = 0; i < streaming->numEntries; i++)
			{
				auto& entry = streaming->Entries[i];

				if ((entry.flags & 3) == 1) // loaded
				{
					StreamingMemoryInfo info;
					info.idx = i;
					info.virtualMemory = entry.ComputeVirtualSize(i, nullptr, false);
					info.physicalMemory = entry.ComputePhysicalSize(i);

					if (entryIdx < entryList.size())
					{
						entryList[entryIdx] = info;

						entryIdx++;
					}

					if (!streaming->IsObjectReadyToDelete(i, 0xF1 | 8))
					{
						lockedMem += info.virtualMemory;
					}

					if ((entry.flags >> 16) & 0x10)
					{
						flag10Mem += info.virtualMemory;

						flag10Mem += CountDependencyMemory(streaming, i);
						// wrong way around
						/*atArray<uint32_t> depArray;
						streaming->FindAllDependents(depArray, i);

						for (uint32_t dependent : depArray)
						{
							flag10Mem += streaming->Entries[dependent].ComputeVirtualSize(dependent, nullptr, false);
						}*/
					}

					usedMem += info.virtualMemory;
					usedPhys += info.physicalMemory;
				}
			}

			ImGui::Text("Virtual memory: %.2f MiB",
				usedMem / 1024.0 / 1024.0);

			ImGui::Text("Locked virtual memory: %.2f MiB (%d%%)", 
				lockedMem / 1024.0 / 1024.0,
				int((lockedMem / (double)usedMem) * 100.0));

			ImGui::Text("Script virtual memory: %.2f MiB (%d%%)",
				flag10Mem / 1024.0 / 1024.0,
				int((flag10Mem / (double)usedMem) * 100.0));

			ImGui::Text("Physical memory: %.2f MiB", usedPhys / 1024.0 / 1024.0);

			static ImGui::ListView lv;

			if (lv.headers.empty())
			{
				lv.headers.push_back(ImGui::ListView::Header{ "Name", "", ImGui::ListViewBase::HT_CUSTOM, 0, 16.0f });
				lv.headers.push_back(ImGui::ListView::Header{ "Virt", "", ImGui::ListViewBase::HT_CUSTOM, 0, 16.0f });
				lv.headers.push_back(ImGui::ListView::Header{ "Phys", "", ImGui::ListViewBase::HT_CUSTOM, 0, 16.0f });
			}

			lv.items.clear();

			for (int i = 0; i < entryIdx; i++)
			{
				lv.items.push_back(&entryList[i]);
			}

			std::sort(lv.items.begin(), lv.items.end(), [](const ImGui::ListView::ItemBase* leftPtr, const ImGui::ListView::ItemBase* rightPtr)
			{
				auto& left = *(const StreamingMemoryInfo*)leftPtr;
				auto& right = *(const StreamingMemoryInfo*)rightPtr;

				return (left.physicalMemory + left.virtualMemory) > (right.physicalMemory + right.virtualMemory);
			});

			lv.render();
		}

		ImGui::End();
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!streamingListEnabled)
		{
			return;
		}

		static bool streamingStatsOpen;

		auto streaming = streaming::Manager::GetInstance();

		ImGui::SetNextWindowSize(ImVec2(500.0f, 300.0f), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Streaming List", &streamingStatsOpen))
		{
			static StreamingListView lv(streaming);

			static uint32_t lastTime;

			if ((GetTickCount() - lastTime) > 5000)
			{
				lv.updateCount();

				lastTime = GetTickCount();
			}

			lv.render();
		}

		ImGui::End();
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!streamingDebugEnabled)
		{
			return;
		}

		static bool streamingStatsOpen;

		auto streaming = streaming::Manager::GetInstance();
		
		if (ImGui::Begin("Streaming Stats", &streamingStatsOpen))
		{
			ImGui::Text("Pending requests: %d", streaming->NumPendingRequests);

			ImGui::Text("Pending prio requests: %d", streaming->NumPendingRequestsPrio);

			ImGui::Text("Pending resource requests: %d", streaming->NumPendingRequests3);

			if (ImGui::CollapsingHeader("Request list"))
			{
				for (const auto* entry = streaming->RequestListHead; entry; entry = entry->Next)
				{
					auto entryName = streaming::GetStreamingNameForIndex(entry->Index);
					auto data = &streaming->Entries[entry->Index];

					// try getting streaming data
					StreamingPackfileEntry* spf = streaming::GetStreamingPackfileForEntry(data);

					if (spf)
					{
						if (entryName.empty() && spf->packfile)
						{
							rage::fiCollection* collection = (rage::fiCollection*)spf->packfile;

							char nameBuffer[256] = { 0 };
							collection->GetEntryNameToBuffer(data->handle & 0xFFFF, nameBuffer, 255);

							entryName = nameBuffer;
						}

						if (!entryName.empty())
						{
							// is this a networked packfile?
							if (!spf->isHdd)
							{
								entryName += " [NET]";
							}

							// is this a known packfile?
							if (spf->packfile)
							{
								const char* name = spf->packfile->GetName();

								// strip from any / at the end
								const char* lastChar = strrchr(name, '/');

								if (lastChar)
								{
									name = lastChar + 1;
								}

								entryName += std::string(" (") + name + ")";
							}
						}
					}

					if (!entryName.empty())
					{
						// replace .y* with .#*
						auto yPos = entryName.find(".y");

						if (yPos != std::string::npos)
						{
							entryName = entryName.substr(0, yPos) + ".#" + entryName.substr(yPos + 2);
						}

						// add to imgui
						ImGui::Selectable(entryName.c_str());
					}
				}
			}
		}

		ImGui::End();
	});
});
