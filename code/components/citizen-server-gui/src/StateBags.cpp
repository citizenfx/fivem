#include <StdInc.h>
#include <GameServer.h>
#include <UdpInterceptor.h>
#include <ServerGui.h>
#include <imgui.h>
#include "ResourceManager.h"
#include "StateBagComponent.h"

static fwRefContainer<fx::StateBagComponent> g_stateBagComponent = nullptr;
static fwRefContainer<fx::ClientRegistry> g_clientRegistry = nullptr;
static std::string selectedStateBagId;
static int selectedStateBagIndex = -1;
static char searchFilter[128] = "";

void RenderRegisteredTargets();
void RenderPreCreatePrefixes();
void RenderRateLimitingDebug();
void RenderStateBagsList();

void RenderStateBagDetails()
{
	if (selectedStateBagId.empty())
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No StateBag selected");
        return;
    }

    auto details = g_stateBagComponent->GetStateBagDetails(selectedStateBagId);
    if (!details.has_value())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "StateBag not found: %s", selectedStateBagId.c_str());
        return;
    }

    const auto& d = details.value();

    ImGui::Text("StateBag ID: %s", d.id.c_str());
    
    if (d.isExpired)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[EXPIRED]");
    }

    ImGui::Separator();

    ImGui::Text("Use Parent Targets: %s", d.useParentTargets ? "Yes" : "No");
    ImGui::Text("Replication Enabled: %s", d.replicationEnabled ? "Yes" : "No");
    
    if (d.owningPeer.has_value())
    {
        ImGui::Text("Owning Peer: %d", d.owningPeer.value());
    }
    else
    {
        ImGui::Text("Owning Peer: None");
    }

    if (ImGui::CollapsingHeader("Routing Targets", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (d.routingTargets.empty())
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  No routing targets");
        }
        else
        {
            for (int target : d.routingTargets)
            {
            	auto client = g_clientRegistry->GetClientBySlotID(target);
                ImGui::Text("  Target: %d", client->GetNetId());
            }
        }
    }

    if (ImGui::CollapsingHeader("Stored Data", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (d.storedData.empty())
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  No stored data");
        }
        else
        {
            for (const auto& [key, value] : d.storedData)
            {
                if (ImGui::TreeNode(key.c_str()))
                {
                    ImGui::TextWrapped("%s", value.c_str());
                    ImGui::TreePop();
                }
            }
        }
    }

    if (ImGui::CollapsingHeader("Queued Replication Data"))
    {
        if (d.queuedData.empty())
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "  No queued data");
        }
        else
        {
            ImGui::Text("  Keys pending replication:");
            for (const auto& [key, size] : d.queuedData)
            {
                ImGui::Text("    %s (%zu bytes)", key.c_str(), size);
            }

            ImGui::Spacing();
            if (ImGui::Button("Send Queued Updates"))
            {
                g_stateBagComponent->SendStateBagQueuedUpdates(d.id);
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Force send all queued updates");
        }
    }
}

void RenderRegisteredTargets()
{
	if (ImGui::CollapsingHeader("Registered Targets"))
	{
		auto targets = g_stateBagComponent->GetRegisteredTargets();
    
		if (targets.empty())
		{
			ImGui::Text("No targets registered");
		}
		else
		{
			ImGui::Columns(2, "TargetsColumns");
			ImGui::Text("Target ID");
			ImGui::NextColumn();
			ImGui::Text("StateBags Count");
			ImGui::NextColumn();
			ImGui::Separator();
        
			for (int target : targets)
			{
				ImGui::Text("%d", target);
				ImGui::NextColumn();
            
				size_t count = g_stateBagComponent->GetStateBagCountForTarget(target);
				ImGui::Text("%zu", count);
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
	}
}

void RenderPreCreatePrefixes()
{
	if (ImGui::CollapsingHeader("Pre-create Prefixes"))
	{
		auto prefixes = g_stateBagComponent->GetPreCreatePrefixes();
	    
		if (prefixes.empty())
		{
			ImGui::Text("No pre-create prefixes registered");
		}
		else
		{
			ImGui::Columns(2, "PrefixColumns");
			ImGui::Text("Prefix");
			ImGui::NextColumn();
			ImGui::Text("Use Parent Targets");
			ImGui::NextColumn();
			ImGui::Separator();
	        
			for (const auto& [prefix, useParentTargets] : prefixes)
			{
				ImGui::Text("%s", prefix.c_str());
				ImGui::NextColumn();
				ImGui::Text("%s", useParentTargets ? "Yes" : "No");
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
	}
}

void RenderRateLimitingDebug()
{
	if (ImGui::CollapsingHeader("Rate Limiting Debug", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Rate Limit Settings:");
		ImGui::Text("  Rate: %u updates/second", g_stateBagComponent->GetRateLimitRate());
		ImGui::Text("  Burst: %u max burst", g_stateBagComponent->GetRateLimitBurst());
		
		ImGui::Separator();
		
		auto rateLimitInfo = g_stateBagComponent->GetRateLimitInfo();
		
		if (rateLimitInfo.empty())
		{
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No rate limiting data available");
			ImGui::Spacing();
		}
		else
		{
			ImGui::Text("Clients with StateBag Activity:");
			ImGui::Separator();
			
			ImGui::Columns(6, "RateLimitColumns");
			ImGui::Text("Client ID");
			ImGui::NextColumn();
			ImGui::Text("Current Rate");
			ImGui::NextColumn();
			ImGui::Text("Burst Count");
			ImGui::NextColumn();
			ImGui::Text("Dropped Updates");
			ImGui::NextColumn();
			ImGui::Text("Last Dropped StateBag");
			ImGui::NextColumn();
			ImGui::Text("Actions");
			ImGui::NextColumn();
			ImGui::Separator();
			
			for (const auto& info : rateLimitInfo)
			{
				if (info.droppedUpdates > 0)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
				}
				else if (info.currentRate > g_stateBagComponent->GetRateLimitRate() * 0.8f)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
				}

				auto client = g_clientRegistry->GetClientBySlotID(info.clientId);
				
				ImGui::Text("%d", client->GetNetId());
				ImGui::NextColumn();
				
				ImGui::Text("%u/s", info.currentRate);
				ImGui::NextColumn();
				
				ImGui::Text("%u", info.burstCount);
				ImGui::NextColumn();
				
				if (info.droppedUpdates > 0)
				{
					ImGui::Text("%u ", info.droppedUpdates);
				}
				else
				{
					ImGui::Text("0");
				}
				ImGui::NextColumn();
				
				if (!info.lastDroppedStateBag.empty())
				{
					ImGui::Text("%s", info.lastDroppedStateBag.c_str());
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						auto timeSinceDrop = std::chrono::duration_cast<std::chrono::seconds>(
							std::chrono::steady_clock::now() - info.lastDropTime).count();
						ImGui::Text("Dropped %ld seconds ago", timeSinceDrop);
						ImGui::EndTooltip();
					}
				}
				else
				{
					ImGui::Text("-");
				}
				ImGui::NextColumn();
				
				ImGui::PushID(info.clientId);
				if (ImGui::SmallButton("Reset"))
				{
					g_stateBagComponent->ResetClientRateLimit(info.clientId);
				}
				ImGui::PopID();
				ImGui::NextColumn();
				
				if (info.droppedUpdates > 0 || info.currentRate > g_stateBagComponent->GetRateLimitRate() * 0.8f)
				{
					ImGui::PopStyleColor();
				}
			}
			
			ImGui::Columns(1);
			
			ImGui::Separator();
			for (const auto& info : rateLimitInfo)
			{
				if (info.droppedUpdates > 0 || !info.recentStateBags.empty())
				{
					std::string headerName = "Client " + std::to_string(info.clientId);
					if (info.droppedUpdates > 0)
					{
						headerName += " DROPPING UPDATES";
					}
					
					if (ImGui::TreeNode(headerName.c_str()))
					{
						if (info.droppedUpdates > 0)
						{
							ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), 
								"This client has %u dropped updates!", info.droppedUpdates);
							ImGui::Text("Last dropped StateBag: %s", info.lastDroppedStateBag.c_str());
							
							auto timeSinceDrop = std::chrono::duration_cast<std::chrono::seconds>(
								std::chrono::steady_clock::now() - info.lastDropTime).count();
							ImGui::Text("Time since last drop: %ld seconds", timeSinceDrop);
							ImGui::Separator();
						}
						
						if (!info.recentStateBags.empty())
						{
							ImGui::Text("Recent StateBag Activity:");
							for (size_t i = 0; i < info.recentStateBags.size(); ++i)
							{
								const auto& bagId = info.recentStateBags[i];
								ImGui::Text("  %zu. %s", i + 1, bagId.c_str());
								
								if (ImGui::IsItemClicked())
								{
									selectedStateBagId = bagId;
									auto stateBagInfoList = g_stateBagComponent->GetStateBagInfoList();
									for (int idx = 0; idx < stateBagInfoList.size(); ++idx)
									{
										if (stateBagInfoList[idx].id == bagId)
										{
											selectedStateBagIndex = idx;
											break;
										}
									}
								}
								
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("Click to select this StateBag for detailed view");
								}
							}
						}
						
						ImGui::TreePop();
					}
				}
			}
		}
	}
}

void RenderStateBagsList()
{
	if (ImGui::CollapsingHeader("StateBags", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputText("Search Filter", searchFilter, sizeof(searchFilter));
		ImGui::Separator();
	    
		auto stateBagInfoList = g_stateBagComponent->GetStateBagInfoList();
	    
		ImGui::BeginChild("StateBagsList", ImVec2(0, 200), true);
	    
		int index = 0;
		for (const auto& info : stateBagInfoList)
		{
			if (strlen(searchFilter) > 0 && info.id.find(searchFilter) == std::string::npos)
			{
				index++;
				continue;
			}
	        
			bool isSelected = (selectedStateBagIndex == index);
	        
			if (info.isExpired)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
	        
			std::string displayText = info.id + (info.isExpired ? " (EXPIRED)" : "");
			if (ImGui::Selectable(displayText.c_str(), isSelected))
			{
				selectedStateBagIndex = index;
				selectedStateBagId = info.id;
			}
	        
			if (info.isExpired)
				ImGui::PopStyleColor();
	        
			index++;
		}
	    
		ImGui::EndChild();
	}
}

static SvGuiModule stateBags("State Bags", "svstatebags", ImGuiWindowFlags_MenuBar, [](fx::ServerInstanceBase* instance)
{
	g_clientRegistry = instance->GetComponent<fx::ClientRegistry>();
	g_stateBagComponent = fx::ResourceManager::GetCurrent()->GetComponent<fx::StateBagComponent>();

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Actions"))
		{
			if (ImGui::MenuItem("Clear Orphaned StateBags"))
			{
				g_stateBagComponent->ClearOrphanedStateBags();
			}
				
			if (ImGui::MenuItem("Force Send Queued Updates"))
			{
				g_stateBagComponent->ForceStateBagUpdates();
			}

			ImGui::Separator();
			
			if (ImGui::MenuItem("Reset All Rate Limits"))
			{
				auto rateLimitInfo = g_stateBagComponent->GetRateLimitInfo();
				for (const auto& info : rateLimitInfo)
				{
					g_stateBagComponent->ResetClientRateLimit(info.clientId);
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	
	if (ImGui::CollapsingHeader("General Information", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Role: %s", g_stateBagComponent->GetRoleName());
		ImGui::Text("Total StateBags: %zu", g_stateBagComponent->GetStateBagCount());
		ImGui::Text("Active StateBags: %zu", g_stateBagComponent->GetActiveStateBagCount());
		ImGui::Text("Expired StateBags: %zu", g_stateBagComponent->GetExpiredStateBagCount());
		ImGui::Text("Registered Targets: %zu", g_stateBagComponent->GetTargetCount());
		ImGui::Text("Pre-created StateBags: %zu", g_stateBagComponent->GetPreCreatedStateBagCount());
		ImGui::Text("StateBags in Erasure List: %zu", g_stateBagComponent->GetErasureListCount());
		ImGui::Text("Pre-create Prefixes: %zu", g_stateBagComponent->GetPreCreatePrefixCount());
		ImGui::Text("Game Interface: %s", g_stateBagComponent->IsGameInterfaceConnected() ? "Connected" : "NULL");
		
		// Rate limiting alert
		auto rateLimitInfo = g_stateBagComponent->GetRateLimitInfo();
		uint32_t totalDropped = 0;
		uint32_t clientsWithDrops = 0;
		
		for (const auto& info : rateLimitInfo)
		{
			totalDropped += info.droppedUpdates;
			if (info.droppedUpdates > 0)
				clientsWithDrops++;
		}
		
		if (totalDropped > 0)
		{
			ImGui::Separator();
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "RATE LIMITING ACTIVE");
			ImGui::Text("Clients with dropped updates: %u", clientsWithDrops);
			ImGui::Text("Total dropped updates: %u", totalDropped);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Check Rate Limiting Debug section below!");
		}
	}
	
	RenderRegisteredTargets();
	RenderPreCreatePrefixes();
	RenderRateLimitingDebug();
	RenderStateBagsList();
	RenderStateBagDetails();
});