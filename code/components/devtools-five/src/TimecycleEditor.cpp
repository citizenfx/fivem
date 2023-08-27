#include <StdInc.h>
#include <CoreConsole.h>
#include <ConsoleHost.h>
#include <GameInit.h>
#include <HostSharedData.h>
#include <tinyxml2.h>
#include <TimecycleEditor.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_API DLL_IMPORT

#include <imgui.h>
#include <imgui_internal.h>

#include <boost/algorithm/string/find.hpp>

const char* istrstr(const char* haystack, const char* needle)
{
	auto result = boost::ifind_first(haystack, needle);
	return result ? result.begin() : NULL; 
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
	if (auto it = m_cachedMods.find(index); it != m_cachedMods.end())
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

static SelectedTimecycle CurrentTimecycle;

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

static InitFunction initFunction([]()
{
#ifndef _DEBUG
	if (!launch::IsSDK() && !launch::IsSDKGuest())
	{
		return;
	}
#endif

	static constexpr size_t kMaxBufferSize = 256;
	static char g_editorSearchNameBuffer[kMaxBufferSize];
	static char g_editorSearchParamBuffer[kMaxBufferSize];
	static char g_editorDetailNameBuffer[kMaxBufferSize];
	static char g_editorDetailFilterBuffer[kMaxBufferSize];
	static char g_editorDetailDropDownBuffer[kMaxBufferSize];
	static bool g_editorApplySelectedTimecycle = false;
	static tinyxml2::XMLPrinter g_editorTimecycleOutput;

	static bool timecycleEditorEnabled;
	static ConVar<bool> timecycleEditorVar("timecycleeditor", ConVar_Archive | ConVar_UserPref, false, &timecycleEditorEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || timecycleEditorEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!timecycleEditorEnabled)
		{
			return;
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

		if (!Instance<ICoreGameInit>::Get()->GetGameLoaded() || Instance<ICoreGameInit>::Get()->HasVariable("gameKilled"))
		{
			return;
		}

		ImGui::SetNextWindowSizeConstraints(ImVec2(1020.0f, 400.0f), ImVec2(1920.0f, 2000.0f));

		if (ImGui::Begin("Timecycle Editor", &timecycleEditorEnabled))
		{
			static float treeSize = 400.f;
			static float detailSize = 600.f;

			Splitter(true, 8.0f, &treeSize, &detailSize, 8.0f, 8.0f);

			if (ImGui::BeginChild("tree", ImVec2(treeSize, -1.0f), true))
			{
				// controls
				ImGui::Text("Search timecycle:");

				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 90.0f);

				ImGui::Text("By name");
				ImGui::SameLine();
				ImGui::NextColumn();

				ImGui::InputText("##controls_search_name", g_editorSearchNameBuffer, kMaxBufferSize);
				ImGui::NextColumn();

				ImGui::Text("By param");
				ImGui::SameLine();
				ImGui::NextColumn();

				ImGui::InputText("##controls_search_param", g_editorSearchParamBuffer, kMaxBufferSize);
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

					ImGui::Separator();

					if (ImGui::Checkbox("Apply selected", &g_editorApplySelectedTimecycle))
					{
						if (g_editorApplySelectedTimecycle)
						{
							if (auto modifier = CurrentTimecycle.GetModifier())
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

					for (auto modifier : tcManager->m_modifiers)
					{
						if (!modifier)
						{
							continue;
						}

						bool isSelected = (CurrentTimecycle.GetModifier() == modifier);
						auto& modifierName = TheTimecycleManager->GetTimecycleName(*modifier);

						if (!isSelected)
						{
							if (strlen(g_editorSearchNameBuffer) > 0 && istrstr(modifierName.c_str(), g_editorSearchNameBuffer) == NULL)
							{
								continue;
							}

							if (strlen(g_editorSearchParamBuffer) > 0)
							{
								bool hasModData = TheTimecycleManager->DoesTimecycleHasModData(*modifier, std::string(g_editorSearchParamBuffer), true);

								if (!hasModData)
								{
									continue;
								}
							}
						}

						if (ImGui::TreeNodeEx(modifier,
							ImGuiTreeNodeFlags_Leaf | ((isSelected) ? ImGuiTreeNodeFlags_Selected : 0),
							"%s", modifierName.c_str()))
						{
							if (ImGui::IsItemClicked())
							{
								g_editorTimecycleOutput.ClearBuffer();
								TheTimecycleManager->EnsureTimecycleBackup(*modifier);
								CurrentTimecycle.SetModifier(modifier);
								strcpy(g_editorDetailNameBuffer, modifierName.c_str());

								if (g_editorApplySelectedTimecycle)
								{
									auto modifier = CurrentTimecycle.GetModifier();
									auto scriptData = TimecycleManager::GetScriptData();

									if (scriptData && modifier != nullptr)
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

					if (!drawnAny && (strlen(g_editorSearchNameBuffer) > 0 || strlen(g_editorSearchParamBuffer) > 0))
					{
						ImGui::Text("No timecycles found");

						if (ImGui::Button("Clear search query"))
						{
							memset(g_editorSearchNameBuffer, 0, kMaxBufferSize);
							memset(g_editorSearchParamBuffer, 0, kMaxBufferSize);
						}
					}
				}

				ImGui::EndChild();
			}

			ImGui::EndChild();
			ImGui::SameLine();

			bool showOutput = g_editorTimecycleOutput.CStrSize() > 1;

			if (showOutput && ImGui::BeginChild("output", ImVec2(-1.0f, -1.0f), true))
			{
				if (ImGui::Button("Close"))
				{
					g_editorTimecycleOutput.ClearBuffer();
				}

				ImGui::SameLine();
				ImGui::Text("Select and copy using CTRL+C");
				ImGui::Separator();

				auto output = g_editorTimecycleOutput.CStr();
				ImGui::InputTextMultiline("##xml_output", (char*)output, g_editorTimecycleOutput.CStrSize(), ImVec2(-1.0f, -1.0f));
			}
			else if (ImGui::BeginChild("detail", ImVec2(-1.0f, -1.0f), true))
			{
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 100.0f);

				ImGui::Text("Name");
				ImGui::SameLine();
				ImGui::NextColumn();

				ImGui::InputText("##detail_name", g_editorDetailNameBuffer, kMaxBufferSize);

				if (auto tc = CurrentTimecycle.GetModifier())
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
					auto nameLength = strlen(g_editorDetailNameBuffer);

					if (nameLength >= 2 && nameLength <= 128)
					{
						auto newName = std::string(g_editorDetailNameBuffer);

						auto created = TheTimecycleManager->CreateTimecycle(newName);
						if (created != nullptr)
						{
							CurrentTimecycle.SetModifier(created);
							memset(g_editorDetailNameBuffer, 0, kMaxBufferSize);
						}
					}
				}

				auto modifier = CurrentTimecycle.GetModifier();

				if (modifier != nullptr)
				{
					ImGui::SameLine();

					if (ImGui::Button("Rename"))
					{
						auto nameLength = strlen(g_editorDetailNameBuffer);

						if (nameLength >= 2 && nameLength <= 128)
						{
							auto newName = std::string(g_editorDetailNameBuffer);
							auto newHash = HashString(newName);

							TheTimecycleManager->RenameTimecycle(*modifier, newName);
						}
					}

					ImGui::SameLine();

					if (ImGui::Button("Clone"))
					{
						auto nameLength = strlen(g_editorDetailNameBuffer);

						if (nameLength >= 2 && nameLength <= 128)
						{
							auto cloned = TheTimecycleManager->CloneTimecycle(*modifier, std::string(g_editorDetailNameBuffer));
							if (cloned != nullptr)
							{
								CurrentTimecycle.SetModifier(cloned);
								memset(g_editorDetailNameBuffer, 0, kMaxBufferSize);
							}
						}
					}

					ImGui::SameLine();

					if (ImGui::Button("Delete"))
					{
						TheTimecycleManager->RemoveTimecycle(*modifier);
						CurrentTimecycle.ResetModifier();
					}

					ImGui::SameLine();

					if (ImGui::Button("Generate XML"))
					{
						auto timecycle = CurrentTimecycle.GetModifier();
						auto tcName = TheTimecycleManager->GetTimecycleName(*timecycle).c_str();

						tinyxml2::XMLDocument document;

						auto rootElement = document.NewElement("timecycle_modifier_data");
						rootElement->SetAttribute("version", "1.000000");

						auto modElement = document.NewElement("modifier");
						modElement->SetAttribute("name", tcName);
						modElement->SetAttribute("numMods", timecycle->m_modData.GetCount());
						modElement->SetAttribute("userFlags", 0);

						for (auto& modData : timecycle->m_modData)
						{
							if (auto varInfo = TimecycleManager::GetConfigVarInfo(modData.m_index))
							{
								auto varElement = document.NewElement(varInfo->m_name);
								varElement->SetText(va("%.3f %.3f", modData.m_value1, modData.m_value2));
								modElement->InsertEndChild(varElement);
							}
						}

						rootElement->InsertEndChild(modElement);
						document.InsertEndChild(rootElement);

						g_editorTimecycleOutput.ClearBuffer();
						g_editorTimecycleOutput.PushHeader(false, true);
						document.Print(&g_editorTimecycleOutput);
					}

					if (CurrentTimecycle.HasModifier()) // as it may be deleted at this point
					{
						CurrentTimecycle.UpdateVars();

						ImGui::NextColumn();
						ImGui::Text("Filter");
						ImGui::SameLine();

						ImGui::NextColumn();
						ImGui::InputText("##detail_filter_param", g_editorDetailFilterBuffer, kMaxBufferSize);
						ImGui::SameLine();

						if (ImGui::Button("Clear"))
						{
							memset(g_editorDetailFilterBuffer, 0, kMaxBufferSize);
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

							ImGui::SameLine();

							if (ImGui::Button("As extra"))
							{
								scriptData->m_extraModifierIndex = TheTimecycleManager->GetTimecycleIndex(*modifier);
							}
						}

						ImGui::NextColumn();
						ImGui::Text("Variable");
						ImGui::SameLine();

						ImGui::NextColumn();
						ImGui::InputText("##input", g_editorDetailDropDownBuffer, kMaxBufferSize);
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
									if (istrstr(tcVarInfos[i].m_name, g_editorDetailDropDownBuffer) == nullptr)
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

									if (ImGui::Selectable(tcVarInfos[i].m_name) || (ImGui::IsItemFocused() && ImGui::IsKeyPressedMap(ImGuiKey_Enter)))
									{
										strcpy(g_editorDetailDropDownBuffer, tcVarInfos[i].m_name);
										isOpen = false;
									}
								}
							}

							ImGui::End();
							isOpen &= isFocused;
						}

						if (ImGui::Button("Add"))
						{
							if (TheTimecycleManager->AddTimecycleModData(*modifier, std::string(g_editorDetailDropDownBuffer)))
							{
								memset(g_editorDetailDropDownBuffer, 0, kMaxBufferSize);
							}
						}

						ImGui::SameLine();

						if (ImGui::Button("Clear"))
						{
							memset(g_editorDetailDropDownBuffer, 0, kMaxBufferSize);
						}

						ImGui::Separator();

						ImGui::Columns(6);
						ImGui::Text("Var");
						ImGui::NextColumn();
						ImGui::Text("Name");
						ImGui::NextColumn();
						ImGui::Text("Default");
						ImGui::NextColumn();
						ImGui::Text("Value 1");
						ImGui::NextColumn();
						ImGui::Text("Value 2");
						ImGui::NextColumn();
						ImGui::Text("");
						ImGui::NextColumn();

#define COLUMN_CLAMP_WIDTH(index, min, max)               \
	if (min > 0.0f && ImGui::GetColumnWidth(index) < min) \
		ImGui::SetColumnWidth(index, min);                \
	if (max > 0.0f && ImGui::GetColumnWidth(index) > max) \
		ImGui::SetColumnWidth(index, max);

						COLUMN_CLAMP_WIDTH(0, 50.0f, 80.0f);
						COLUMN_CLAMP_WIDTH(1, 300.0f, -1.0f);
						COLUMN_CLAMP_WIDTH(2, 100.0f, -1.0f);
						COLUMN_CLAMP_WIDTH(3, 125.0f, -1.0f);
						COLUMN_CLAMP_WIDTH(4, 125.0f, -1.0f);
						COLUMN_CLAMP_WIDTH(5, 100.0f, 120.0f);

#undef COLUMN_CLAMP_WIDTH

						bool drawnAny = false;

						// delayed mod data removing queue to not mutate map when drawing stuff
						std::set<int> removeQueue;

						for (auto& [index, modData] : CurrentTimecycle.GetVars())
						{
							if (modData.m_index == -1 || modData.m_index >= TimecycleManager::GetConfigVarInfoCount())
							{
								continue; // invalid?
							}

							auto& varInfo = tcVarInfos[index];

							if (strlen(g_editorDetailFilterBuffer) > 0 && istrstr(varInfo.m_name, g_editorDetailFilterBuffer) == nullptr)
							{
								continue;
							}

							bool isSearched = (strlen(g_editorSearchParamBuffer) > 0 && istrstr(varInfo.m_name, g_editorSearchParamBuffer) != nullptr);

							ImGui::Text("%d", varInfo.m_index);
							ImGui::NextColumn();

							if (isSearched)
							{
								ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(81, 179, 236, 255));
							}

							ImGui::Text("%s", varInfo.m_name);

							if (isSearched)
							{
								ImGui::PopStyleColor();
							}

							ImGui::NextColumn();
							ImGui::Text("%f", varInfo.m_value);
							ImGui::NextColumn();

							bool nulled = (varInfo.m_value == 0.0f);
							float min = nulled ? -1.0f : ((varInfo.m_value > 0.0f) ? -varInfo.m_value : varInfo.m_value) * 10.0f;
							float max = nulled ? 1.0f : ((varInfo.m_value > 0.0f) ? varInfo.m_value : -varInfo.m_value) * 10.0f;
							float step = nulled ? 0.02f : (varInfo.m_value / 20.0f);

							bool isVarEnabled = !CurrentTimecycle.IsVarDisabled(index);
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

							ImGui::NextColumn();

							if (ImGui::DragFloat(va("##%x-%d-2", modifier->m_nameHash, index), &modData.m_value2, step, min, max, "%.3f", 1.0f))
							{
								if (dataSource == actualData)
								{
									actualData->m_value2 = modData.m_value2;
								}
							}

							ImGui::NextColumn();

							if (!isVarEnabled)
							{
								ImGui::EndDisabled();
							}

							if (ImGui::Checkbox(va("##%x-%d-check", modifier->m_nameHash, index), &isVarEnabled))
							{
								if (isVarEnabled)
								{
									if (TheTimecycleManager->AddTimecycleModData(*modifier, std::string(varInfo.m_name)))
									{
										CurrentTimecycle.EnableVar(index);

										auto addedData = TheTimecycleManager->GetTimecycleModData(*modifier, index);
										addedData->m_value1 = modData.m_value1;
										addedData->m_value2 = modData.m_value2;
									}
								}
								else
								{
									if (TheTimecycleManager->RemoveTimecycleModData(*modifier, modData))
									{
										CurrentTimecycle.DisableVar(index);
									}
								}
							}

							ImGui::SameLine();

							if (ImGui::Button(va("Delete##%x-%d-del", modifier->m_nameHash, index)))
							{
								if (TheTimecycleManager->RemoveTimecycleModData(*modifier, std::string(varInfo.m_name)))
								{
									removeQueue.insert(varInfo.m_index);
								}
							}

							ImGui::NextColumn();

							drawnAny = true;
						}

						ImGui::Columns();

						if (!drawnAny && strlen(g_editorDetailFilterBuffer) > 0)
						{
							ImGui::Text("No parameters found");

							if (ImGui::Button("Clear filter query"))
							{
								memset(g_editorDetailFilterBuffer, 0, kMaxBufferSize);
							}
						}

						for (auto index : removeQueue)
						{
							CurrentTimecycle.RemoveVar(index);
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
	});

	OnKillNetworkDone.Connect([=]()
	{
		CurrentTimecycle.ResetModifier();

		memset(g_editorSearchNameBuffer, 0, kMaxBufferSize);
		memset(g_editorSearchParamBuffer, 0, kMaxBufferSize);
		memset(g_editorDetailNameBuffer, 0, kMaxBufferSize);
		memset(g_editorDetailFilterBuffer, 0, kMaxBufferSize);
		memset(g_editorDetailDropDownBuffer, 0, kMaxBufferSize);

		g_editorApplySelectedTimecycle = false;
		g_editorTimecycleOutput.ClearBuffer();
	});
});
