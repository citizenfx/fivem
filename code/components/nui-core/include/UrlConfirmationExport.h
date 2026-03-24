#pragma once

#include <mutex>

#ifdef COMPILING_NUI_CORE
#define NUI_EXPORT DLL_EXPORT
#else
#define NUI_EXPORT DLL_IMPORT
#endif

namespace nui
{
// State for the URL confirmation modal
extern NUI_EXPORT std::atomic<bool> g_showUrlConfirmModal;
extern NUI_EXPORT std::string g_pendingUrl;
extern NUI_EXPORT std::mutex g_urlModalMutex;
}
