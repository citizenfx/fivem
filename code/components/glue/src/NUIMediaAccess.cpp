#include <StdInc.h>
#include <CefOverlay.h>
#include <json.hpp>
#include <NetLibrary.h>

#include <ConsoleHost.h>

#include <shlobj.h>
#include "KnownFolders.h"
#include <fstream>

static bool displayingPermissionRequest;
static std::string requestedPermissionResource;
static std::string requestedPermissionUrl;
static std::string requestedPermissionOrigin;
static int requestedPermissionMask;
static std::function<void(bool, int)> requestedPermissionCallback;
static std::map<std::string, int> g_mediaSettings;

extern NetLibrary* netLibrary;

static void ParseMediaSettings(const std::string& jsonStr)
{
	try
	{
		auto j = nlohmann::json::parse(jsonStr);
		
		for (auto& pair : j["origins"])
		{
			g_mediaSettings.emplace(pair[0].get<std::string>(), pair[1].get<int>());
		}
	}
	catch (std::exception& e)
	{

	}
}

void LoadPermissionSettings()
{
	PWSTR appDataPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
	{
		// create the directory if not existent
		std::wstring cfxPath = std::wstring(appDataPath) + L"\\VMP";
		CreateDirectory(cfxPath.c_str(), nullptr);

		// open and read the profile file
		std::wstring settingsPath = cfxPath + L"\\media_access.json";
		if (FILE* profileFile = _wfopen(settingsPath.c_str(), L"rb"))
		{
			std::ifstream settingsFile(settingsPath);

			std::stringstream settingsStream;
			settingsStream << settingsFile.rdbuf();
			settingsFile.close();

			ParseMediaSettings(settingsStream.str());
		}

		CoTaskMemFree(appDataPath);
	}
}

void SavePermissionSettings(const std::string& json)
{
	PWSTR appDataPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
	{
		// create the directory if not existent
		std::wstring cfxPath = std::wstring(appDataPath) + L"\\VMP";
		CreateDirectory(cfxPath.c_str(), nullptr);
		// open and read the profile file
		std::wstring settingsPath = cfxPath + L"\\media_access.json";
		std::ofstream settingsFile(settingsPath);
		settingsFile << json;
		settingsFile.close();
		CoTaskMemFree(appDataPath);
	}
}

void SavePermissionSettings()
{
	auto a = nlohmann::json::array();

	for (auto& pair : g_mediaSettings)
	{
		a.push_back(nlohmann::json::array({ pair.first, pair.second }));
	}

	auto j = nlohmann::json::object();
	j["origins"] = a;

	try
	{
		SavePermissionSettings(j.dump());
	}
	catch (std::exception& e)
	{
	
	}
}

static bool permGrants[32];

bool HandleMediaRequest(const std::string& frameOrigin, const std::string& url, int permissions, const std::function<void(bool, int)>& onComplete)
{
	// if not connected to a server, we will allow media
	if (netLibrary->GetConnectionState() == NetLibrary::CS_IDLE)
	{
		onComplete(true, permissions);
		return true;
	}

	// do not allow desktop video capture (this has been shown to be abused and users do not understand security implications)
	permissions &= ~nui::NUI_MEDIA_PERMISSION_DESKTOP_VIDEO_CAPTURE;

	// if this server+resource already has permission, keep it
	auto origin = netLibrary->GetCurrentPeer().ToString() + ":" + frameOrigin;
	int hadPermissions = 0;

	if (auto it = g_mediaSettings.find(origin); it != g_mediaSettings.end())
	{
		hadPermissions = it->second;

		if ((it->second & permissions) == permissions)
		{
			onComplete(true, it->second & permissions);
			return true;
		}
	}

	// if we're already in a request, deny it
	if (displayingPermissionRequest)
	{
		return false;
	}

	// start a request cycle
	requestedPermissionResource = frameOrigin;
	requestedPermissionUrl = url;
	requestedPermissionCallback = [onComplete, hadPermissions, permissions](bool success, int mask)
	{
		displayingPermissionRequest = false;

		if (onComplete)
		{
			onComplete(success, (mask | hadPermissions) & permissions);

			if (success)
			{
				g_mediaSettings[requestedPermissionOrigin] |= mask;
				SavePermissionSettings();
			}
		}
	};
	requestedPermissionMask = permissions & ~hadPermissions;
	requestedPermissionOrigin = origin;
	
	permGrants[0] = true;
	permGrants[1] = false;
	permGrants[2] = false;
	permGrants[3] = false;

	displayingPermissionRequest = true;

	return true;
}

#include <imgui.h>

static InitFunction mediaRequestInit([]()
{
	LoadPermissionSettings();

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || displayingPermissionRequest;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (displayingPermissionRequest)
		{
			const float DISTANCE = 10.0f;
			ImVec2 window_pos = ImVec2(ImGui::GetMainViewport()->Pos.x + ImGui::GetIO().DisplaySize.x - DISTANCE, ImGui::GetMainViewport()->Pos.y + DISTANCE);
			ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f);

			if (ConHost::IsConsoleOpen())
			{
				window_pos = ImVec2(ImGui::GetMainViewport()->Pos.x + ImGui::GetIO().DisplaySize.x - DISTANCE, ImGui::GetMainViewport()->Pos.y + ImGui::GetIO().DisplaySize.y - DISTANCE);
				window_pos_pivot = ImVec2(1.0f, 1.0f);
			}

			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
			ImGui::SetNextWindowFocus();
			ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
			if (ImGui::Begin("Permission Request", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
			{
				ImGui::Text("Permission request: %s", requestedPermissionResource.c_str());
				ImGui::Separator();
				
				if (ConHost::IsConsoleOpen())
				{
					ImGui::Text("The resource %s at %s wants access to the following:", requestedPermissionResource.c_str(), requestedPermissionUrl.c_str());

					static std::map<int, std::string> permTypes{
						{ nui::NUI_MEDIA_PERMISSION_DESKTOP_AUDIO_CAPTURE, "Capture your desktop sound" },
						{ nui::NUI_MEDIA_PERMISSION_DESKTOP_VIDEO_CAPTURE, "Capture your full screen ^1/!\\ ALERT! This may expose personal information!" },
						{ nui::NUI_MEDIA_PERMISSION_DEVICE_AUDIO_CAPTURE, "Capture your microphone" },
						{ nui::NUI_MEDIA_PERMISSION_DEVICE_VIDEO_CAPTURE, "Capture your webcam" },
					};

					int i = 0;

					for (auto& type : permTypes)
					{
						if (requestedPermissionMask & type.first)
						{
							ImGui::Checkbox(type.second.c_str(), &permGrants[i]);
						}

						i++;
					}

					if (requestedPermissionMask & nui::NUI_MEDIA_PERMISSION_DESKTOP_VIDEO_CAPTURE)
					{
						if (permGrants[2] && !permGrants[3])
						{
							permGrants[3] = permGrants[2];
						}
					}

					// "accepted must match requested" dumb stuff
					if (!(requestedPermissionMask & nui::NUI_MEDIA_PERMISSION_DEVICE_VIDEO_CAPTURE))
					{
						bool allowedAny = permGrants[0] || permGrants[1];
						permGrants[0] = allowedAny;
						permGrants[1] = allowedAny;
					}

					ImGui::Separator();
					if (ImGui::Button("Allow"))
					{
						int outcomeMask = 0;
						int i = 0;

						for (auto& type : permTypes)
						{
							if (permGrants[i] && (requestedPermissionMask & type.first))
							{
								outcomeMask |= type.first;
							}

							i++;
						}

						requestedPermissionCallback(true, outcomeMask);
						requestedPermissionCallback = {};
					}
					
					ImGui::SameLine();

					if (ImGui::Button("Deny"))
					{
						requestedPermissionCallback(false, 0);
						requestedPermissionCallback = {};
					}
				}
				else
				{
					ImGui::Text("The resource at %s is requesting your permission to access media devices.", requestedPermissionUrl.c_str());
					ImGui::Separator();
					ImGui::Text("Press ^2F8^7 to accept/deny.");
				}
			}
			ImGui::End();
		}
	});
});
