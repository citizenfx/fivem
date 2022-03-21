#include <StdInc.h>
#include "USKeyboardMapping.h"

static bool EnsureKbdUsTables()
{
	if (!kbdUsTables)
	{
		auto kbdUsLib = LoadLibrary(L"kbdus.dll");
		if (!kbdUsLib)
			return false;
		auto descriptorLoadFunc = GetProcAddress(kbdUsLib, "KbdLayerDescriptor");
		kbdUsTables = (PKBDTABLES)descriptorLoadFunc();
	}
	return true;
}

static HKL GetUSKeyboard()
{
	static HKL usKeyboard = LoadKeyboardLayoutW(L"00000409", 0);
	return usKeyboard;
}

static UINT MapExtendedVirtualKey(UINT virtualKey, PVSC_VK VscVkTableStart, UINT resultOr)
{
	if (VscVkTableStart)
	{
		auto map = VscVkTableStart;
		while (map->Vk)
		{
			if ((map->Vk & 0xFF) == virtualKey)
				return map->Vsc | resultOr;
			map++;
		}
	}
	return 0;
}

static UINT MapKeyUsingExtendedTables(UINT input, bool isEx, PKBDTABLES kbdTables)
{
	auto e0ExtMap = MapExtendedVirtualKey(input, kbdTables->pVSCtoVK_E0, isEx ? 0xE000u : 0);
	if (e0ExtMap != 0)
		return e0ExtMap;
	if (isEx)
	{
		auto e1ExtMap = MapExtendedVirtualKey(input, kbdTables->pVSCtoVK_E1, 0xE100);
		if (e1ExtMap != 0)
			return e1ExtMap;
	}
	for (int i = 0; i < sizeof(aVkNumpad); i++)
	{
		if (aVkNumpad[i] == input)
			return i + 71;
	}
	return 0;
}

static UINT MapKeyToScanCode(UINT input, bool isEx, PKBDTABLES kbdTables)
{
	switch (input)
	{
		case VK_SHIFT:
			input = VK_LSHIFT;
			break;
		case VK_CONTROL:
			input = VK_LCONTROL;
			break;
		case VK_MENU:
			input = VK_LMENU;
			break;
	}
	for (int i = 0; i < kbdTables->bMaxVSCtoVK; i++)
	{
		auto vk = (kbdTables->pusVSCtoVK[i] & 0xFF);
		if (vk == input)
			return i;
	}
	return MapKeyUsingExtendedTables(input, isEx, kbdTables);
}

static UINT MapExtendedScanCode(BYTE scanCode, PVSC_VK VscVkTableStart)
{
	if (VscVkTableStart)
	{
		auto map = VscVkTableStart;
		while (map->Vk)
		{
			if (map->Vsc == scanCode)
				return map->Vk & 0xFF;
			map++;
		}
	}
	return 0;
}

static UINT MapScanCodeToKey(UINT input, bool isEx, PKBDTABLES kbdTables)
{
	auto result = 0u;
	if (input < kbdTables->bMaxVSCtoVK)
	{
		result = kbdTables->pusVSCtoVK[input] & 0xFF;
	}
	else
	{
		if ((input & 0xFFFFFF00) == 0xE000)
		{
			auto extMap = MapExtendedScanCode(input, kbdTables->pVSCtoVK_E0);
			if (extMap != 0)
				result = extMap;
		}
		if ((input & 0xFFFFFF00) == 0xE100)
		{
			auto extMap = MapExtendedScanCode(input, kbdTables->pVSCtoVK_E1);
			if (extMap != 0)
				result = extMap;
		}
	}
	if (!isEx)
	{
		switch (result)
		{
			case VK_LSHIFT:
			case VK_RSHIFT:
				result = VK_SHIFT;
				break;
			case VK_LCONTROL:
			case VK_RCONTROL:
				result = VK_CONTROL;
				break;
			case VK_LMENU:
			case VK_RMENU:
				result = VK_MENU;
				break;
		}
	}
	if (result != 0xFF)
		return result;
	return 0;
}

static UINT MapKeyToWchar(UINT input, PKBDTABLES kbdTables)
{
	if (input >= 0x41 && input <= 0x5A)
		return input;
	if (!kbdTables->pVkToWcharTable)
		return 0;
	auto wcharTableDesc = kbdTables->pVkToWcharTable;
	while (wcharTableDesc->pVkToWchars)
	{
		auto vkwMap = wcharTableDesc->pVkToWchars;
		while (vkwMap->VirtualKey)
		{
			if (vkwMap->VirtualKey == input)
			{
				switch (vkwMap->wch[0])
				{
					case WCH_NONE:
						return 0;
					case WCH_DEAD:
					{
						auto nextWchar = (PVK_TO_WCHARS2)((BYTE*)vkwMap + wcharTableDesc->cbSize);
						return nextWchar->wch[1] | 0x80000000;
					}
					default:
						return vkwMap->wch[0];
				}
			}
			vkwMap = (PVK_TO_WCHARS1)((BYTE*)vkwMap + wcharTableDesc->cbSize);
		}
		wcharTableDesc++;
	}
	return 0;
}

static UINT InternalMapVirtualKeyEx(UINT input, UINT type, PKBDTABLES kbdTables)
{
	switch (type)
	{
		case MAPVK_VK_TO_VSC:
			return MapKeyToScanCode(input, false, kbdTables);
		case MAPVK_VSC_TO_VK:
			return MapScanCodeToKey(input, false, kbdTables);
		case MAPVK_VK_TO_CHAR:
			return MapKeyToWchar(input, kbdTables);
		case MAPVK_VSC_TO_VK_EX:
			return MapScanCodeToKey(input, true, kbdTables);
		case MAPVK_VK_TO_VSC_EX:
			return MapKeyToScanCode(input, true, kbdTables);
		default:
			return 0;
	}
}

UINT MapVirtualKeyInternal(UINT input, UINT type)
{
	UINT newVk;
	if (EnsureKbdUsTables())
		newVk = InternalMapVirtualKeyEx(input, type, kbdUsTables);
	else
		newVk = MapVirtualKeyExW(input, type, GetUSKeyboard());
	return newVk;
}
