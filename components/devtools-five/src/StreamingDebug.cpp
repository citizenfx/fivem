#include "StdInc.h"
#include <ConsoleHost.h>

#include <imgui.h>

#include <Streaming.h>

static InitFunction initFunction([]()
{
	static bool streamingDebugEnabled;

	ConHost::OnInvokeNative.Connect([](const char* native, const char* arg)
	{
		if (_stricmp(native, "streamingDebug") == 0)
		{
			streamingDebugEnabled = !streamingDebugEnabled;
		}
	});

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || streamingDebugEnabled;
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