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
		refCount = strModule->GetRefCount(relativeIndex);

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
				*out = "LOADING_2";
				return true;
			case 3:
				*out = "LOADING_3";
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

static InitFunction initFunction([]()
{
	static bool streamingDebugEnabled;
	static bool streamingListEnabled;

	static ConVar<bool> streamingDebugVar("strdbg", ConVar_Archive, false, &streamingDebugEnabled);
	static ConVar<bool> streamingListVar("strlist", ConVar_Archive, false, &streamingListEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || streamingDebugEnabled || streamingListEnabled;
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

			ImGui::End();
		}
	});
});
