#include "StdInc.h"
#include <ConsoleHost.h>

#include <imgui.h>

#include <Streaming.h>

#include <grcTexture.h>
#include <CoreConsole.h>

#include <VFSManager.h>
#include <VFSWin32.h>

#include <wrl.h>
#include <wincodec.h>

using Microsoft::WRL::ComPtr;

#include <imguilistview.h>

class StreamingListView : public ImGui::ListViewBase
{
public:
	inline StreamingListView(streaming::Manager* streaming)
		: m_streaming(streaming)
	{
		m_indexMap.resize(streaming->numEntries);
	}

	inline void updateCount(const char* filterText, bool requested, bool loading, bool loaded)
	{
		m_indexMap.clear();

		for (int i = 0; i < m_streaming->numEntries; i++)
		{
			if (m_streaming->Entries[i].handle)
			{
				int state = m_streaming->Entries[i].flags & 3;

				if (!state ||
					(state == 1 && !loaded) ||
					(state == 2 && !requested) ||
					(state == 3 && !loading))
				{
					continue;
				}

				if (!filterText[0] || strstr(streaming::GetStreamingNameForIndex(i).c_str(), filterText) != nullptr)
				{
					m_indexMap.push_back(i);
				}
			}
		}
	}

	virtual size_t getNumColumns() const override;
	virtual size_t getNumRows() const override;

	virtual void getHeaderData(size_t column, HeaderData& headerDataOut) const override;
	virtual void getCellData(size_t row, size_t column, CellData& cellDataOut) const override;

private:
	streaming::Manager* m_streaming;

	std::vector<size_t> m_indexMap;
};

// TODO: share this code somewhere
#pragma comment(lib, "windowscodecs.lib")

ComPtr<IWICImagingFactory> g_imagingFactory;

static void* MakeIcon(const std::string& filename)
{
	if (!g_imagingFactory)
	{
		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)g_imagingFactory.GetAddressOf());
	}

	ComPtr<IWICBitmapDecoder> decoder;

	auto s = vfs::OpenRead(filename);

	if (!s.GetRef())
	{
		return nullptr;
	}

	ComPtr<IStream> stream = vfs::CreateComStream(s);

	HRESULT hr = g_imagingFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		ComPtr<IWICBitmapFrameDecode> frame;

		hr = decoder->GetFrame(0, frame.GetAddressOf());

		if (SUCCEEDED(hr))
		{
			ComPtr<IWICBitmapSource> source;
			ComPtr<IWICBitmapSource> convertedSource;

			UINT width = 0, height = 0;

			frame->GetSize(&width, &height);

			// try to convert to a pixel format we like
			frame.As(&source);

			hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, source.Get(), convertedSource.GetAddressOf());

			if (SUCCEEDED(hr))
			{
				source = convertedSource;
			}

			// create a pixel data buffer
			uint32_t* pixelData = new uint32_t[width * height];

			hr = source->CopyPixels(nullptr, width * 4, width * height * 4, reinterpret_cast<BYTE*>(pixelData));

			if (SUCCEEDED(hr))
			{
				rage::grcTextureReference reference;
				memset(&reference, 0, sizeof(reference));
				reference.width = width;
				reference.height = height;
				reference.depth = 1;
				reference.stride = width * 4;
				reference.format = 11; // should correspond to DXGI_FORMAT_B8G8R8A8_UNORM
				reference.pixelData = (uint8_t*)pixelData;

				auto iconPtr = new void*[2];
				iconPtr[0] = rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
				iconPtr[1] = nullptr;
				return iconPtr;
			}
		}
	}

	return nullptr;
}

static std::map<uint32_t, void*> MakeIcons()
{
	return {
		{ HashString("Archive"), MakeIcon("citizen:/resources/icons/rpf.png") },
		{ HashString("CutSceneStore"), MakeIcon("citizen:/resources/icons/cube.png") },
		{ HashString("ScaleformStore"), MakeIcon("citizen:/resources/icons/gfx.png") },
		{ HashString("Paths"), MakeIcon("citizen:/resources/icons/node.png") },
		{ HashString("wptrec"), MakeIcon("citizen:/resources/icons/recording.png") },
		{ HashString("carrec"), MakeIcon("citizen:/resources/icons/recording.png") },
		{ HashString("AnimStore"), MakeIcon("citizen:/resources/icons/animation.png") },
		{ HashString("NetworkDefStore"), MakeIcon("citizen:/resources/icons/move.png") },
		{ HashString("StaticBounds"), MakeIcon("citizen:/resources/icons/bounds.png") },
		{ HashString("ScriptStore"), MakeIcon("citizen:/resources/icons/script.png") },
		{ HashString("PoseMatcherStore"), MakeIcon("citizen:/resources/icons/pose.png") },
		{ HashString("MetaDataStore"), MakeIcon("citizen:/resources/icons/xmlfile.png") },
		{ HashString("MapDataStore"), MakeIcon("citizen:/resources/icons/instance.png") },
		{ HashString("MapTypesStore"), MakeIcon("citizen:/resources/icons/class.png") },
		{ HashString("TxdStore"), MakeIcon("citizen:/resources/icons/image.png") },
		{ HashString("PtFxAssetStore"), MakeIcon("citizen:/resources/icons/effect.png") },
		{ HashString("ClothStore"), MakeIcon("citizen:/resources/icons/cloth.png") },
		{ HashString("NavMeshes"), MakeIcon("citizen:/resources/icons/navigationpath.png") },
		{ HashString("HeightMeshes"), MakeIcon("citizen:/resources/icons/navigationpath.png") }, // funny unused store
		{ HashString("DrawableStore"), MakeIcon("citizen:/resources/icons/cube.png") },
		{ HashString("DwdStore"), MakeIcon("citizen:/resources/icons/dict.png") },
		{ HashString("FragmentStore"), MakeIcon("citizen:/resources/icons/part.png") },
		{ HashString("ModelInfo"), MakeIcon("citizen:/resources/icons/modelthreed.png") },
	};
}

static std::map<uint32_t, ImGui::ListViewBase::CellData::IconData> MakeIconDatas(const std::map<uint32_t, void*>& icons)
{
	std::map<uint32_t, ImGui::ListViewBase::CellData::IconData> ids;

	for (auto& [ key, tex ] : icons)
	{
		ids.emplace(key, ImGui::ListViewBase::CellData::IconData{ ImTextureID{ tex }, ImVec2{ 0.0f, 0.0f }, ImVec2{ 1.0f, 1.0f }, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f } });
	}

	return ids;
}

static std::string GetStreamingModuleName(streaming::strStreamingModule* module)
{
	auto name = (atArray<char>*)(((char*)module) + 24);
	return &name->Get(0);
}

void StreamingListView::getCellData(size_t row, size_t column, CellData& cellDataOut) const
{
	// ok, got i
	auto num = m_indexMap[row];

	auto& entry = m_streaming->Entries[num];
	auto strModule = m_streaming->moduleMgr.GetStreamingModule(num);
	auto relativeIndex = (num - strModule->baseIdx);

	static std::map<uint32_t, void*> icons = MakeIcons();
	static std::map<uint32_t, CellData::IconData> iconDatas = MakeIconDatas(icons);

	switch (column)
	{
	case 0:
	{
		cellDataOut.fieldPtr = &iconDatas[HashString(GetStreamingModuleName(strModule))];

		break;
	}
	case 1:
	{
		auto type = GetStreamingModuleName(strModule);

		cellDataOut.customText = va("%s", type);
		break;
	}
	case 2:
	{
		cellDataOut.customText = va("%d (+%d)", relativeIndex, strModule->baseIdx);
		break;
	}
	case 3:
	{
		cellDataOut.customText = va("%s", streaming::GetStreamingNameForIndex(num));
		break;
	}
	case 4:
		cellDataOut.fieldPtr = &entry.flags;
		break;
	case 5:
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
		headerDataOut.name = "";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_ICON;
		headerDataOut.formatting.columnWidth = 36;
		break;
	case 1:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "Type";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_CUSTOM;
		break;
	case 2:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "Index";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_CUSTOM;
		break;
	case 3:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "Name";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_CUSTOM;
		break;
	case 4:
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
		headerDataOut.formatting.columnWidth = 200;
		break;
	case 5:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "Refs";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_INT;
		headerDataOut.formatting.columnWidth = 64;
		break;
	}
}

size_t StreamingListView::getNumColumns() const
{
	return 6;
}

size_t StreamingListView::getNumRows() const
{
	return m_indexMap.size();
}

DLL_EXPORT size_t CountDependencyMemory(streaming::Manager* streaming, uint32_t strIdx)
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

	static ConVar<bool> streamingDebugVar("strdbg", ConVar_Archive | ConVar_UserPref, false, &streamingDebugEnabled);
	static ConVar<bool> streamingListVar("strlist", ConVar_Archive | ConVar_UserPref, false, &streamingListEnabled);
	static ConVar<bool> streamingMemoryVar("strmem", ConVar_Archive | ConVar_UserPref, false, &streamingMemoryEnabled);

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

		auto streaming = streaming::Manager::GetInstance();
		auto streamingAllocator = rage::strStreamingAllocator::GetInstance();

		ImGui::SetNextWindowSize(ImVec2(500.0f, 300.0f), ImGuiCond_FirstUseEver);

		struct StreamingMemoryInfo
		{
			uint32_t idx;
			size_t physicalMemory;
			size_t virtualMemory;
		};

		auto avMem = 0x40000000;//streamingAllocator->GetMemoryAvailable()

		if (ImGui::Begin("Streaming Memory", &streamingMemoryEnabled))
		{
			static std::vector<StreamingMemoryInfo> entryList;
			entryList.reserve(streaming->numEntries);
			entryList.resize(0); // basically: invalidate counter

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

					if (info.virtualMemory > 8192 || info.physicalMemory > 8192)
					{
						entryList.push_back(info);
					}

					if (!streaming->IsObjectReadyToDelete(i, 0xF1 | 8))
					{
						lockedMem += info.virtualMemory;
					}

					if ((entry.flags >> 16) & 0x10)
					{
						flag10Mem += info.virtualMemory;

						flag10Mem += CountDependencyMemory(streaming, i);
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

			ImGui::Text("grcore/ResourceCache usage: %.2f / %.2f MiB",
				rage::grcResourceCache::GetInstance()->GetUsedPhysicalMemory() / 1024.0 / 1024.0,
				rage::grcResourceCache::GetInstance()->GetTotalPhysicalMemory() / 1024.0 / 1024.0);

			if (ImGui::BeginTable("##strmem", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Virt", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Phys", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Total", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_DefaultSort);
				ImGui::TableHeadersRow();

				{
					auto sortSpecs = ImGui::TableGetSortSpecs();

					if (sortSpecs && sortSpecs->SpecsCount > 0)
					{
						std::sort(entryList.begin(), entryList.end(), [sortSpecs](const StreamingMemoryInfo& left, const StreamingMemoryInfo& right)
						{
							auto compare = [](const auto& left, const auto& right)
							{
								if (left < right)
									return -1;

								if (left > right)
									return 1;

								return 0;
							};

							for (int n = 0; n < sortSpecs->SpecsCount; n++)
							{
								const ImGuiTableColumnSortSpecs* sortSpec = &sortSpecs->Specs[n];
								int delta = 0;
								switch (sortSpec->ColumnIndex)
								{
									case 0:
										delta = compare(streaming::GetStreamingNameForIndex(left.idx), streaming::GetStreamingNameForIndex(right.idx));
										break;
									case 1:
										delta = compare(left.virtualMemory, right.virtualMemory);
										break;
									case 2:
										delta = compare(left.physicalMemory, right.physicalMemory);
										break;
									case 3:
										delta = compare(left.virtualMemory + left.physicalMemory, right.virtualMemory + right.physicalMemory);
										break;
								}
								if (delta > 0)
									return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
								if (delta < 0)
									return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
							}

							return left.idx < right.idx;
						});
					}
				}

				ImGuiListClipper clipper;
				clipper.Begin(entryList.size());

				while (clipper.Step())
				{
					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
					{
						const auto& item = entryList[row];
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%s", streaming::GetStreamingNameForIndex(item.idx).c_str());

						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%.2f MiB", item.virtualMemory / 1024.0 / 1024.0);

						ImGui::TableSetColumnIndex(2);
						ImGui::Text("%.2f MiB", item.physicalMemory / 1024.0 / 1024.0);

						ImGui::TableSetColumnIndex(3);
						ImGui::Text("%.2f MiB", (item.virtualMemory + item.physicalMemory) / 1024.0 / 1024.0);
					}
				}

				ImGui::EndTable();
			}
		}

		ImGui::End();
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!streamingListEnabled)
		{
			return;
		}

		auto streaming = streaming::Manager::GetInstance();

		ImGui::SetNextWindowSize(ImVec2(500.0f, 300.0f), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Streaming List", &streamingListEnabled))
		{
			static StreamingListView lv(streaming);

			static uint32_t lastTime;

			static char filterText[512];
			static char lastFilterText[512];
			static bool loading = true;
			static bool lastLoading;
			static bool loaded = true;
			static bool lastLoaded;
			static bool requested = true;
			static bool lastRequested;

			ImGui::InputText("Filter", filterText, sizeof(filterText));
			ImGui::SameLine();
			ImGui::Checkbox("Req'd", &requested);
			ImGui::SameLine();
			ImGui::Checkbox("Loading", &loading);
			ImGui::SameLine();
			ImGui::Checkbox("Loaded", &loaded);

			ImGui::BeginChild("InnerRegion");
			if ((GetTickCount() - lastTime) > 1500 || strcmp(filterText, lastFilterText) != 0 ||
				requested != lastRequested || loaded != lastLoaded || loading != lastLoading)
			{
				lv.updateCount(filterText, requested, loading, loaded);
				memcpy(lastFilterText, filterText, sizeof(filterText));
				lastRequested = requested;
				lastLoaded = loaded;
				lastLoading = loading;

				lastTime = GetTickCount();
			}

			lv.render();

			ImGui::EndChild();
		}

		ImGui::End();
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!streamingDebugEnabled)
		{
			return;
		}

		auto streaming = streaming::Manager::GetInstance();
		
		if (ImGui::Begin("Streaming Stats", &streamingDebugEnabled))
		{
			ImGui::Text("Pending requests: %d", streaming->NumPendingRequests);

			ImGui::Text("Pending prio requests: %d", streaming->NumPendingRequestsPrio);

			ImGui::Text("Pending resource requests: %d", streaming->NumPendingRequests3);

			std::vector<uint32_t> ifrs;
			int ifr = 0;

			auto es = streaming->Entries;

			if (es)
			{
				for (int i = 0; i < streaming->numEntries; i++)
				{
					auto& entry = es[i];

					if ((entry.flags & 3) == 3) // loading
					{
						ifrs.push_back(i);
						ifr++;
					}
				}
			}

			ImGui::Text("In-Flight Requests: %d", ifr);

			auto push = [streaming](uint32_t idx)
			{
				auto entryName = streaming::GetStreamingNameForIndex(idx);
				auto data = &streaming->Entries[idx];

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
			};

			if (ImGui::CollapsingHeader("IFRs"))
			{
				for (uint32_t entry : ifrs)
				{
					push(entry);
				}
			}

			if (ImGui::CollapsingHeader("Request list"))
			{
				for (const auto* entry = streaming->RequestListHead; entry; entry = entry->Next)
				{
					push(entry->Index);
				}
			}
		}

		ImGui::End();
	});
});
