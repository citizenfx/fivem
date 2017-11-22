/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

class CitizenGame
{
private:
	static void InvokeEntryPoint(void(*)());

public:
	static void Launch(const std::wstring& gamePath, bool isMainGame = false);

    static void SetCoreMapping();
};
