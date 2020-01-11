#include <StdInc.h>
#include <imgui.h>

#include <Resource.h>
#include <ResourceManager.h>

#include <ResourceScriptingComponent.h>
#include <fxScripting.h>

#include <ConsoleHost.h>
#include <nutsnbolts.h>

#include <ICoreGameInit.h>

#include <CoreConsole.h>

std::string OpenFileBrowser(const std::string& extension, const std::string& fileType);

static bool shChecked = false;

static std::string execLuaAction;

static InitFunction initFunction([]()
{
	ConHost::OnDrawGui.Connect([]()
	{
		if (!ConHost::IsConsoleOpen())
		{
			return;
		}

		static bool open = true;

		ImGui::SetNextWindowPos(ImVec2(20.0f, 400.0f), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Tools", &open))
		{
			ImGui::Text("Since cheaters have been going crazy 'selling' their hacks, and every so\noften a new cheat comes along, and we don't have the capacity/manpower to prevent this,\nwe're just going to let ^4everyone^7 do ^4anything^7.");
			ImGui::Text("The last straw was some group of new cheat developers - who definitely,\nunlike the skids before, have the skills to contribute to the core project - starting to use their\nskills to use cheats, and contributing to "
						"the core project didn't even\n*begin* to cross their mind.\n\nTo spite them, we're offering the basic functionality right here, right now.");

			if (ImGui::Button("Execute Lua"))
			{
				std::thread([]()
				{
					execLuaAction = OpenFileBrowser("*.lua", "Lua script");
				}).detach();
			}

			if (ImGui::Button("Dumper"))
			{
				console::Printf("minicon:hax", "dumper coming soon!\n");
			}

			ImGui::Checkbox("Bypass ScriptHook check", &shChecked);
		}

		ImGui::End();
	});

	OnMainGameFrame.Connect([]()
	{
		if (shChecked)
		{
			Instance<ICoreGameInit>::Get()->ShAllowed = true;
		}

		if (!execLuaAction.empty())
		{
			std::string fileToRun;
			std::swap(execLuaAction, fileToRun);

			fx::ResourceManager::GetCurrent()->GetResource("_cfx_internal")->GetComponent<fx::ResourceScriptingComponent>()->ForAllRuntimes([fileToRun](fx::OMPtr<IScriptRuntime> rt)
			{
				fx::OMPtr<IScriptFileHandlingRuntime> fhRt;

				if (FX_SUCCEEDED(rt.As(&fhRt)))
				{
					if (fhRt->HandlesFile(const_cast<char*>(fileToRun.c_str())))
					{
						fhRt->LoadFile(const_cast<char*>(fileToRun.c_str()));
					}
				}
			});
		}
	});
});
