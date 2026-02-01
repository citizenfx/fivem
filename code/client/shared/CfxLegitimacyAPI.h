#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32)
#if defined(COMPILING_CFX_LEGITIMACY)
#define CFX_API __declspec(dllexport)
#else
#define CFX_API __declspec(dllimport)
#endif
#define CFX_CALL __stdcall
#else
#define CFX_API
#define CFX_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void(CFX_CALL* cfx_auth_callback)(bool success, const char* data, size_t data_length);
typedef void(CFX_CALL* cfx_steam_ticket_callback)(const char* ticket, size_t ticket_len, const char* response, size_t response_len, void* user_data);

CFX_API void CFX_CALL cfx_discord_authenticate(const char* user_id, const char* code, cfx_auth_callback callback);
CFX_API void CFX_CALL cfx_discourse_authenticate(const char* client_id, const char* auth_token, cfx_auth_callback callback);

CFX_API bool CFX_CALL cfx_should_process_headers(const char* hostname);
CFX_API void CFX_CALL cfx_process_headers(char*, char*);

CFX_API void CFX_CALL cfx_steam_init(void);
CFX_API bool CFX_CALL cfx_steam_is_running(void);
CFX_API bool CFX_CALL cfx_steam_is_initialized(void);
CFX_API void CFX_CALL cfx_steam_get_auth_ticket(cfx_steam_ticket_callback callback, bool enforce_auth, void* user_data);
CFX_API uint64_t CFX_CALL cfx_steam_get_id(void);
CFX_API const char* CFX_CALL cfx_steam_get_username(void);
CFX_API void CFX_CALL cfx_steam_set_rich_presence(const char* key, const char* value);
CFX_API void CFX_CALL cfx_steam_reset_rich_presence(void);
CFX_API bool CFX_CALL cfx_steam_set_app_id(int legacy);
CFX_API void CFX_CALL cfx_steam_wait_for_app_switch(void);

#ifdef __cplusplus
}
#endif
