#pragma once

#include <windows.h>
#include <kbd.h>

static const BYTE aVkNumpad[13] = {
	0x67, 0x68, 0x69, 0xFF, 0x64, 0x65, 0x66, 0xFF, 0x61, 0x62, 0x63, 0x60, 0x6E
};
static PKBDTABLES kbdUsTables;

static bool EnsureKbdUsTables();
static HKL GetUSKeyboard();
static UINT MapExtendedVirtualKey(UINT virtualKey, PVSC_VK VscVkTableStart, UINT resultOr);
static UINT MapKeyUsingExtendedTables(UINT input, bool isEx, PKBDTABLES kbdTables);
static UINT MapKeyToScanCode(UINT input, bool isEx, PKBDTABLES kbdTables);
static UINT MapExtendedScanCode(BYTE scanCode, PVSC_VK VscVkTableStart);
static UINT MapScanCodeToKey(UINT input, bool isEx, PKBDTABLES kbdTables);
static UINT MapKeyToWchar(UINT input, PKBDTABLES kbdTables);
static UINT InternalMapVirtualKeyEx(UINT input, UINT type, PKBDTABLES kbdTables);
UINT MapVirtualKeyInternal(UINT input, UINT type);
uint16_t MapGameKey(const uint16_t virtualKey, const uint16_t scanCode, bool isE0);
