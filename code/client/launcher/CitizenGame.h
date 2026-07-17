/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

class CitizenGame
{
private:
	static void InvokeEntryPoint(void(*)());

public:
	static void SetMinModeManifest(const std::string& manifest);

	static void Launch(const std::wstring& gamePath, bool isMainGame = false);

    static void SetCoreMapping();
};
