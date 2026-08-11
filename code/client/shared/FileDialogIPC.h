#pragma once

// Request/response block for the out-of-process file dialog helper.
//
// The game process cannot safely show IFileDialog itself: shell extension creation
// deadlocks on the shell's own worker pool inside the hooked/hand-mapped game process
// (see nui-core/src/NUIFileDialog.cpp). The dialog is therefore shown by a clean copy of
// the main launcher exe ("FiveM_FileDialog.exe", spawned via MakeCfxSubProcess), which
// receives this block through an inherited anonymous file mapping and signals completion
// through an inherited anonymous event - both passed as handle integers on the command
// line, following the -dumpserver/-switchcl convention.
//
// Fixed layout on purpose: no allocation, no parsing, no versioned serialization - the
// mapping is created and consumed by binaries built from the same tree.

#include <windows.h>
#include <stdint.h>

namespace fdipc
{
constexpr uint32_t kVersion = 1;

constexpr size_t kMaxStringChars = 1024;
constexpr size_t kMaxFilters = 24;
constexpr size_t kMaxFilterChars = 256;
constexpr size_t kMaxResultChars = 64 * 1024;

// mirrors CefDialogHandler::FileDialogMode - kept as raw integers so the launcher side
// needs no CEF headers
constexpr uint32_t kModeOpen = 0;
constexpr uint32_t kModeOpenMultiple = 1;
constexpr uint32_t kModeOpenFolder = 2;
constexpr uint32_t kModeSave = 3;

struct Request
{
	uint32_t version;
	uint32_t mode;

	wchar_t title[kMaxStringChars];
	wchar_t defaultPath[kMaxStringChars];

	// pre-built COMDLG_FILTERSPEC pairs - the game side owns filter derivation (MIME map
	// etc.), the helper just passes them through
	uint32_t numFilters;
	wchar_t filterNames[kMaxFilters][kMaxFilterChars];
	wchar_t filterSpecs[kMaxFilters][kMaxFilterChars];
};

struct Response
{
	// set by the helper the moment the dialog window exists; the game uses it to tell
	// "user is browsing, wait forever" from "wedged in init, kill the helper"
	volatile LONG windowCreated;

	uint32_t succeeded; // 1 = user picked file(s), 0 = cancelled or failed
	uint32_t numFiles;

	// numFiles null-terminated strings, packed back to back
	uint32_t resultChars;
	wchar_t results[kMaxResultChars];
};

struct SharedBlock
{
	Request request;
	Response response;
};
}
