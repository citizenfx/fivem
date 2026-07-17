/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

class UserLibrary
{
private:
	std::vector<uint8_t> m_libraryBuffer;

public:
	UserLibrary(const wchar_t* fileName);

	uint32_t GetExportCode(const char* name) const;

	const uint8_t* GetOffsetPointer(uint32_t offset) const;
};
