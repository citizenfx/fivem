# This document describes how different components of fxdk communicate with each other

These are different isolated components that require that or another IPC comms:
1. SDK_HOST - FxDK cpp core
2. SDK_BACKEND - FxDK ts backend (resources/sdk-root/src/backend)
3. SDK_FRONTEND - FxDK ts react frontend (resources/sdk-root/src)
4. SDK_FRONTEND_THEIA - Theia fronend
5. GAME_CLIENT - Game client
6. GAME_SERVER - Game server

Ways they communicate:
 - SDK_HOST     <=> SDK_BACKEND        - Events
 - SDK_HOST     <=> GAME_CLIENT        - Custom IPC labelled `launcherTalk`
 - SDK_BACKEND  <=> SDK_FRONTEND       - WebSockets
 - SDK_BACKEND  <=> GAME_SERVER        - Custom IPC channel
 - GAME_SERVER  <=> GAME_CLIENT        - Events
 - SDK_FRONTEND <=> SDK_FRONTEND_THEIA - Window messages
 - SDK_FRONTEND* => SDK_HOST           - Custom V8 handlers of functions that send process messages to SDK_HOST
 - SDK_FRONTEND* => GAME_CLIENT        - Limited communication via SharedHostData
 - SDK_HOST      => SDK_FRONTEND       - Window messages
