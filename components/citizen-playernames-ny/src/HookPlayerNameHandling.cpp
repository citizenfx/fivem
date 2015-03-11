/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <FontRenderer.h>

#include <NetLibrary.h>
#include <ResourceManager.h>

#include <Screen.h>

#include <msgpack.hpp>

#include <DrawCommands.h>

static NetLibrary* g_netLibrary;
static std::unordered_map<int, std::string> g_netIdToNames;

// redefined here as game_ny migration still isn't complete
typedef struct
{
	IN_ADDR ina;
	IN_ADDR inaOnline;
	WORD wPortOnline;
	BYTE abEnet[6];
	BYTE abOnline[20];
} XNADDR;

class CPlayerInfo
{
public:
	XNADDR address; // 36 bytes long

	void* GetPed()
	{
		return *(void**)((char*)this + 1420);
	}

	static CPlayerInfo* GetPlayer(int index);
};

WRAPPER CPlayerInfo* CPlayerInfo::GetPlayer(int index) { EAXJMP(0x817F20); }

static const char* __fastcall CPlayerInfo__GetName(CPlayerInfo* playerInfo)
{
	int netId = playerInfo->address.inaOnline.s_addr;

	return g_netIdToNames[netId].c_str();
}

static void InlineGetName(const char** outNamePtr, char* playerInfoOffset)
{
	CPlayerInfo* playerInfo = reinterpret_cast<CPlayerInfo*>(playerInfoOffset - 0x4C);

	*outNamePtr = CPlayerInfo__GetName(playerInfo);
}

static void __declspec(naked) InlineDrawNameHook()
{
	__asm
	{
		// preserve edx
		push edx

		// pass eax and a ptr to the value of eax on the stack to the subroutine that mingles with the stack
		push eax

		lea eax, [esp + 8]
		push eax

		call InlineGetName

		add esp, 8h

		pop edx

		// push '%s' anyway
		push 0E875ACh

		// and return to sender
		push 04799B4h
		retn
	}
}

static CRGBA g_lastColor;
static void* g_lastPlayer;
static uint32_t g_drawnNameBitfield;

void DrawNetworkNameText(float x, float y, const wchar_t* text, int, int)
{
	// find a player info with this ped
	for (int i = 0; i < 32; i++)
	{
		auto player = CPlayerInfo::GetPlayer(i);

		if (player && player->GetPed() == g_lastPlayer)
		{
			if ((g_drawnNameBitfield & (1 << i)) == 0)
			{
				static float fontSize = (28.0f / 1440.0f) * GetScreenResolutionY();

				g_drawnNameBitfield |= (1 << i);

				wchar_t wideStr[512];
				MultiByteToWideChar(CP_UTF8, 0, CPlayerInfo__GetName(player), -1, wideStr, _countof(wideStr));

				x *= GetScreenResolutionX();
				y *= GetScreenResolutionY();

				CRect rect(x, y, x + 300, y + 40);

				TheFonts->DrawText(wideStr, rect, g_lastColor, fontSize, 1.0f, "Segoe UI");

				return;
			}
		}
	}
}

void __declspec(naked) DrawNetworkNameTextStub()
{
	__asm
	{
		mov g_lastPlayer, edi

		jmp DrawNetworkNameText
	}
}

void DrawNetworkNameSetColor(CRGBA color)
{
	g_lastColor = color;

	// BGR swap the color
	uint8_t tempVar = g_lastColor.red;
	g_lastColor.red = g_lastColor.blue;
	g_lastColor.blue = tempVar;
}

static HookFunction hookFunction([] ()
{
	// CPlayerInfo non-inlined function
	hook::jump(0x75D2E0, CPlayerInfo__GetName);

	// network player names, inline sneaky asm hook
	hook::jump(0x4799AF, InlineDrawNameHook);

	// temp dbg: also show network player name for local player
	//hook::nop(0x479271, 6);

	// network name CFont::SetColour
	hook::call(0x479990, DrawNetworkNameSetColor);

	// network name text draw call
	hook::call(0x479B30, DrawNetworkNameTextStub);

	// don't do player names at the *right* time as we're too lazy to put our calls in the right place
	hook::nop(0x86AFAD, 5);
});

static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* netLibrary)
	{
		g_netLibrary = netLibrary;
	});

	ResourceManager::OnTriggerEvent.Connect([] (const fwString& eventName, const fwString& argsSerialized, int source)
	{
		// if this is the event 'we' handle...
		if (eventName == "onPlayerJoining")
		{
			// deserialize the arguments
			msgpack::unpacked msg;
			msgpack::unpack(msg, argsSerialized.c_str(), argsSerialized.size());

			msgpack::object obj = msg.get();

			// get the netid/name pair

			// convert to an array
			std::vector<msgpack::object> arguments;
			obj.convert(arguments);

			// get the fields from the dictionary, if existent
			if (arguments.size() >= 2)
			{
				// convert to the concrete types
				int netId = arguments[0].as<int>();
				std::string name = arguments[1].as<std::string>();

				// and add to the list
				g_netIdToNames[netId] = name;
			}
		}
	});

	OnPostFrontendRender.Connect([] ()
	{
		g_drawnNameBitfield = 0;

		// draw player names
		((void(*)())0x463310)();
	}, -50);
});