#include <StdInc.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <ConsoleHost.h>

#include <CoreConsole.h>

extern console::Context* GetConsoleContext();

struct DevGuiPath
{
	std::vector<std::string> pathEntries;
};

template<>
struct ConsoleArgumentType<DevGuiPath>
{
	static std::string Unparse(const DevGuiPath& input)
	{
		return "Unsupported operation!";
	}

	static bool Parse(const std::string& input, DevGuiPath* out)
	{
		*out = DevGuiPath{};

		size_t curStart = 0;

		for (size_t i = 0; i < input.length(); i++)
		{
			if (input[i] == '/')
			{
				out->pathEntries.push_back(input.substr(curStart, i - curStart));
				curStart = i + 1;
			}
		}

		// add the last entry
		out->pathEntries.push_back(input.substr(curStart, input.length() - curStart));

		if (out->pathEntries.size() >= 2)
		{
			return true;
		}

		return false;
	}
};

template<>
struct ConsoleArgumentName<DevGuiPath>
{
	inline static const char* Get()
	{
		return "DevGuiPath";
	}
};

struct DevGuiNode
{
	enum NodeType
	{
		DevGuiNode_Root,
		DevGuiNode_Menu,
		DevGuiNode_ConVar,
		DevGuiNode_Command,
	} type;

	std::string name;
	std::string commandOrConVar;

	std::list<std::unique_ptr<DevGuiNode>> children;

	inline DevGuiNode* RegisterMenu(const std::string& name)
	{
		for (auto& child : children)
		{
			if (child->name == name)
			{
				return child.get();
			}
		}

		auto child = std::make_unique<DevGuiNode>();
		child->name = name;
		child->type = DevGuiNode_Menu;

		auto childPtr = child.get();
		children.push_back(std::move(child));

		return childPtr;
	}
};

static DevGuiNode devGuiRoot = { DevGuiNode::DevGuiNode_Root, "", "", {} };

static DevGuiNode* DevGui_InstantiatePath(const DevGuiPath& path)
{
	DevGuiNode* node = &devGuiRoot;

	for (int i = 0; i < path.pathEntries.size(); i++)
	{
		node = node->RegisterMenu(path.pathEntries[i]);
	}

	return node;
}

static void DevGui_Draw(const DevGuiNode* node)
{
	if (node->type == DevGuiNode::DevGuiNode_Menu)
	{
		if (ImGui::BeginMenu(node->name.c_str()))
		{
			for (auto& child : node->children)
			{
				DevGui_Draw(child.get());
			}

			ImGui::EndMenu();
		}
	}
	else if (node->type == DevGuiNode::DevGuiNode_Root)
	{
		for (auto& child : node->children)
		{
			DevGui_Draw(child.get());
		}
	}
	else if (node->type == DevGuiNode::DevGuiNode_Command)
	{
		if (ImGui::MenuItem(node->name.c_str()))
		{
			se::ScopedPrincipal scope{
				se::Principal{ "system.console" }
			};
			GetConsoleContext()->ExecuteSingleCommand(node->commandOrConVar);
		}
	}
	else if (node->type == DevGuiNode::DevGuiNode_ConVar)
	{
		auto varMan = GetConsoleContext()->GetVariableManager();
		auto entry = varMan->FindEntryRaw(node->commandOrConVar);

		if (!entry)
		{
			auto varMan = console::GetDefaultContext()->GetVariableManager();
			entry = varMan->FindEntryRaw(node->commandOrConVar);

			if (!entry)
			{
				return;
			}
		}

		try
		{
			auto boolEntry = std::dynamic_pointer_cast<internal::ConsoleVariableEntry<bool>>(entry);

			if (boolEntry)
			{
				auto value = boolEntry->GetRawValue();
				auto initialValue = value;

				ImGui::Checkbox(node->name.c_str(), &value);

				if (initialValue != value)
				{
					boolEntry->SetRawValue(value);
				}

				return;
			}

			auto intEntry = std::dynamic_pointer_cast<internal::ConsoleVariableEntry<int>>(entry);

			if (intEntry)
			{
				auto value = intEntry->GetRawValue();
				auto initialValue = value;

				ImGui::InputScalar(node->name.c_str(), ImGuiDataType_S32, &value);

				if (initialValue != value)
				{
					intEntry->SetRawValue(value);
				}

				return;
			}
		}
		catch (std::bad_cast& e)
		{
			
		}

		auto oldVal = entry->GetValue();
		char textData[8192];
		strcpy_s(textData, oldVal.c_str());
		ImGui::InputText(node->name.c_str(), textData, sizeof(textData));

		if (strcmp(oldVal.c_str(), textData) != 0)
		{
			entry->SetValue(textData);
		}
	}
}

float g_menuHeight;

void DrawDevGui()
{
	if (ImGui::BeginMainMenuBar())
	{
		DevGui_Draw(&devGuiRoot);

		g_menuHeight = ImGui::GetWindowSize().y;

		ImGui::EndMainMenuBar();
	}
}

static InitFunction initFunction([]()
{
	static ConsoleCommand devguiAddCmd("devgui_cmd", [](const DevGuiPath& path, const std::string& commandString)
	{
		auto node = DevGui_InstantiatePath(path);

		node->type = DevGuiNode::DevGuiNode_Command;
		node->commandOrConVar = commandString;
	});

	static ConsoleCommand devguiAddConVar("devgui_convar", [](const DevGuiPath& path, const std::string& convarName)
	{
		auto node = DevGui_InstantiatePath(path);

		node->type = DevGuiNode::DevGuiNode_ConVar;
		node->commandOrConVar = convarName;
	});

#ifndef IS_FXSERVER
	console::GetDefaultContext()->AddToBuffer(R"(
devgui_convar "Tools/Performance/Resource Monitor" resmon
devgui_convar "Tools/Performance/Streaming Memory Viewer" strmem
devgui_convar "Tools/Streaming/Streaming Stats" strdbg
devgui_convar "Tools/Streaming/Streaming List" strlist
devgui_convar "Tools/Network/OneSync/Network Object Viewer" netobjviewer
devgui_convar "Tools/Network/OneSync/Network SyncLog" netobjviewer_syncLog
devgui_convar "Tools/Network/OneSync/Network Time" net_showTime
devgui_convar "Tools/Network/Network Command Log" net_showCommands
devgui_convar "Tools/Network/Network Event Log" netEventLog
devgui_cmd "Tools/NUI/Open DevTools" nui_devTools

devgui_convar "Overlays/Draw FPS" cl_drawFPS
devgui_convar "Overlays/Draw Performance" cl_drawPerf
devgui_convar "Overlays/NetGraph" netgraph

devgui_cmd "Launch/SP/Story Mode" "storymode"
devgui_cmd "Launch/SP/GTA5" "loadlevel gta5"
devgui_cmd "Launch/SP/RDR3" "loadlevel rdr3"
devgui_cmd "Launch/SP/Blank Map" "loadlevel blank"
devgui_cmd "Launch/MP/Localhost" "connect localhost"
devgui_cmd "Launch/MP/Disconnect" "disconnect"
devgui_cmd "Launch/Quit" "quit"

devgui_cmd "Tools/Performance/Profiler/Start Recording - 5 frames" "profiler record 5"
devgui_cmd "Tools/Performance/Profiler/View Last Recording" "profiler view"

set "game_mute" "profile_sfxVolume 0; profile_musicVolumeInMp 0; profile_musicVolume 0"
set "game_unmute" "profile_sfxVolume 25; profile_musicVolumeInMp 10; profile_musicVolume 10"

devgui_convar "Game/Enable Handbrake Camera" cam_enableHandbrakeCamera
devgui_convar "Game/SFX Volume" profile_sfxVolume
devgui_cmd "Game/Mute" "vstr game_mute"
devgui_cmd "Game/Unmute" "vstr game_unmute"

devgui_convar "Overlays/Draw Performance" cl_drawPerf
)");
<<<<<<< HEAD

	if (IsNonProduction())
	{
		console::GetDefaultContext()->AddToBuffer(R"(
devgui_convar "Tools/Performance/Resource Monitor" resmon
devgui_convar "Tools/Performance/Streaming Memory Viewer" strmem
devgui_convar "Tools/Streaming/Streaming Stats" strdbg
devgui_convar "Tools/Streaming/Streaming List" strlist
devgui_convar "Tools/Network/OneSync/Network Object Viewer" netobjviewer
devgui_convar "Tools/Network/OneSync/Network SyncLog" netobjviewer_syncLog
devgui_convar "Tools/Network/OneSync/Network Time" net_showTime
devgui_convar "Tools/Network/Network Command Log" net_showCommands
devgui_convar "Tools/Network/Network Event Log" netEventLog
devgui_cmd "Tools/NUI/Open DevTools" nui_devTools
)");
	}
=======
>>>>>>> parent of ab7ef4208 (tweak(conhost): censor out console and 'non-trivial' tools when running prod)
#endif
});
