#include <StdInc.h>
#include <CoreConsole.h>
#include <ConsoleHost.h>
#include <GameInit.h>
#include <tinyxml2.h>
#include <TimecycleEditor.h>
#include <HostSharedData.h>
#include <PureModeState.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_API DLL_IMPORT

#include <imgui.h>
#include <imgui_internal.h>

#include <boost/algorithm/string/find.hpp>

inline const char* istrstr(const char* haystack, const char* needle)
{
	auto result = boost::ifind_first(haystack, needle);
	return result ? result.begin() : nullptr; 
}

void SelectedTimecycle::RestoreVar(int index)
{
	if (const auto modifier = GetModifier())
	{
		if (const auto it = m_cachedMods.find(index); it != m_cachedMods.end())
		{
			for (auto& modData : modifier->m_modData)
			{
				if (modData.m_index == index)
				{
					modData.m_value1 = it->second.m_value1;
					modData.m_value2 = it->second.m_value2;
				}
			}
		}
	}
}

void SelectedTimecycle::RemoveVar(int index)
{
	if (auto& it = m_cachedMods.find(index); it != m_cachedMods.end())
	{
		m_cachedMods.erase(index);
		m_disabled.erase(index);
	}
}

void SelectedTimecycle::DisableVar(int index)
{
	if (!IsVarDisabled(index))
	{
		m_disabled.insert(index);
	}
}

void SelectedTimecycle::EnableVar(int index)
{
	if (IsVarDisabled(index))
	{
		m_disabled.erase(index);
	}
}

void SelectedTimecycle::UpdateVars()
{
	if (const auto modifier = GetModifier())
	{
		for (auto& modData : modifier->m_modData)
		{
			if (auto& it = m_cachedMods.find(modData.m_index); it == m_cachedMods.end())
			{
				m_cachedMods.insert({ modData.m_index, modData });
			}
		}
	}
}

bool SelectedTimecycle::IsVarDisabled(int index)
{
	return m_disabled.find(index) != m_disabled.end();
}

rage::tcModData* SelectedTimecycle::GetVar(int index)
{
	if (const auto it = m_cachedMods.find(index); it != m_cachedMods.end())
	{
		return &it->second;
	}

	return nullptr;
}

void SelectedTimecycle::SetModifier(rage::tcModifier* mod)
{
	if (mod != GetModifier() && GetModifier() != nullptr)
	{
		for (auto index : m_disabled)
		{
			RestoreVar(index);
		}
	}

	ResetModifier();

	m_selection = mod;

	UpdateVars();
}

void SelectedTimecycle::ResetModifier()
{
	m_disabled.clear();
	m_cachedMods.clear();
	m_selection = nullptr;
}

std::map<int, rage::tcModData>& SelectedTimecycle::GetVars()
{
	return m_cachedMods;
}

bool SelectedTimecycle::HasModifier()
{
	return GetModifier() != nullptr;
}

rage::tcModifier* SelectedTimecycle::GetModifier()
{
	if (m_selection != nullptr && m_selection->m_modData.m_offset == nullptr)
	{
		ResetModifier();
	}

	return m_selection;
}


// copy pasted from "CloneDebug.cpp"
static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

bool TimecycleEditor::ShouldDraw()
{
	return m_editorEnabled || TheTimecycleManager->ShouldActivateEditor();
}

void TimecycleEditor::Draw()
{
	if (!m_editorEnabled && TheTimecycleManager->ShouldActivateEditor())
	{
		m_editorEnabled = true;
		TheTimecycleManager->SetActivateEditor(false);
	}

	auto tcVarInfos = TimecycleManager::GetConfigVarInfos();
	if (!tcVarInfos)
	{
		return;
	}

	auto tcManager = TimecycleManager::GetGameManager();
	if (!tcManager)
	{
		return;
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(1020.0f, 400.0f), ImVec2(1920.0f, 2000.0f));

	if (ImGui::Begin("Timecycle Editor", &m_editorEnabled))
	{
		static float treeSize = 400.f;
		static float detailSize = 600.f;

		Splitter(true, 8.0f, &treeSize, &detailSize, 8.0f, 8.0f);

		if (ImGui::BeginChild("tree", ImVec2(treeSize, -1.0f), true))
		{
			// controls
			ImGui::Text("Search timecycle:");

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, ImGui::CalcTextSize("By param ").x);

			ImGui::Text("By name");
			ImGui::SameLine();
			ImGui::NextColumn();

			if (ImGui::InputText("##controls_search_name", m_searchNameBuffer, kMaxBufferSize))
			{
				RebuildSearchCache();
			}

			ImGui::NextColumn();

			ImGui::Text("By param");
			ImGui::SameLine();
			ImGui::NextColumn();

			if (ImGui::InputText("##controls_search_param", m_searchParamBuffer, kMaxBufferSize))
			{
				RebuildSearchCache();
			}

			ImGui::NextColumn();

			ImGui::Separator();
			ImGui::EndColumns();

			if (auto scriptData = TimecycleManager::GetScriptData())
			{
				if (scriptData->m_primaryModifierIndex != -1)
				{
					auto modifier = TheTimecycleManager->GetTimecycleByIndex(scriptData->m_primaryModifierIndex);
					auto tcName = modifier ? TheTimecycleManager->GetTimecycleName(*modifier).c_str() : "INVALID";
					ImGui::Text("Primary modifier: %s", tcName);

					ImGui::DragFloat(va("##primary-strength"), &scriptData->m_primaryModifierStrength, 0.05f, -10.0f, 10.0f, "%.3f", 1.0f);

					ImGui::SameLine();

					if (ImGui::Button("Clear"))
					{
						scriptData->m_primaryModifierIndex = -1;
					}
				}
				else
				{
					ImGui::Text("Primary modifier: NONE");
				}

				ImGui::Separator();

				if (scriptData->m_transitionModifierIndex != -1)
				{
					auto modifier = TheTimecycleManager->GetTimecycleByIndex(scriptData->m_transitionModifierIndex);
					auto tcName = modifier ? TheTimecycleManager->GetTimecycleName(*modifier).c_str() : "INVALID";
					ImGui::Text("Transition modifier: %s", tcName);

					ImGui::DragFloat(va("##transition-strength"), &scriptData->m_transitionModifierStrength, 0.05f, -10.0f, 10.0f, "%.3f", 1.0f);

					ImGui::SameLine();

					if (ImGui::Button("Clear"))
					{
						scriptData->m_transitionModifierIndex = -1;
					}
				}
				else
				{
					ImGui::Text("Transition modifier: NONE");
				}

#ifdef GTA_FIVE
				ImGui::Separator();

				if (scriptData->m_extraModifierIndex != -1)
				{
					auto modifier = TheTimecycleManager->GetTimecycleByIndex(scriptData->m_extraModifierIndex);
					auto tcName = modifier ? TheTimecycleManager->GetTimecycleName(*modifier).c_str() : "INVALID";
					ImGui::Text("Extra modifier: %s", tcName);

					ImGui::SameLine();

					if (ImGui::Button("Clear"))
					{
						scriptData->m_extraModifierIndex = -1;
					}
				}
				else
				{
					ImGui::Text("Extra modifier: NONE");
				}
#endif

				ImGui::Separator();

				if (ImGui::Checkbox("Apply selected", &m_applySelectedTimecycle))
				{
					if (m_applySelectedTimecycle)
					{
						if (auto modifier = m_selectedModifier.GetModifier())
						{
							scriptData->m_primaryModifierIndex = TheTimecycleManager->GetTimecycleIndex(*modifier);
						}
					}
				}
			}

			if (ImGui::BeginChild("list", ImVec2(-1.0f, -1.0f), false))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize());

				bool drawnAny = false;
				bool useSearch = ShouldUseSearchCache();

				for (auto modifier : tcManager->m_modifiers)
				{
					if (!modifier)
					{
						continue;
					}

					if (useSearch && m_searchCache.find(modifier->m_nameHash) == m_searchCache.end())
					{
						continue;
					}

					bool isSelected = (m_selectedModifier.GetModifier() == modifier);
					auto& modifierName = TheTimecycleManager->GetTimecycleName(*modifier);

					if (ImGui::TreeNodeEx(modifier,
						ImGuiTreeNodeFlags_Leaf | ((isSelected) ? ImGuiTreeNodeFlags_Selected : 0),
						"%s", modifierName.c_str()))
					{
						if (ImGui::IsItemClicked())
						{
							g_timecycleXmlOutput.ClearBuffer();
							TheTimecycleManager->EnsureTimecycleBackup(*modifier);
							m_selectedModifier.SetModifier(modifier);
							strcpy(m_detailNameBuffer, modifierName.c_str());

							if (m_applySelectedTimecycle)
							{
								auto modifier = m_selectedModifier.GetModifier();
								auto scriptData = TimecycleManager::GetScriptData();

								if (scriptData != nullptr && modifier != nullptr)
								{
									scriptData->m_primaryModifierIndex = TheTimecycleManager->GetTimecycleIndex(*modifier);
								}
							}
						}

						ImGui::TreePop();
					}

					drawnAny = true;
				}

				ImGui::PopStyleVar();

				if (!drawnAny && (strlen(m_searchNameBuffer) > 0 || strlen(m_searchParamBuffer) > 0))
				{
					ImGui::Text("No timecycles found");

					if (ImGui::Button("Clear search query"))
					{
						memset(m_searchNameBuffer, 0, kMaxBufferSize);
						memset(m_searchParamBuffer, 0, kMaxBufferSize);
						RebuildSearchCache();
					}
				}
			}

			ImGui::EndChild();
		}

		ImGui::EndChild();
		ImGui::SameLine();

		bool showOutput = g_timecycleXmlOutput.CStrSize() > 1;

		if (showOutput && ImGui::BeginChild("output", ImVec2(-1.0f, -1.0f), true))
		{
			if (ImGui::Button("Close"))
			{
				g_timecycleXmlOutput.ClearBuffer();
			}

			ImGui::SameLine();
			ImGui::Text("Select and copy using CTRL+C");
			ImGui::Separator();

			auto output = g_timecycleXmlOutput.CStr();
			ImGui::InputTextMultiline("##xml_output", (char*)output, g_timecycleXmlOutput.CStrSize(), ImVec2(-1.0f, -1.0f));
		}
		else if (ImGui::BeginChild("detail", ImVec2(-1.0f, -1.0f), true))
		{
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.0f);

			ImGui::Text("Name");
			ImGui::SameLine();
			ImGui::NextColumn();

			ImGui::InputText("##detail_name", m_detailNameBuffer, kMaxBufferSize);

			if (auto tc = m_selectedModifier.GetModifier())
			{
				ImGui::SameLine();
				ImGui::Text("(index: %d)", TheTimecycleManager->GetTimecycleIndex(*tc));
			}

			ImGui::NextColumn();

			ImGui::Text("Controls");
			ImGui::SameLine();
			ImGui::NextColumn();

			if (ImGui::Button("Create"))
			{
				auto nameLength = strlen(m_detailNameBuffer);

				if (nameLength >= 2 && nameLength <= 128)
				{
					auto newName = std::string(m_detailNameBuffer);

					auto created = TheTimecycleManager->CreateTimecycle(newName);
					if (created != nullptr)
					{
						m_selectedModifier.SetModifier(created);
						memset(m_detailNameBuffer, 0, kMaxBufferSize);
					}
				}
			}

			auto modifier = m_selectedModifier.GetModifier();

			if (modifier != nullptr)
			{
				ImGui::SameLine();

				if (ImGui::Button("Rename"))
				{
					auto nameLength = strlen(m_detailNameBuffer);

					if (nameLength >= 2 && nameLength <= 128)
					{
						auto newName = std::string(m_detailNameBuffer);
						auto newHash = HashString(newName);

						TheTimecycleManager->RenameTimecycle(*modifier, newName);
					}
				}

				ImGui::SameLine();

				if (ImGui::Button("Clone"))
				{
					auto nameLength = strlen(m_detailNameBuffer);

					if (nameLength >= 2 && nameLength <= 128)
					{
						auto cloned = TheTimecycleManager->CloneTimecycle(*modifier, std::string(m_detailNameBuffer));
						if (cloned != nullptr)
						{
							m_selectedModifier.SetModifier(cloned);
							memset(m_detailNameBuffer, 0, kMaxBufferSize);
						}
					}
				}

				ImGui::SameLine();

				if (ImGui::Button("Delete"))
				{
					TheTimecycleManager->RemoveTimecycle(*modifier);
					m_selectedModifier.ResetModifier();
				}

				ImGui::SameLine();

				if (ImGui::Button("Generate XML"))
				{
					auto timecycle = m_selectedModifier.GetModifier();
					auto tcName = TheTimecycleManager->GetTimecycleName(*timecycle).c_str();

					tinyxml2::XMLDocument document;

					auto rootElement = document.NewElement("timecycle_modifier_data");
					rootElement->SetAttribute("version", "1.000000");

					auto modElement = document.NewElement("modifier");
					modElement->SetAttribute("name", tcName);
#if IS_RDR3
					modElement->SetAttribute("description", "");
#endif
					modElement->SetAttribute("numMods", timecycle->m_modData.GetCount());
					modElement->SetAttribute("userFlags", timecycle->m_userFlags);

					for (auto& modData : timecycle->m_modData)
					{
						if (auto varInfo = TimecycleManager::GetConfigVarInfo(modData.m_index))
						{
							if (auto varName = TimecycleManager::GetVarInfoName(*varInfo))
							{
								auto varElement = document.NewElement(varName);
								varElement->SetText(va("%.3f %.3f", modData.m_value1, modData.m_value2));
								modElement->InsertEndChild(varElement);	
							}
						}
					}

					rootElement->InsertEndChild(modElement);
					document.InsertEndChild(rootElement);

					g_timecycleXmlOutput.ClearBuffer();
					g_timecycleXmlOutput.PushHeader(false, true);
					document.Print(&g_timecycleXmlOutput);
				}

				if (m_selectedModifier.HasModifier()) // as it may be deleted at this point
				{
					m_selectedModifier.UpdateVars();

#if 0
					ImGui::NextColumn();

					ImGui::Text("User flags");
					ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::InputInt("##detail_flags_param", (int*)&modifier->m_userFlags);
#endif

					ImGui::NextColumn();
					ImGui::Text("Filter");
					ImGui::SameLine();

					ImGui::NextColumn();
					ImGui::InputText("##detail_filter_param", m_detailFilterBuffer, kMaxBufferSize);
					ImGui::SameLine();

					if (ImGui::Button("Clear"))
					{
						memset(m_detailFilterBuffer, 0, kMaxBufferSize);
					}

					if (auto scriptData = TimecycleManager::GetScriptData())
					{
						ImGui::NextColumn();
						ImGui::Text("Apply");
						ImGui::SameLine();

						ImGui::NextColumn();

						if (ImGui::Button("As primary"))
						{
							scriptData->m_primaryModifierIndex = TheTimecycleManager->GetTimecycleIndex(*modifier);
						}

						ImGui::SameLine();

						if (ImGui::Button("As transition"))
						{
							scriptData->m_transitionModifierIndex = TheTimecycleManager->GetTimecycleIndex(*modifier);
							scriptData->m_transitionModifierSpeed = 0.1f;
							scriptData->m_transitionModifierStrength = 0.0f;
						}

#ifdef GTA_FIVE
						ImGui::SameLine();

						if (ImGui::Button("As extra"))
						{
							scriptData->m_extraModifierIndex = TheTimecycleManager->GetTimecycleIndex(*modifier);
						}
#endif
					}

					ImGui::NextColumn();
					ImGui::Text("Variable");
					ImGui::SameLine();

					ImGui::NextColumn();
					ImGui::InputText("##input", m_detailDropDownBuffer, kMaxBufferSize);
					ImGui::SameLine();

					static bool isOpen = false;
					bool isFocused = ImGui::IsItemFocused();
					isOpen |= ImGui::IsItemActive();

					if (isOpen)
					{
						ImGui::SetNextWindowPos({ ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y });
						ImGui::SetNextWindowSize({ ImGui::GetItemRectSize().x, 450.0f });

						if (ImGui::Begin("##popup", &isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_Tooltip))
						{
							ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
							isFocused |= ImGui::IsWindowFocused();

							for (int i = 0; i < TimecycleManager::GetConfigVarInfoCount(); i++)
							{
								auto varName = TimecycleManager::GetVarInfoName(tcVarInfos[i]);

								if (!varName || istrstr(varName, m_detailDropDownBuffer) == nullptr)
								{
									continue;
								}

								bool addedParam = false;

								// do not add params that already in the list of this timecycle
								for (auto modData : modifier->m_modData)
								{
									if (modData.m_index == tcVarInfos[i].m_index)
									{
										addedParam = true;
										break;
									}
								}

								if (addedParam)
								{
									continue;
								}

								if (ImGui::Selectable(varName) || (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter)))
								{
									strcpy(m_detailDropDownBuffer, varName);
									isOpen = false;
								}
							}
						}

						ImGui::End();
						isOpen &= isFocused;
					}

					if (ImGui::Button("Add"))
					{
						if (TheTimecycleManager->AddTimecycleModData(*modifier, std::string(m_detailDropDownBuffer)))
						{
							memset(m_detailDropDownBuffer, 0, kMaxBufferSize);
						}
					}

					ImGui::SameLine();

					if (ImGui::Button("Clear"))
					{
						memset(m_detailDropDownBuffer, 0, kMaxBufferSize);
					}

					ImGui::Separator();

					bool drawnAny = false;
					// delayed mod data removing queue to not mutate map when drawing stuff
					std::set<int> removeQueue;

					ImGui::Columns(1);
					if (ImGui::BeginTable("##proptable", 6, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable))
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Var");
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("Name");
						ImGui::TableSetColumnIndex(2);
						ImGui::Text("Default");
						ImGui::TableSetColumnIndex(3);
						ImGui::Text("Value 1");
						ImGui::TableSetColumnIndex(4);
						ImGui::Text("Value 2");
						ImGui::TableSetColumnIndex(5);
						ImGui::Text("");

						ImGui::TableNextRow();

						for (auto& [index, modData] : m_selectedModifier.GetVars())
						{
							if (modData.m_index == -1 || modData.m_index >= TimecycleManager::GetConfigVarInfoCount())
							{
								continue; // invalid?
							}

							auto& varInfo = tcVarInfos[index];
							auto varName = TimecycleManager::GetVarInfoName(varInfo);

							if (!varName || (strlen(m_detailFilterBuffer) > 0 && istrstr(varName, m_detailFilterBuffer) == nullptr))
							{
								continue;
							}

							bool isSearched = (strlen(m_searchParamBuffer) > 0 && istrstr(varName, m_searchParamBuffer) != nullptr);

							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%d", varInfo.m_index);

							ImGui::TableSetColumnIndex(1);

							if (isSearched)
							{
								ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(81, 179, 236, 255));
							}

							ImGui::Text("%s", varName);

							if (isSearched)
							{
								ImGui::PopStyleColor();
							}

							ImGui::TableSetColumnIndex(2);
							ImGui::Text("%f", varInfo.m_value);
							ImGui::TableSetColumnIndex(3);

							bool nulled = (varInfo.m_value == 0.0f);
							float min = nulled ? -1.0f : ((varInfo.m_value > 0.0f) ? -varInfo.m_value : varInfo.m_value) * 10.0f;
							float max = nulled ? 1.0f : ((varInfo.m_value > 0.0f) ? varInfo.m_value : -varInfo.m_value) * 10.0f;
							float step = nulled ? 0.02f : (varInfo.m_value / 20.0f);

							bool isVarEnabled = !m_selectedModifier.IsVarDisabled(index);
							auto actualData = TheTimecycleManager->GetTimecycleModData(*modifier, index);
							auto dataSource = (isVarEnabled) ? actualData : &modData;

							if (!isVarEnabled)
							{
								ImGui::BeginDisabled();
							}

							if (ImGui::DragFloat(va("##%x-%d-1", modifier->m_nameHash, index), &modData.m_value1, step, min, max, "%.3f", 1.0f))
							{
								if (dataSource == actualData)
								{
									actualData->m_value1 = modData.m_value1;
								}
							}

							ImGui::TableSetColumnIndex(4);

							if (ImGui::DragFloat(va("##%x-%d-2", modifier->m_nameHash, index), &modData.m_value2, step, min, max, "%.3f", 1.0f))
							{
								if (dataSource == actualData)
								{
									actualData->m_value2 = modData.m_value2;
								}
							}

							ImGui::TableSetColumnIndex(5);

							if (!isVarEnabled)
							{
								ImGui::EndDisabled();
							}

							if (ImGui::Checkbox(va("##%x-%d-check", modifier->m_nameHash, index), &isVarEnabled))
							{
								if (isVarEnabled)
								{
									if (TheTimecycleManager->AddTimecycleModData(*modifier, std::string(varName)))
									{
										m_selectedModifier.EnableVar(index);

										auto addedData = TheTimecycleManager->GetTimecycleModData(*modifier, index);
										addedData->m_value1 = modData.m_value1;
										addedData->m_value2 = modData.m_value2;
									}
								}
								else
								{
									if (TheTimecycleManager->RemoveTimecycleModData(*modifier, modData))
									{
										m_selectedModifier.DisableVar(index);
									}
								}
							}

							ImGui::SameLine();

							if (ImGui::Button(va("Delete##%x-%d-del", modifier->m_nameHash, index)))
							{
								if (TheTimecycleManager->RemoveTimecycleModData(*modifier, std::string(varName)))
								{
									removeQueue.insert(varInfo.m_index);
								}
							}

							ImGui::NextColumn();

							drawnAny = true;
						}

						ImGui::EndTable();
					}

					ImGui::Columns();

					if (!drawnAny && strlen(m_detailFilterBuffer) > 0)
					{
						ImGui::Text("No parameters found");

						if (ImGui::Button("Clear filter query"))
						{
							memset(m_detailFilterBuffer, 0, kMaxBufferSize);
						}
					}

					for (auto index : removeQueue)
					{
						m_selectedModifier.RemoveVar(index);
					}
				}
			}
			else
			{
				ImGui::Separator();
			}
		}

		ImGui::EndChild();
	}

	ImGui::End();
}

void TimecycleEditor::Reset()
{
	m_selectedModifier.ResetModifier();

	memset(m_searchNameBuffer, 0, kMaxBufferSize);
	memset(m_searchParamBuffer, 0, kMaxBufferSize);
	memset(m_detailNameBuffer, 0, kMaxBufferSize);
	memset(m_detailFilterBuffer, 0, kMaxBufferSize);
	memset(m_detailDropDownBuffer, 0, kMaxBufferSize);

	m_editorEnabled = false;
	m_applySelectedTimecycle = false;
	g_timecycleXmlOutput.ClearBuffer();
	m_searchCache.clear();
}

bool TimecycleEditor::ShouldUseSearchCache()
{
	return strlen(m_searchNameBuffer) > 0 || strlen(m_searchParamBuffer) > 0;
}

void TimecycleEditor::RebuildSearchCache()
{
	m_searchCache.clear();

	if (!ShouldUseSearchCache())
	{
		return;
	}

	const auto tcManager = TimecycleManager::GetGameManager();
	if (!tcManager)
	{
		return;
	}

	for (const auto modifier : tcManager->m_modifiers)
	{
		if (!modifier)
		{
			continue;
		}

		bool isSelected = (m_selectedModifier.GetModifier() == modifier);

		if (!isSelected)
		{
			if (strlen(m_searchNameBuffer) > 0)
			{
				auto& modifierName = TheTimecycleManager->GetTimecycleName(*modifier);

				if (istrstr(modifierName.c_str(), m_searchNameBuffer) == nullptr)
				{
					continue;
				}
			}

			if (strlen(m_searchParamBuffer) > 0)
			{
				bool hasModData = TheTimecycleManager->DoesTimecycleHasModData(*modifier, std::string(m_searchParamBuffer), true);

				if (!hasModData)
				{
					continue;
				}
			}
		}

		m_searchCache.insert(modifier->m_nameHash);
	}
}

static InitFunction initFunction([]()
{
#ifndef _DEBUG
	if (fx::client::GetPureLevel() > 0)
	{
		return;
	}
#endif

	static TimecycleEditor TCEditor;

#ifndef _DEBUG
	if (launch::IsSDK() || launch::IsSDKGuest())
#endif
	{
		static ConVar<bool> timecycleEditorVar("timecycleeditor", ConVar_Archive | ConVar_UserPref, false, &TCEditor.m_editorEnabled);
	}

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || TCEditor.ShouldDraw();
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!Instance<ICoreGameInit>::Get()->GetGameLoaded() || Instance<ICoreGameInit>::Get()->HasVariable("gameKilled"))
		{
			return;
		}

		if (TCEditor.ShouldDraw())
		{
			TCEditor.Draw();
		}
	});

	OnKillNetworkDone.Connect([=]()
	{
		TCEditor.Reset();
	});
});
