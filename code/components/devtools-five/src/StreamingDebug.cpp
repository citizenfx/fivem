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

static rage::grcTexture* MakeIcon(const std::string& filename)
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

				return rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
			}
		}
	}

	return nullptr;
}

static std::map<uint32_t, rage::grcTexture*> MakeIcons()
{
	return {
		{ HashString("rage::strPackfileStreamingModule"), MakeIcon("citizen:/resources/icons/rpf.png") },
		{ HashString("CCutsceneStore"), MakeIcon("citizen:/resources/icons/cube.png") },
		{ HashString("CScaleformStore"), MakeIcon("citizen:/resources/icons/gfx.png") },
		{ HashString("CPathFind"), MakeIcon("citizen:/resources/icons/node.png") },
		{ HashString("CWaypointRecordingStreamingInterface"), MakeIcon("citizen:/resources/icons/recording.png") },
		{ HashString("CVehicleRecordingStreamingModule"), MakeIcon("citizen:/resources/icons/recording.png") },
		{ HashString("rage::fwClipDictionaryStore"), MakeIcon("citizen:/resources/icons/animation.png") },
		{ HashString("rage::fwNetworkDefStore"), MakeIcon("citizen:/resources/icons/move.png") },
		{ HashString("rage::fwStaticBoundsStore"), MakeIcon("citizen:/resources/icons/bounds.png") },
		{ HashString("CStreamedScripts"), MakeIcon("citizen:/resources/icons/script.png") },
		{ HashString("rage::fwPoseMatcherStore"), MakeIcon("citizen:/resources/icons/pose.png") },
		{ HashString("rage::fwMetaDataStore"), MakeIcon("citizen:/resources/icons/xmlfile.png") },
		{ HashString("rage::fwMapDataStore"), MakeIcon("citizen:/resources/icons/instance.png") },
		{ HashString("rage::fwMapTypesStore"), MakeIcon("citizen:/resources/icons/class.png") },
		{ HashString("rage::fwTxdStore"), MakeIcon("citizen:/resources/icons/image.png") },
		{ HashString("rage::ptfxAssetStore"), MakeIcon("citizen:/resources/icons/effect.png") },
		{ HashString("rage::fwClothStore"), MakeIcon("citizen:/resources/icons/cloth.png") },
		{ HashString("rage::aiNavMeshStore"), MakeIcon("citizen:/resources/icons/navigationpath.png") },
		{ HashString("rage::fwDrawableStore"), MakeIcon("citizen:/resources/icons/cube.png") },
		{ HashString("rage::fwDwdStore"), MakeIcon("citizen:/resources/icons/dict.png") },
		{ HashString("rage::fwFragmentStore"), MakeIcon("citizen:/resources/icons/part.png") },
		{ HashString("CModelInfoStreamingModule"), MakeIcon("citizen:/resources/icons/modelthreed.png") },
	};
}

static std::map<uint32_t, ImGui::ListViewBase::CellData::IconData> MakeIconDatas(const std::map<uint32_t, rage::grcTexture*>& icons)
{
	std::map<uint32_t, ImGui::ListViewBase::CellData::IconData> ids;

	for (auto& [ key, tex ] : icons)
	{
		ids.emplace(key, ImGui::ListViewBase::CellData::IconData{ ImTextureID{ tex }, ImVec2{ 0.0f, 0.0f }, ImVec2{ 1.0f, 1.0f }, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f } });
	}

	return ids;
}

void StreamingListView::getCellData(size_t row, size_t column, CellData& cellDataOut) const
{
	// ok, got i
	auto num = m_indexMap[row];

	auto& entry = m_streaming->Entries[num];
	auto strModule = m_streaming->moduleMgr.GetStreamingModule(num);
	auto relativeIndex = (num - strModule->baseIdx);

	static std::map<uint32_t, rage::grcTexture*> icons = MakeIcons();
	static std::map<uint32_t, CellData::IconData> iconDatas = MakeIconDatas(icons);

	switch (column)
	{
	case 0:
	{
		cellDataOut.fieldPtr = &iconDatas[HashString(&typeid(*strModule).name()[6])];

		break;
	}
	case 1:
	{
		// icon
		// ^ not yet

		auto type = std::string(typeid(*strModule).name());

		cellDataOut.customText = va("%s", type.substr(6));
		break;
	}
	case 2:
	{
		const auto& entryName = streaming::GetStreamingNameForIndex(num);

		cellDataOut.customText = entryName.c_str();
		break;
	}
	case 3:
		cellDataOut.fieldPtr = &entry.flags;
		break;
	case 4:
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
		headerDataOut.name = "Name";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_CUSTOM;
		break;
	case 3:
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
	case 4:
		headerDataOut.sorting.sortable = false;
		headerDataOut.name = "Refs";
		headerDataOut.type.headerType = ImGui::ListViewBase::HT_INT;
		headerDataOut.formatting.columnWidth = 64;
		break;
	}
}

size_t StreamingListView::getNumColumns() const
{
	return 5;
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

		static bool streamingStatsOpen;

		auto streaming = streaming::Manager::GetInstance();
		
		if (ImGui::Begin("Streaming Stats", &streamingStatsOpen))
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
