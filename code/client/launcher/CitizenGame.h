/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

class CitizenGame
{
private:
	static void InvokeEntryPoint(void(*)());

public:
	static void Launch(const std::wstring& gamePath);

    static void SetCoreMapping();
};