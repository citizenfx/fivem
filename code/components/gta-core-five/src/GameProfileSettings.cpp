#include <StdInc.h>
#include <Hooking.h>

#include <CoreConsole.h>
#include <RageParser.h>

#include <nutsnbolts.h>

#include <CrossBuildRuntime.h>

#include <boost/algorithm/string.hpp>

#include <atArray.h>

static void** g_profileSettings;

#define PS_NAME(s) { HashRageString(s), s }

static std::map<uint32_t, std::string> _profileSettingNames = {
	PS_NAME("PREF_SFX_VOLUME"),
	PS_NAME("PREF_MUSIC_VOLUME_IN_MP"),
	PS_NAME("PREF_MUSIC_VOLUME"),
	PS_NAME("PREF_DIAG_BOOST"),
	PS_NAME("PREF_RADIO_STATION"),
	PS_NAME("PREF_UR_PLAYMODE"),
	PS_NAME("PREF_UR_AUTOSCAN"),
	PS_NAME("PREF_VOICE_OUTPUT"),
	PS_NAME("PREF_VOICE_SPEAKERS"),
	PS_NAME("PREF_SPEAKER_OUTPUT"),
	PS_NAME("PREF_CTRL_SPEAKER"),
	PS_NAME("PREF_CTRL_SPEAKER_HEADPHONE"),
	PS_NAME("PREF_CTRL_SPEAKER_VOL"),
	PS_NAME("PREF_PULSE_HEADSET"),
	PS_NAME("PREF_SS_FRONT"),
	PS_NAME("PREF_SS_REAR"),
	PS_NAME("PREF_AUDIO_MUTE_ON_FOCUS_LOSS"),
	PS_NAME("PREF_RADAR_MODE"),
	PS_NAME("PREF_DISPLAY_HUD"),
	PS_NAME("PREF_RETICULE"),
	PS_NAME("PREF_RETICULE_SIZE"),
	PS_NAME("PREF_DISPLAY_GPS"),
	PS_NAME("PREF_BIG_RADAR"),
	PS_NAME("PREF_BIG_RADAR_NAMES"),
	PS_NAME("PREF_GAMMA"),
	PS_NAME("PREF_SAFEZONE_SIZE"),
	PS_NAME("PREF_DOF"),
	PS_NAME("PREF_SKFX"),
	PS_NAME("PREF_SUBTITLES"),
	PS_NAME("PREF_MEASUREMENT_SYSTEM"),
	PS_NAME("PREF_CURRENT_LANGUAGE"),
	PS_NAME("PREF_GFX_VID_OVERRIDE"),
	PS_NAME("PREF_GFX_DXVERSION"),
	PS_NAME("PREF_VID_SCREEN_TYPE"),
	PS_NAME("PREF_VID_RESOLUTION"),
	PS_NAME("PREF_VID_ASPECT"),
	PS_NAME("PREF_VID_REFRESH"),
	PS_NAME("PREF_VID_ADAPTER"),
	PS_NAME("PREF_VID_MONITOR"),
	PS_NAME("PREF_GFX_FXAA"),
	PS_NAME("PREF_GFX_MSAA"),
	PS_NAME("PREF_GFX_TXAA"),
	PS_NAME("PREF_VID_VSYNC"),
	PS_NAME("PREF_VID_STEREO"),
	PS_NAME("PREF_VID_STEREO_CONVERGENCE"),
	PS_NAME("PREF_VID_STEREO_SEPARATION"),
	PS_NAME("PREF_VID_PAUSE_ON_FOCUS_LOSS"),
	PS_NAME("PREF_GFX_CITY_DENSITY"),
	PS_NAME("PREF_GFX_POP_VARIETY"),
	PS_NAME("PREF_GFX_DIST_SCALE"),
	PS_NAME("PREF_GFX_TEXTURE_QUALITY"),
	PS_NAME("PREF_GFX_SHADER_QUALITY"),
	PS_NAME("PREF_GFX_SHADOW_QUALITY"),
	PS_NAME("PREF_GFX_REFLECTION_QUALITY"),
	PS_NAME("PREF_GFX_REFLECTION_MSAA"),
	PS_NAME("PREF_GFX_WATER_QUALITY"),
	PS_NAME("PREF_GFX_PARTICLES_QUALITY"),
	PS_NAME("PREF_GFX_GRASS_QUALITY"),
	PS_NAME("PREF_GFX_NIGHT_LIGHTS"),
	PS_NAME("PREF_GFX_SHADOW_SOFTNESS"),
	PS_NAME("PREF_GFX_POST_FX"),
	PS_NAME("PREF_GFX_MB_STRENGTH"),
	PS_NAME("PREF_GFX_DOF"),
	PS_NAME("PREF_GFX_ANISOTROPIC_FILTERING"),
	PS_NAME("PREF_GFX_AMBIENT_OCCLUSION"),
	PS_NAME("PREF_GFX_TESSELLATION"),
	PS_NAME("PREF_ADV_GFX_LONG_SHADOWS"),
	PS_NAME("PREF_ADV_GFX_ULTRA_SHADOWS"),
	PS_NAME("PREF_ADV_GFX_HD_FLIGHT"),
	PS_NAME("PREF_ADV_GFX_MAX_LOD"),
	PS_NAME("PREF_ADV_GFX_SHADOWS_DIST_MULT"),
	PS_NAME("PREF_GFX_SCALING"),
	PS_NAME("PREF_REPLAY_MODE"),
	PS_NAME("PREF_REPLAY_MEM_LIMIT"),
	PS_NAME("PREF_REPLAY_AUTO_RESUME_RECORDING"),
	PS_NAME("PREF_REPLAY_AUTO_SAVE_RECORDING"),
	PS_NAME("PREF_VIDEO_UPLOAD_PRIVACY"),
	PS_NAME("PREF_ROCKSTAR_EDITOR_TOOLTIP"),
	PS_NAME("PREF_ROCKSTAR_EDITOR_EXPORT_GRAPHICS_UPGRADE"),
	PS_NAME("PREF_VOICE_ENABLE"),
	PS_NAME("PREF_VOICE_OUTPUT_DEVICE"),
	PS_NAME("PREF_VOICE_OUTPUT_VOLUME"),
	PS_NAME("PREF_VOICE_SOUND_VOLUME"),
	PS_NAME("PREF_VOICE_MUSIC_VOLUME"),
	PS_NAME("PREF_VOICE_TALK_ENABLED"),
	PS_NAME("PREF_VOICE_FEEDBACK"),
	PS_NAME("PREF_VOICE_INPUT_DEVICE"),
	PS_NAME("PREF_VOICE_CHAT_MODE"),
	PS_NAME("PREF_VOICE_MIC_VOLUME"),
	PS_NAME("PREF_VOICE_MIC_SENSITIVITY"),
	PS_NAME("PREF_CONTROLS_CONTEXT"),
	PS_NAME("PREF_TARGET_CONFIG"),
	PS_NAME("PREF_TARGET_CONFIG"),
	PS_NAME("PREF_VIBRATION"),
	PS_NAME("PREF_INVERT_LOOK"),
	PS_NAME("PREF_CONTROL_CONFIG"),
	PS_NAME("PREF_CONTROL_CONFIG_FPS"),
	PS_NAME("PREF_CONTROLLER_SENSITIVITY"),
	PS_NAME("PREF_FPS_LOOK_SENSITIVITY"),
	PS_NAME("PREF_FPS_AIM_SENSITIVITY"),
	PS_NAME("PREF_LOOK_AROUND_SENSITIVITY"),
	PS_NAME("PREF_AIM_DEADZONE"),
	PS_NAME("PREF_FPS_AIM_DEADZONE"),
	PS_NAME("PREF_AIM_ACCELERATION"),
	PS_NAME("PREF_FPS_AIM_ACCELERATION"),
	PS_NAME("PREF_SNIPER_ZOOM"),
	PS_NAME("PREF_CINEMATIC_SHOOTING"),
	PS_NAME("PREF_ALTERNATE_HANDBRAKE"),
	PS_NAME("PREF_ALTERNATE_DRIVEBY"),
	PS_NAME("PREF_CONTROLLER_LIGHT_EFFECT"),
	PS_NAME("PREF_MOUSE_TYPE"),
	PS_NAME("PREF_MOUSE_ON_FOOT_SCALE"),
	PS_NAME("PREF_MOUSE_WEIGHT_SCALE"),
	PS_NAME("PREF_MOUSE_ACCELERATION"),
	PS_NAME("PREF_INVERT_MOUSE"),
	PS_NAME("PREF_KBM_TOGGLE_AIM"),
	PS_NAME("PREF_FPS_DEFAULT_AIM_TYPE"),
	PS_NAME("PREF_MOUSE_DRIVE"),
	PS_NAME("PREF_MOUSE_FLY"),
	PS_NAME("PREF_MOUSE_SUB"),
	PS_NAME("PREF_MOUSE_DRIVING_SCALE"),
	PS_NAME("PREF_MOUSE_PLANE_SCALE"),
	PS_NAME("PREF_MOUSE_HELI_SCALE"),
	PS_NAME("PREF_MOUSE_SUB_SCALE"),
	PS_NAME("PREF_INVERT_MOUSE_FLYING"),
	PS_NAME("PREF_INVERT_MOUSE_SUB"),
	PS_NAME("PREF_SWAP_ROLL_YAW_MOUSE_FLYING"),
	PS_NAME("PREF_MOUSE_AUTOCENTER_BIKE"),
	PS_NAME("PREF_MOUSE_AUTOCENTER_CAR"),
	PS_NAME("PREF_MOUSE_AUTOCENTER_PLANE"),
	PS_NAME("PREF_CONTROLLER_LIGHT_EFFECT"),
	PS_NAME("PREF_FPS_PERSISTANT_VIEW"),
	PS_NAME("PREF_CAMERA_HEIGHT"),
	PS_NAME("PREF_FPS_AUTO_LEVEL"),
	PS_NAME("PREF_FPS_FIELD_OF_VIEW"),
	PS_NAME("PREF_FPS_RAGDOLL"),
	PS_NAME("PREF_FPS_COMBATROLL"),
	PS_NAME("PREF_FPS_HEADBOB"),
	PS_NAME("PREF_FPS_THIRD_PERSON_COVER"),
	PS_NAME("PREF_FPS_VEH_AUTO_CENTER"),
	PS_NAME("PREF_HOOD_CAMERA"),
	PS_NAME("PREF_FPS_RELATIVE_VEHICLE_CAMERA_DRIVEBY_AIMING"),
	PS_NAME("PREF_LANDING_PAGE"),
	PS_NAME("PREF_STARTUP_FLOW"),
	PS_NAME("PREF_AUTOSAVE"),
	PS_NAME("PREF_FACEBOOK_UPDATES"),
	PS_NAME("PREF_FEED_PHONE"),
	PS_NAME("PREF_FEED_STATS"),
	PS_NAME("PREF_FEED_CREW"),
	PS_NAME("PREF_FEED_FRIENDS"),
	PS_NAME("PREF_FEED_SOCIAL"),
	PS_NAME("PREF_FEED_STORE"),
	PS_NAME("PREF_FEED_TOOLTIP"),
	PS_NAME("PREF_FEED_DELAY"),
	PS_NAME("PREF_SIXAXIS_HELI"),
	PS_NAME("PREF_SIXAXIS_BIKE"),
	PS_NAME("PREF_SIXAXIS_BOAT"),
	PS_NAME("PREF_SIXAXIS_RELOAD"),
};

static int* g_prefs;

struct ProfileConVar
{
	ConVar<int> value;
	int lastValue;

	ProfileConVar(const std::string& name, int defaultValue)
		: value(name, ConVar_Archive, defaultValue)
	{
		lastValue = defaultValue;
	}
};

static std::map<int, std::shared_ptr<ProfileConVar>> _profileConVars;

static hook::cdecl_stub<void(int idx, int, int)> _updatePref([]()
{
	if (Is372())
	{
		return (void*)nullptr;
	}

	return hook::get_pattern("83 F9 62 0F 8F ? ? 00 00 83 F9 61 0F", (xbr::IsGameBuildOrGreater<2060>()) ? -0x29 : -0x23);
});

void ProfileSettingsInit()
{
	OnGameFrame.Connect([]()
	{
		if (Is372())
		{
			return;
		}

		if (!*g_profileSettings || !**(void***)g_profileSettings)
		{
			return;
		}

		if (_profileConVars.empty())
		{
			auto structureField = rage::GetStructureDefinition(0xA0063C79);
			auto enumeration = structureField->m_members[2]->m_definition->enumData;

			for (auto field = enumeration->fields; field->index != -1 || field->hash != 0; field++)
			{
				auto name = fmt::sprintf("0x%08x", field->hash);

				if (_profileSettingNames.find(field->hash) != _profileSettingNames.end())
				{
					name = _profileSettingNames[field->hash].substr(5);

					boost::algorithm::to_lower(name);

					while (name.find('_') != std::string::npos)
					{
						auto undPos = name.find('_');
						name = name.substr(0, undPos) + (char)toupper(name[undPos + 1]) + name.substr(undPos + 2);
					}
				}

				name = "profile_" + name;

				// profile_vidMonitor pointing to an invalid index will crash the game when opening settings, and
				// the game doesn't use the profile setting as authoritative anyway
				if (name == "profile_vidMonitor")
				{
					continue;
				}

				// profile_gfx* is also a pretty bad one
				if (name.find("profile_gfx") == 0)
				{
					continue;
				}

				auto setting = g_prefs[field->index];
				_profileConVars[field->index] = std::make_shared<ProfileConVar>(name, setting);
			}
		}

		for (auto& pair : _profileConVars)
		{
			int val = pair.second->value.GetValue();

			// if the pref changed, but the convar didn't, update convar with pref value
			if (g_prefs[pair.first] != val && pair.second->lastValue == val)
			{
				val = g_prefs[pair.first];
				pair.second->value.GetHelper()->SetRawValue(val);
			}

			// if the convar changed, update pref with convar value
			if (pair.second->lastValue != val)
			{
				if (val != -999)
				{
					g_prefs[pair.first] = val;
					_updatePref(pair.first, 2, -1);
				}

				pair.second->lastValue = val;
			}
		}
	});
}

static bool (*g_orig_parSettingsLoad)(void* parManager, const char* filename, const char* ext, void* outStruct, void* handle);

static bool parser_SettingsLoad(void* parManager, const char* filename, const char* ext, char* outStruct, void* handle)
{
	bool rv = g_orig_parSettingsLoad(parManager, filename, ext, outStruct, handle);

	if (rv)
	{
		int& shadowQuality = *(int*)(outStruct + 40);

		if (shadowQuality < 0 || shadowQuality > 3)
		{
			shadowQuality = 1;
		}
	}

	return rv;
}

static HookFunction hookFunction([]()
{
	g_prefs = hook::get_address<int*>(hook::get_pattern("8D 0C 9B 8B 14 AA 8D 3C 4A 83 F8 FF 0F 84", -4));
	g_profileSettings = hook::get_address<void**>(hook::get_pattern("44 38 7B 20 0F 84 ? ? 00 00 41 8D 7F 02", -4));

	// reset shadow quality to 'normal' if it has had -1 saved
	{
		auto location = hook::get_pattern("4C 8B C7 48 89 44 24 20 E8 ? ? ? ? 84 C0 75 14", 8);
		hook::set_call(&g_orig_parSettingsLoad, location);
		hook::call(location, parser_SettingsLoad);
	}
	
	// Patches enabling ShadowQuality=OFF in pausemenu
	if (xbr::IsGameBuildOrGreater<2802>())
	{
		// 44 8B C6      mov     r8d, esi
		// FF CA         dec     edx    <---------------
		hook::nop(hook::get_pattern<unsigned char>("44 8B C6 FF CA", 3), 2);
	}
	else
	{
		// 8D 4B 42      lea     ecx, [rbx+42h]
		// FF CA         dec     edx    <---------------
		hook::nop(hook::get_pattern<unsigned char>("8D 4B 42 FF CA", 3), 2);
	}

	// 8B 45 88      mov     eax, [rbp+0D0h+var_148]
    // FF C8         dec     eax   <--------------
    // 3B C1         cmp     eax, ecx
	hook::nop(hook::get_pattern<unsigned char>("8B 45 ? FF C8 3B C1", 3), 2);

	// 8B 44 24 68   mov     eax, [rsp+300h+var_298]
    // FF C8         dec     eax   <----------------
    // 3B C1         cmp     eax, ecx
	hook::nop(hook::get_pattern<unsigned char>("8B 44 ? ? FF C8 3B C1", 4), 2);

	// *This is where the Game converts the value from profile/preferences in the pausemenu to the CGraphicsSettings in the CSettingsManager*
	// 89 43 5C             mov     [rbx+5Ch], eax
    // 8B 05 04 E3 ED 01    mov     eax, cs:pref_shadowQuality
    // FF C0                inc     eax  <---------------
	hook::nop(hook::get_pattern<unsigned char>("89 43 ? 8B 05 ? ? ? ? FF C0 89", 9), 2);

	// Most of the checks are done via the game's internal settings, but there is one here in the pausemenu for another option(#93)
	// 83 3D 92 A2 EC 01 02   cmp     cs:pref_shadowQuality, 2  <--------------- (this should be 3 now)
	// 41 0F 9D C4            setnl   r12b
	unsigned char *cmp2 = hook::get_pattern<unsigned char>("83 3D ? ? ? ? 02 41 0F 9D C4", 6);
	hook::put<uint8_t>(cmp2, 0x03);

	// Crash fix (connecticut-texas-carbon) for `_updatePref()` (settings->display->safezone-size)
	// The game doesn't check for nullptr here, but does in other places...

	// Conveniently, there is a function [1604]sub_14018014C() that checks null and sets=1 for us
	unsigned char* checkAndSetFuncAddr = hook::get_pattern<unsigned char>("48 8B 05 ? ? ? ? 48 85 C0 74 ? C6 80 AC 00 00 00 01 C3");

	if (!xbr::IsGameBuildOrGreater<2372>())
	{
		// 48 8B 05 EB 74 E7 01    mov     rax, cs:qword_14206E448
		// 40 88 B8 AC 00 00 00    mov     [rax+0ACh], dil(value=1)
		unsigned char* crashAddr = hook::get_pattern<unsigned char>("48 8B 05 ? ? ? ? 40 88 B8 AC 00 00 00");
		hook::nop(crashAddr, 14);
		hook::call(crashAddr, checkAndSetFuncAddr);
	}
	else
	{
		// this area was somewhat re-worked in 2372.
		// The pointer is now retrieved via a function ex: auto ptr = GetPointer();
		// And then ptr+172 is set with another Function ex: SetBool172(ptr);
		// however the functions are simple 1-liners, it still doesn't seem to check for null.

		// E8 E2 9A 2A 00                          call    GetPointer
		// 48 8B C8                                mov     rcx, rax
		// E8 C2 0A 2B 00                          call    SetBool172  <-------------------
		// E8 B5 DD 13 00                          call    sub_7FF712DE62B4
		// 48 8B 0D AA 11 E0 01                    mov     rcx, cs:qword_7FF714AA96

		unsigned char* setBool172 = hook::get_pattern<unsigned char>("E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? E8 ? ? ? ? 48 8B 0D", 8);
		hook::call(setBool172, checkAndSetFuncAddr);
	}
});
