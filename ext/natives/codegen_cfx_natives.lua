native 'GET_GAME_TIMER'
	arguments {}
	apiset 'server'
	returns 'long'
	doc [[
<summary>
Gets the current game timer in milliseconds.
</summary>
<returns>The game time.</returns>
]]

native 'CREATE_RUNTIME_TXD'
	arguments {
		charPtr 'name'
	}
	apiset 'client'
	returns 'long'
	doc [[
<summary>
Creates a runtime texture dictionary with the specified name.

Example:
```lua
local txd = CreateRuntimeTxd('meow')
```
</summary>
<param name="name">The name for the runtime TXD.</param>
<returns>A handle to the runtime TXD.</returns>
]]

native 'CREATE_RUNTIME_TEXTURE'
	arguments {
		long 'txd',
		charPtr 'txn',
		int 'width',
		int 'height'
	}
	apiset 'client'
	returns 'long'
	doc [[
<summary>
Creates a blank runtime texture.
</summary>
<param name="txd">A handle to the runtime TXD to create the runtime texture in.</param>
<param name="txn">The name for the texture in the runtime texture dictionary.</param>
<param name="width">The width of the new texture.</param>
<param name="height">The height of the new texture.</param>
<returns>A runtime texture handle.</returns>
]]

native 'CREATE_RUNTIME_TEXTURE_FROM_DUI_HANDLE'
	arguments {
		long 'txd',
		long 'txn',
		charPtr 'duiHandle'
	}
	apiset 'client'
	returns 'long'
	doc [[
<summary>
Creates a runtime texture from a DUI handle.
</summary>
<param name="txd">A handle to the runtime TXD to create the runtime texture in.</param>
<param name="txn">The name for the texture in the runtime texture dictionary.</param>
<param name="duiHandle">The DUI handle returned from GET\_DUI\_HANDLE.</param>
<returns>The runtime texture handle.</returns>
]]

native 'CREATE_RUNTIME_TEXTURE_FROM_IMAGE'
	arguments {
		long 'txd',
		charPtr 'txn',
		charPtr 'fileName'
	}
	apiset 'client'
	returns 'long'
	doc [[
<summary>
Creates a runtime texture from the specified file in the current resource.
</summary>
<param name="txd">A handle to the runtime TXD to create the runtime texture in.</param>
<param name="txn">The name for the texture in the runtime texture dictionary.</param>
<param name="fileName">The file name of an image to load. This should preferably be a PNG, and has to be specified as a `file` in the resource manifest.</param>
<returns>A runtime texture handle.</returns>
]]

native 'GET_RUNTIME_TEXTURE_WIDTH'
	arguments {
		long 'tex'
	}
	apiset 'client'
	returns 'int'
	doc [[
<summary>
Gets the width of the specified runtime texture.
</summary>
<param name="tex">A handle to the runtime texture.</param>
<returns>The width in pixels.</returns>
]]

native 'GET_RUNTIME_TEXTURE_HEIGHT'
	arguments {
		long 'tex'
	}
	apiset 'client'
	returns 'int'
	doc [[
<summary>
Gets the height of the specified runtime texture.
</summary>
<param name="tex">A handle to the runtime texture.</param>
<returns>The height in pixels.</returns>
]]

native 'GET_RUNTIME_TEXTURE_PITCH'
	arguments {
		long 'tex'
	}
	apiset 'client'
	returns 'int'
	doc [[
<summary>
Gets the row pitch of the specified runtime texture, for use when creating data for `SET_RUNTIME_TEXTURE_ARGB_DATA`.
</summary>
<param name="tex">A handle to the runtime texture.</param>
<returns>The row pitch in bytes.</returns>
]]

native 'SET_RUNTIME_TEXTURE_PIXEL'
	arguments {
		long 'tex',
		int 'x',
		int 'y',
		int 'r',
		int 'g',
		int 'b',
		int 'a'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Sets a pixel in the specified runtime texture. This will have to be committed using `COMMIT_RUNTIME_TEXTURE` to have any effect.
</summary>
<param name="tex">A handle to the runtime texture.</param>
<param name="x">The X position of the pixel to change.</param>
<param name="y">The Y position of the pixel to change.</param>
<param name="r">The new R value (0-255).</param>
<param name="g">The new G value (0-255).</param>
<param name="b">The new B value (0-255).</param>
<param name="a">The new A value (0-255).</param>
]]

native 'SET_RUNTIME_TEXTURE_ARGB_DATA'
	arguments {
		long 'tex',
		charPtr 'buffer',
		int 'length'
	}
	apiset 'client'
	returns 'BOOL'
	
native 'COMMIT_RUNTIME_TEXTURE'
	arguments {
		long 'tex'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Commits the backing pixels to the specified runtime texture.
</summary>
<param name="tex">The runtime texture handle.</param>
]]

native 'CREATE_DUI'
	arguments {
		charPtr 'url',
		int 'width',
		int 'height'
	}
	apiset 'client'
	returns 'long'
	doc [[
<summary>
Creates a DUI browser. This can be used to draw on a runtime texture using CREATE\_RUNTIME\_TEXTURE\_FROM\_DUI\_HANDLE.
</summary>
<param name="url">The initial URL to load in the browser.</param>
<param name="width">The width of the backing surface.</param>
<param name="height">The height of the backing surface.</param>
<returns>A DUI object.</returns>
]]

native 'SET_DUI_URL'
	arguments {
		long 'duiObject',
		charPtr 'url'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Navigates the specified DUI browser to a different URL.
</summary>
<param name="duiObject">The DUI browser handle.</param>
<param name="url">The new URL.</param>
]]

native 'SEND_DUI_MESSAGE'
	arguments {
		long 'duiObject',
		charPtr 'jsonString'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Sends a message to the specific DUI root page. This is similar to SEND\_NUI\_MESSAGE.
</summary>
<param name="duiObject">The DUI browser handle.</param>
<param name="jsonString">The message, encoded as JSON.</param>
]]

native 'GET_DUI_HANDLE'
	arguments {
		long 'duiObject'
	}
	apiset 'client'
	returns 'charPtr'
	doc [[
<summary>
Returns the NUI window handle for a specified DUI browser object.
</summary>
<param name="duiObject">The DUI browser handle.</param>
<returns>The NUI window handle, for use in e.g. CREATE\_RUNTIME\_TEXTURE\_FROM\_DUI\_HANDLE.</returns>
]]

native 'IS_DUI_AVAILABLE'
	arguments {
		long 'duiObject'
	}
	apiset 'client'
	returns 'BOOL'
	doc [[
<summary>
Returns whether or not a browser is created for a specified DUI browser object.
</summary>
<param name="duiObject">The DUI browser handle.</param>
<returns>A boolean indicating TRUE if the browser is created.</returns>
]]

native 'SEND_DUI_MOUSE_MOVE'
	arguments {
		long 'duiObject',
		int 'x',
		int 'y'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Injects a 'mouse move' event for a DUI object. Coordinates are in browser space.
</summary>
<param name="duiObject">The DUI browser handle.</param>
<param name="x">The mouse X position.</param>
<param name="y">The mouse Y position.</param>
]]

native 'SEND_DUI_MOUSE_DOWN'
	arguments {
		long 'duiObject',
		charPtr 'button'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Injects a 'mouse down' event for a DUI object. Coordinates are expected to be set using SEND\_DUI\_MOUSE\_MOVE.
</summary>
<param name="duiObject">The DUI browser handle.</param>
<param name="button">Either `'left'`, `'middle'` or `'right'`.</param>
]]

native 'SEND_DUI_MOUSE_UP'
	arguments {
		long 'duiObject',
		charPtr 'button'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Injects a 'mouse up' event for a DUI object. Coordinates are expected to be set using SEND\_DUI\_MOUSE\_MOVE.
</summary>
<param name="duiObject">The DUI browser handle.</param>
<param name="button">Either `'left'`, `'middle'` or `'right'`.</param>
]]

native 'SEND_DUI_MOUSE_WHEEL'
	arguments {
		long 'duiObject',
		int 'deltaY',
		int 'deltaX'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Injects a 'mouse wheel' event for a DUI object.
</summary>
<param name="duiObject">The DUI browser handle.</param>
<param name="deltaY">The wheel Y delta.</param>
<param name="deltaX">The wheel X delta.</param>
]]

native 'DESTROY_DUI'
	arguments {
		long 'duiObject'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Destroys a DUI browser.
</summary>
<param name="duiObject">The DUI browser handle.</param>
]]

native 'IS_DUPLICITY_VERSION'
	arguments {}
	apiset 'shared'
	returns 'BOOL'
	doc [[
<summary>Gets whether or not this is the CitizenFX server.</summary>
<returns>A boolean value.</returns>
]]

native 'ADD_REPLACE_TEXTURE'
	arguments {
		charPtr 'origTxd',
		charPtr 'origTxn',
		charPtr 'newTxd',
		charPtr 'newTxn',
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Experimental natives, please do not use in a live environment.
</summary>
]]

native 'REMOVE_REPLACE_TEXTURE'
	arguments {
		charPtr 'origTxd',
		charPtr 'origTxn'
	}
	apiset 'client'
	returns 'void'
	doc [[
<summary>
Experimental natives, please do not use in a live environment.
</summary>
]]

native 'TRIGGER_EVENT_INTERNAL'
	arguments {
		charPtr 'eventName',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'shared'
	returns 'void'
	doc [[
	<summary>
	The backing function for TriggerEvent.
	</summary>
]]

native 'TRIGGER_SERVER_EVENT_INTERNAL'
	arguments {
		charPtr 'eventName',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	The backing function for TriggerServerEvent.
	</summary>
]]

native 'TRIGGER_CLIENT_EVENT_INTERNAL'
	arguments {
		charPtr 'eventName',
		charPtr 'eventTarget',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'server'
	returns 'void'
	doc [[
	<summary>
	The backing function for TriggerClientEvent.
	</summary>
]]

native 'CANCEL_EVENT'
	arguments {}
	returns 'void'
	apiset 'shared'
	doc [[
	<summary>
	Cancels the currently executing event. See https://wiki.fivem.net/wiki/CancelEvent
	</summary>
]]

native 'WAS_EVENT_CANCELED'
	arguments {}
	returns 'BOOL'
	apiset 'shared'
	doc [[
	<summary>
	Returns whether or not the currently executing event was canceled. See https://wiki.fivem.net/wiki/WasEventCanceled
	</summary>
	<returns>A boolean.</returns>
]]

native 'INVOKE_FUNCTION_REFERENCE'
	arguments {
		charPtr 'referenceIdentity',
		charPtr 'argsSerialized',
		int 'argsLength',
		intPtr 'retvalLength'
	}
	apiset 'shared'
	returns 'charPtr'

native 'DUPLICATE_FUNCTION_REFERENCE'
	arguments {
		charPtr 'referenceIdentity'
	}
	apiset 'shared'
	returns 'charPtr'

native 'DELETE_FUNCTION_REFERENCE'
	arguments {
		charPtr 'referenceIdentity'
	}
	apiset 'shared'
	returns 'void'

native 'GET_NUM_RESOURCE_METADATA'
	arguments {
		charPtr 'resourceName',
		charPtr 'metadataKey'
	}
	apiset 'shared'
	returns 'int'
	doc [[
	<summary>
	Gets the amount of metadata values with the specified key existing in the specified resource's manifest.
	See also: [Resource manifest](https://wiki.fivem.net/wiki/Resource_manifest)
	</summary>
	<param name="resourceName">The resource name.</param>
	<param name="metadataKey">The key to look up in the resource manifest.</param>
	<returns>The amount of values.</return>
]]

native 'GET_RESOURCE_METADATA'
	arguments {
		charPtr 'resourceName',
		charPtr 'metadataKey',
		int 'index'
	}
	apiset 'shared'
	returns 'charPtr'
	doc [[
	<summary>
	Gets the metadata value at a specified key/index from a resource's manifest.
	See also: [Resource manifest](https://wiki.fivem.net/wiki/Resource_manifest)
	</summary>
	<param name="resourceName">The resource name.</param>
	<param name="metadataKey">The key in the resource manifest.</param>
	<param name="index">The value index, in a range from [0..GET_NUM_RESOURCE_METDATA-1].</param>
	<returns>The metadata value.</return>
]]

native 'LOAD_RESOURCE_FILE'
	arguments {
		charPtr 'resourceName',
		charPtr 'fileName'
	}
	apiset 'shared'
	returns 'charPtr'
	doc [[
	<summary>
	Reads the contents of a text file in a specified resource.
	If executed on the client, this file has to be included in `files` in the resource manifest.

	Example: `local data = LoadResourceFile("devtools", "data.json")`
	</summary>
	<param name="resourceName">The resource name.</param>
	<param name="fileName">The file in the resource.</param>
	<returns>The file contents</returns>
]]

native 'SAVE_RESOURCE_FILE'
	arguments {
		charPtr 'resourceName',
		charPtr 'fileName',
		charPtr 'data',
		int 'dataLength'
	}
	apiset 'server'
	returns 'BOOL'
	doc [[
	<summary>
	Writes the specified data to a file in the specified resource.

	Using a length of `-1` will automatically detect the length assuming the data is a C string.
	</summary>
	<param name="resourceName">The name of the resource.</param>
	<param name="fileName">The name of the file.</param>
	<param name="data">The data to write to the file.</param>
	<param name="dataLength">The length of the written data.</param>
	<returns>A value indicating if the write succeeded.</returns>
]]

native 'GET_RESOURCE_STATE'
	arguments {
		charPtr 'resourceName'
	}
	apiset 'shared'
	returns 'charPtr'
	doc [[
	<summary>
	Returns the current state of the specified resource.
	</summary>
	<param name="resourceName">The name of the resource.</param>
	<returns>The resource state. One of `"missing", "started", "starting", "stopped", "stopping", "uninitialized" or "unknown"`.</returns>
]]

native 'GET_RESOURCE_PATH'
	arguments {
		charPtr 'resourceName'
	}
	apiset 'server'
	returns 'charPtr'
	doc [[
	<summary>
	Returns the physical on-disk path of the specified resource.
	</summary>
	<param name="resourceName">The name of the resource.</param>
	<returns>The resource directory name, possibly without trailing slash.</returns>
]]

native 'GET_CURRENT_RESOURCE_NAME'
	arguments {}
	apiset 'shared'
	returns 'charPtr'
	doc [[
	<summary>
	Returns the name of the currently executing resource.
	</summary>
	<returns>The name of the resource.</returns>
	]]

native 'GET_PLAYER_FROM_SERVER_ID'
	arguments {
		int 'serverId'
	}
	apiset 'client'
	returns 'Player'

native 'GET_PLAYER_SERVER_ID'
	arguments {
		Player 'player'
	}
	apiset 'client'
	returns 'int'

native 'SEND_NUI_MESSAGE'
	arguments {
		charPtr 'jsonString'
	}
	apiset 'client'
	returns 'BOOL'

native 'REGISTER_NUI_CALLBACK_TYPE'
	arguments {
		charPtr 'callbackType'
	}
	apiset 'client'
	returns 'void'

native 'SET_NUI_FOCUS'
	arguments {
		BOOL 'hasFocus',
		BOOL 'hasCursor'
	}
	apiset 'client'
	returns 'void'

native 'GET_NUI_CURSOR_POSITION'
	arguments {
		intPtr 'x',
		intPtr 'y'
	}
	apiset 'client'
	returns 'void'

native 'SET_TEXT_CHAT_ENABLED'
	arguments {
		BOOL 'enabled'
	}
	apiset 'client'
	returns 'BOOL'

native 'ADD_TEXT_ENTRY'
	arguments {
		charPtr 'entryKey',
		charPtr 'entryText'
	}
	apiset 'client'
	returns 'void'

native 'ADD_TEXT_ENTRY_BY_HASH'
	arguments {
		Hash 'entryKey',
		charPtr 'entryText'
	}
	apiset 'client'
	returns 'void'

native 'GET_RESOURCE_KVP_INT'
	arguments {
		charPtr 'key'
	}
	apiset 'client'
	returns 'int'

native 'GET_RESOURCE_KVP_FLOAT'
	arguments {
		charPtr 'key'
	}
	apiset 'client'
	returns 'float'

native 'GET_RESOURCE_KVP_STRING'
	arguments {
		charPtr 'key'
	}
	apiset 'client'
	returns 'charPtr'

native 'SET_RESOURCE_KVP'
	arguments {
		charPtr 'key',
		charPtr 'value'
	}
	apiset 'client'
	returns 'void'

native 'SET_RESOURCE_KVP_INT'
	arguments {
		charPtr 'key',
		int 'value'
	}
	apiset 'client'
	returns 'void'

native 'SET_RESOURCE_KVP_FLOAT'
	arguments {
		charPtr 'key',
		float 'value'
	}
	apiset 'client'
	returns 'void'

native 'DELETE_RESOURCE_KVP'
	arguments {
		charPtr 'key'
	}
	apiset 'client'
	returns 'void'

native 'START_FIND_KVP'
	arguments {
		charPtr 'prefix'
	}
	apiset 'client'
	returns 'int'

native 'FIND_KVP'
	arguments {
		int 'handle'
	}
	apiset 'client'
	returns 'charPtr'

native 'END_FIND_KVP'
	arguments {
		int 'handle'
	}
	apiset 'client'
	returns 'void'

native 'SET_SNAKEOIL_FOR_ENTRY'
	arguments {
		charPtr 'name',
		charPtr 'path',
		charPtr 'data'
	}
	apiset 'client'
	returns 'void'

native 'SET_MODEL_HEADLIGHT_CONFIGURATION'
	arguments {
		Hash 'modelHash',
		float 'ratePerSecond',
		float 'headlightRotation',
		BOOL 'invertRotation'
	}
	apiset 'client'
	returns 'void'

native 'REGISTER_FONT_FILE'
	arguments {
		charPtr 'fileName'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Registers a specified .gfx file as GFx font library.

	The .gfx file has to be registered with the streamer already.
	</summary>
	<param name="fileName">The name of the .gfx file, without extension.</param>
]]

native 'REGISTER_FONT_ID'
	arguments {
		charPtr 'fontName'
	}
	apiset 'client'
	returns 'int'
	doc [[
	<summary>
	Registers a specified font name for use with text draw commands.
	</summary>
	<param name="fontName">The name of the font in the GFx font library.</param>
	<returns>An index to use with [SET\_TEXT\_FONT](#_0x66E0276CC5F6B9DA) and similar natives.</returns>
]]

native 'GET_INSTANCE_ID'
	arguments {
	}
	apiset 'shared'
	returns 'int'

native 'SET_GAME_TYPE'
	arguments {
		charPtr 'gametypeName'
	}
	apiset 'server'
	returns 'void'

native 'SET_MAP_NAME'
	arguments {
		charPtr 'mapName'
	}
	apiset 'server'
	returns 'void'

native 'GET_NUM_RESOURCES'
	arguments {
	}
	apiset 'shared'
	returns 'int'

native 'GET_RESOURCE_BY_FIND_INDEX'
	arguments {
		int 'findIndex'
	}
	apiset 'shared'
	returns 'charPtr'

native 'START_RESOURCE'
	arguments {
		charPtr 'resourceName'
	}
	apiset 'server'
	returns 'BOOL'

native 'STOP_RESOURCE'
	arguments {
		charPtr 'resourceName'
	}
	apiset 'server'
	returns 'BOOL'

native 'GET_CONVAR'
	arguments {
		charPtr 'varName',
		charPtr 'default'
	}
	apiset 'server'
	returns 'charPtr'

native 'GET_CONVAR_INT'
	arguments {
		charPtr 'varName',
		int 'default'
	}
	apiset 'server'
	returns 'int'

native 'SET_CONVAR'
	arguments {
		charPtr 'varName',
		charPtr 'value'
	}
	apiset 'server'
	returns 'void'

native 'GET_PLAYER_NAME'
	arguments {
		charPtr 'playerSrc'
	}
	apiset 'server'
	returns 'charPtr'

native 'GET_PLAYER_GUID'
	arguments {
		charPtr 'playerSrc'
	}
	apiset 'server'
	returns 'charPtr'

native 'GET_NUM_PLAYER_IDENTIFIERS'
	arguments {
		charPtr 'playerSrc'
	}
	apiset 'server'
	returns 'int'

native 'GET_PLAYER_IDENTIFIER'
	arguments {
		charPtr 'playerSrc',
		int 'identifier'
	}
	apiset 'server'
	returns 'charPtr'

native 'GET_PLAYER_ENDPOINT'
	arguments {
		charPtr 'playerSrc'
	}
	apiset 'server'
	returns 'charPtr'

native 'GET_PLAYER_PING'
	arguments {
		charPtr 'playerSrc'
	}
	apiset 'server'
	returns 'int'

native 'GET_PLAYER_LAST_MSG'
	arguments {
		charPtr 'playerSrc'
	}
	apiset 'server'
	returns 'int'

native 'GET_HOST_ID'
	arguments {
	}
	apiset 'server'
	returns 'charPtr'

native 'DROP_PLAYER'
	arguments {
		charPtr 'playerSrc',
		charPtr 'reason'
	}
	apiset 'server'
	returns 'void'

native 'TEMP_BAN_PLAYER'
	arguments {
		charPtr 'playerSrc',
		charPtr 'reason'
	}
	apiset 'server'
	returns 'void'

native 'GET_INVOKING_RESOURCE'
	arguments {
	}
	apiset 'server'
	returns 'charPtr'

native 'ENABLE_ENHANCED_HOST_SUPPORT'
	arguments {
		BOOL 'enabled'
	}
	apiset 'server'
	returns 'void'

native 'FLAG_SERVER_AS_PRIVATE'
	arguments {
		BOOL 'private'
	}
	apiset 'server'
	returns 'void'

native 'GET_NUM_PLAYER_INDICES'
	arguments {
	}
	apiset 'server'
	returns 'int'

native 'GET_PLAYER_FROM_INDEX'
	arguments {
		int 'index'
	}
	apiset 'server'
	returns 'charPtr'

native 'PERFORM_HTTP_REQUEST_INTERNAL'
	arguments {
		charPtr 'requestData',
		int 'requestDataLength'
	}
	apiset 'server'
	returns 'int'

native 'GET_PASSWORD_HASH'
	arguments {
		charPtr 'password'
	}
	apiset 'server'
	returns 'charPtr'

native 'VERIFY_PASSWORD_HASH'
	arguments {
		charPtr 'password',
		charPtr 'hash'
	}
	apiset 'server'
	returns 'BOOL'

native 'EXECUTE_COMMAND'
	arguments {
		charPtr 'commandString'
	}
	apiset 'shared'
	returns 'void'

native 'REGISTER_COMMAND'
	arguments {
		charPtr 'commandName',
		func 'handler',
		BOOL 'restricted'
	}
	apiset 'shared'
	returns 'void'

native 'IS_ACE_ALLOWED'
	arguments {
		charPtr 'object'
	}
	apiset 'shared'
	returns 'BOOL'

native 'IS_PLAYER_ACE_ALLOWED'
	arguments {
		charPtr 'playerSrc',
		charPtr 'object'
	}
	apiset 'server'
	returns 'BOOL'

native 'FIND_FIRST_PED'
	arguments {
		EntityPtr 'outEntity'
	}
	apiset 'client'
	returns 'int'

native 'FIND_FIRST_VEHICLE'
	arguments {
		EntityPtr 'outEntity'
	}
	apiset 'client'
	returns 'int'

native 'FIND_FIRST_OBJECT'
	arguments {
		EntityPtr 'outEntity'
	}
	apiset 'client'
	returns 'int'

native 'FIND_FIRST_PICKUP'
	arguments {
		EntityPtr 'outEntity'
	}
	apiset 'client'
	returns 'int'

native 'FIND_NEXT_PED'
	arguments {
		int 'findHandle',
		EntityPtr 'outEntity'
	}
	apiset 'client'
	returns 'BOOL'

native 'FIND_NEXT_VEHICLE'
	arguments {
		int 'findHandle',
		EntityPtr 'outEntity'
	}
	apiset 'client'
	returns 'BOOL'

native 'FIND_NEXT_OBJECT'
	arguments {
		int 'findHandle',
		EntityPtr 'outEntity'
	}
	apiset 'client'
	returns 'BOOL'

native 'FIND_NEXT_PICKUP'
	arguments {
		int 'findHandle',
		EntityPtr 'outEntity'
	}
	apiset 'client'
	returns 'BOOL'

native 'END_FIND_PED'
	arguments {
		int 'findHandle'
	}
	apiset 'client'
	returns 'void'

native 'END_FIND_VEHICLE'
	arguments {
		int 'findHandle'
	}
	apiset 'client'
	returns 'void'

native 'END_FIND_OBJECT'
	arguments {
		int 'findHandle'
	}
	apiset 'client'
	returns 'void'

native 'END_FIND_PICKUP'
	arguments {
		int 'findHandle'
	}
	apiset 'client'
	returns 'void'

native 'SET_VEHICLE_AUTO_REPAIR_DISABLED'
	arguments{
		Vehicle 'vehicle',
		BOOL 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Disables the vehicle from being repaired when a vehicle extra is enabled.
	</summary>
	<param name="vehicle">The vehicle to set disable auto vehicle repair.</param>
	<param name="value">Setting the value to  true prevents the vehicle from being repaired when a extra is enabled. Setting the value to false allows the vehicle from being repaired when a extra is enabled.</param>
	]]

native 'GET_VEHICLE_FUEL_LEVEL'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'SET_VEHICLE_FUEL_LEVEL'
	arguments {
		Vehicle 'vehicle',
		float 'level'
	}
	apiset 'client'
	returns 'void'

native 'GET_VEHICLE_OIL_LEVEL'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'SET_VEHICLE_OIL_LEVEL'
	arguments {
		Vehicle 'vehicle',
		float 'level'
	}
	apiset 'client'
	returns 'void'

native 'GET_VEHICLE_GRAVITY_AMOUNT'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'IS_VEHICLE_ENGINE_STARTING'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'BOOL'

native 'GET_VEHICLE_WHEEL_SPEED'
	arguments {
		Vehicle 'vehicle',
		int 'wheelIndex'
	}
	apiset 'client'
	returns 'float'
	doc [[
	<summary>
	Gets speed of a wheel at the tyre. 
	Max number of wheels can be retrieved with the native GET_VEHICLE_NUMBER_OF_WHEELS.
	</summary>
	<returns>An integer.</returns>
]]
	
native 'GET_VEHICLE_DASHBOARD_SPEED'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'GET_VEHICLE_ACCELERATION'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'SET_VEHICLE_GRAVITY_AMOUNT'
	arguments {
		Vehicle 'vehicle',
		float 'gravity'
	}
	apiset 'client'
	returns 'void'

native 'GET_VEHICLE_CURRENT_RPM'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'SET_VEHICLE_CURRENT_RPM'
	arguments {
		Vehicle 'vehicle',
		float 'rpm'
	}
	apiset 'client'
	returns 'void'

native 'GET_VEHICLE_HIGH_GEAR'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'int'

native 'SET_VEHICLE_HIGH_GEAR'
	arguments {
		Vehicle 'vehicle',
		int 'gear'
	}
	apiset 'client'
	returns 'void'

native 'GET_VEHICLE_CURRENT_GEAR'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'int'

native 'GET_VEHICLE_NEXT_GEAR'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'int'

native 'GET_VEHICLE_STEERING_ANGLE'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'SET_VEHICLE_STEERING_ANGLE'
	arguments {
		Vehicle 'vehicle',
		float 'angle'
	}
	apiset 'client'
	returns 'void'

native 'GET_VEHICLE_STEERING_SCALE'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'SET_VEHICLE_STEERING_SCALE'
	arguments {
		Vehicle 'vehicle',
		float 'scale'
	}
	apiset 'client'
	returns 'void'

native 'IS_VEHICLE_ALARM_SET'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'BOOL'

native 'GET_VEHICLE_ALARM_TIME_LEFT'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'int'

native 'SET_VEHICLE_ALARM_TIME_LEFT'
	arguments {
		Vehicle 'vehicle',
		int 'time'
	}
	apiset 'client'
	returns 'void'

native 'IS_VEHICLE_WANTED'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'BOOL'

native 'IS_VEHICLE_PREVIOUSLY_OWNED_BY_PLAYER'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'BOOL'

native 'IS_VEHICLE_NEEDS_TO_BE_HOTWIRED'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'BOOL'

native 'IS_VEHICLE_INTERIOR_LIGHT_ON'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'BOOL'

native 'GET_VEHICLE_ENGINE_TEMPERATURE'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'
	
native 'SET_VEHICLE_ENGINE_TEMPERATURE'
	arguments {
		Vehicle 'vehicle',
		float 'temperature'
	}
	apiset 'client'
	returns 'void'
	
native 'GET_VEHICLE_CLUTCH'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'
	
native 'SET_VEHICLE_CLUTCH'
	arguments {
		Vehicle 'vehicle',
		float 'clutch'
	}
	apiset 'client'
	returns 'void'	
	
native 'GET_VEHICLE_TURBO_PRESSURE'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'float'

native 'SET_VEHICLE_TURBO_PRESSURE'
	arguments {
		Vehicle 'vehicle',
		float 'pressure'
	}
	apiset 'client'
	returns 'void'
	
native 'GET_VEHICLE_HANDBRAKE'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'BOOL'
	
native 'GET_VEHICLE_NUMBER_OF_WHEELS'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'int'

native 'GET_VEHICLE_INDICATOR_LIGHTS'
	arguments {
		Vehicle 'vehicle'
	}
	apiset 'client'
	returns 'int'	
	doc [[
	<summary>
	Gets the vehicle indicator light state. 0 = off, 1 = left, 2 = right, 3 = both
	</summary>
	<returns>An integer.</returns>
]]

native 'GET_VEHICLE_WHEEL_HEALTH'
	arguments {
		Vehicle 'vehicle',
		int 'wheelIndex',
	}
	apiset 'client'
	returns 'float'
	
native 'SET_VEHICLE_WHEEL_HEALTH'
	arguments {
		Vehicle 'vehicle',
		int 'wheelIndex',
		float 'health'
	}
	apiset 'client'
	returns 'void'

native 'SET_HTTP_HANDLER'
	arguments {
		func 'handler'	
	}
	apiset 'server'
	returns 'void'

native 'GET_CAM_MATRIX'
	arguments {
		Cam 'camera',
		Vector3Ptr 'rightVector',
		Vector3Ptr 'forwardVector',
		Vector3Ptr 'upVector',
		Vector3Ptr 'position',
	}
	apiset 'client'
	returns	'void'
	doc [[
	<summary>
	Returns the world matrix of the specified camera. To turn this into a view matrix, calculate the inverse.
	</summary>
]]

native 'EXPERIMENTAL_SAVE_CLONE_CREATE'
	arguments {
		Entity 'entity'
	}
	apiset 'client'
	returns 'charPtr'
	doc [[
	<summary>
	This native is not implemented.
	</summary>
]]

native 'EXPERIMENTAL_SAVE_CLONE_SYNC'
	arguments {
		Entity 'entity'
	}
	apiset 'client'
	returns 'charPtr'
	doc [[
	<summary>
	This native is not implemented.
	</summary>
]]

native 'EXPERIMENTAL_LOAD_CLONE_CREATE'
	arguments {
		charPtr 'data',
		int 'objectId',
		charPtr 'tree'
	}
	apiset 'client'
	returns 'Entity'
	doc [[
	<summary>
	This native is not implemented.
	</summary>
]]

native 'EXPERIMENTAL_LOAD_CLONE_SYNC'
	arguments {
		Entity 'entity',
		charPtr 'data'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	This native is not implemented.
	</summary>
]]

native 'SET_HANDLING_FIELD'
	arguments {
		charPtr 'vehicle',
		charPtr 'class',
		charPtr 'fieldName',
		Any 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets a global handling override for a specific vehicle class. The name is supposed to match the `handlingName` field from handling.meta.

	Example: `SetHandlingField('AIRTUG', 'CHandlingData', 'fSteeringLock', 360.0)`
	</summary>
	<param name="vehicle">The vehicle class to set data for.</param>
	<param name="class">The handling class to set. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to set. These match the keys in `handling.meta`.</param>
	<param name="value">The value to set.</param>
]]

native 'SET_HANDLING_FLOAT'
	arguments {
		charPtr 'vehicle',
		charPtr 'class',
		charPtr 'fieldName',
		float 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets a global handling override for a specific vehicle class. The name is supposed to match the `handlingName` field from handling.meta.

	Example: `SetHandlingFloat('AIRTUG', 'CHandlingData', 'fSteeringLock', 360.0)`
	</summary>
	<param name="vehicle">The vehicle class to set data for.</param>
	<param name="class">The handling class to set. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to set. These match the keys in `handling.meta`.</param>
	<param name="value">The floating-point value to set.</param>
]]

native 'SET_HANDLING_INT'
	arguments {
		charPtr 'vehicle',
		charPtr 'class',
		charPtr 'fieldName',
		int 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets a global handling override for a specific vehicle class. The name is supposed to match the `handlingName` field from handling.meta.
	</summary>
	<param name="vehicle">The vehicle class to set data for.</param>
	<param name="class">The handling class to set. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to set. These match the keys in `handling.meta`.</param>
	<param name="value">The integer value to set.</param>
]]

native 'SET_HANDLING_VECTOR'
	arguments {
		charPtr 'vehicle',
		charPtr 'class',
		charPtr 'fieldName',
		Vector3 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets a global handling override for a specific vehicle class. The name is supposed to match the `handlingName` field from handling.meta.

	Example: `SetHandlingVector('AIRTUG', 'CHandlingData', 'vecCentreOfMassOffset', vector3(0.0, 0.0, -5.0))`
	</summary>
	<param name="vehicle">The vehicle class to set data for.</param>
	<param name="class">The handling class to set. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to set. These match the keys in `handling.meta`.</param>
	<param name="value">The Vector3 value to set.</param>
]]

native 'GET_VEHICLE_HANDLING_FLOAT'
	arguments {
		Vehicle 'vehicle',
		charPtr 'class',
		charPtr 'fieldName'
	}
	apiset 'client'
	returns 'float'
	doc [[
	<summary>
	Returns the effective handling data of a vehicle as a floating-point value.

	Example: `local fSteeringLock = GetVehicleHandlingFloat(vehicle, 'CHandlingData', 'fSteeringLock')`
	</summary>
	<param name="vehicle">The vehicle to obtain data for.</param>
	<param name="class">The handling class to get. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to get. These match the keys in `handling.meta`.</param>
	<returns>A floating-point value.</returns>
]]

native 'GET_VEHICLE_HANDLING_INT'
	arguments {
		Vehicle 'vehicle',
		charPtr 'class',
		charPtr 'fieldName'
	}
	apiset 'client'
	returns 'int'
	doc [[
	<summary>
	Returns the effective handling data of a vehicle as an integer value.

	Example: `local modelFlags = GetVehicleHandlingInt(vehicle, 'CHandlingData', 'strModelFlags')`
	</summary>
	<param name="vehicle">The vehicle to obtain data for.</param>
	<param name="class">The handling class to get. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to get. These match the keys in `handling.meta`.</param>
	<returns>An integer.</returns>
]]

native 'GET_VEHICLE_HANDLING_VECTOR'
	arguments {
		Vehicle 'vehicle',
		charPtr 'class',
		charPtr 'fieldName'
	}
	apiset 'client'
	returns 'Vector3'
	doc [[
	<summary>
	Returns the effective handling data of a vehicle as a vector value.

	Example: `local inertiaMultiplier = GetVehicleHandlingVector(vehicle, 'CHandlingData', 'vecInertiaMultiplier')`
	</summary>
	<param name="vehicle">The vehicle to obtain data for.</param>
	<param name="class">The handling class to get. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to get. These match the keys in `handling.meta`.</param>
	<returns>An integer.</returns>
]]

native 'SET_VEHICLE_HANDLING_FIELD'
	arguments {
		Vehicle 'vehicle',
		charPtr 'class',
		charPtr 'fieldName',
		Any 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets a handling override for a specific vehicle. Certain handling flags can only be set globally using `SET_HANDLING_FIELD`, this might require some experimentation.

	Example: `SetVehicleHandlingField(vehicle, 'CHandlingData', 'fSteeringLock', 360.0)`
	</summary>
	<param name="vehicle">The vehicle to set data for.</param>
	<param name="class">The handling class to set. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to set. These match the keys in `handling.meta`.</param>
	<param name="value">The value to set.</param>
]]

native 'SET_VEHICLE_HANDLING_FLOAT'
	arguments {
		Vehicle 'vehicle',
		charPtr 'class',
		charPtr 'fieldName',
		float 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets a handling override for a specific vehicle. Certain handling flags can only be set globally using `SET_HANDLING_FLOAT`, this might require some experimentation.

	Example: `SetVehicleHandlingFloat(vehicle, 'CHandlingData', 'fSteeringLock', 360.0)`
	</summary>
	<param name="vehicle">The vehicle to set data for.</param>
	<param name="class">The handling class to set. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to set. These match the keys in `handling.meta`.</param>
	<param name="value">The floating-point value to set.</param>
]]

native 'SET_VEHICLE_HANDLING_INT'
	arguments {
		Vehicle 'vehicle',
		charPtr 'class',
		charPtr 'fieldName',
		int 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets a handling override for a specific vehicle. Certain handling flags can only be set globally using `SET_HANDLING_INT`, this might require some experimentation.
	</summary>
	<param name="vehicle">The vehicle to set data for.</param>
	<param name="class">The handling class to set. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to set. These match the keys in `handling.meta`.</param>
	<param name="value">The integer value to set.</param>
]]

native 'SET_VEHICLE_HANDLING_VECTOR'
	arguments {
		Vehicle 'vehicle',
		charPtr 'class',
		charPtr 'fieldName',
		Vector3 'value'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets a handling override for a specific vehicle. Certain handling flags can only be set globally using `SET_HANDLING_VECTOR`, this might require some experimentation.
	</summary>
	<param name="vehicle">The vehicle to set data for.</param>
	<param name="class">The handling class to set. Only "CHandlingData" is supported at this time.</param>
	<param name="fieldName">The field name to set. These match the keys in `handling.meta`.</param>
	<param name="value">The Vector3 value to set.</param>
]]

native 'GET_VEHICLE_WHEEL_XROT'
	arguments {
		Vehicle 'vehicle',
		int 'wheelIndex'
	}
	apiset 'client'
	returns 'float'

native 'SET_VEHICLE_WHEEL_XROT'
	arguments {
		Vehicle 'vehicle',
		int 'wheelIndex',
		float 'value'
	}
	apiset 'client'
	returns 'void'

native 'GET_VEHICLE_WHEEL_X_OFFSET'
	arguments {
		Vehicle 'vehicle',
		int 'wheelIndex'
	}
	apiset 'client'
	returns 'float'
	doc [[
	<summary>
	Returns the offset of the specified wheel relative to the wheel's axle center.
	</summary>
]]

native 'SET_VEHICLE_WHEEL_X_OFFSET'
	arguments {
		Vehicle 'vehicle',
		int 'wheelIndex',
		float 'offset'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Adjusts the offset of the specified wheel relative to the wheel's axle center.
	Needs to be called every frame in order to function properly, as GTA will reset the offset otherwise.

	This function can be especially useful to set the track width of a vehicle, for example:
	```
	function SetVehicleFrontTrackWidth(vehicle, width)
		SetVehicleWheelXOffset(vehicle, 0, -width/2)
		SetVehicleWheelXOffset(vehicle, 1, width/2)
	end
	```
	</summary>
]]

native 'SET_PLAYER_TALKING_OVERRIDE'
	arguments {
		Player 'player',
		BOOL 'state'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	the status of default voip system. It affects on `NETWORK_IS_PLAYER_TALKING` and `mp_facial` animation.
	This function doesn't need to be called every frame, it works like a switcher.
	</summary>
	<param name="player">The target player.</param>
	<param name="state">Overriding state.</param>
]]

native 'SEND_LOADING_SCREEN_MESSAGE'
	arguments {
		charPtr 'jsonString'
	}
	apiset 'client'
	returns 'BOOL'
	doc [[
	<summary>
	Sends a message to the `loadingScreen` NUI frame, which contains the HTML page referenced in `loadscreen` resources.
	</summary>
	<param name="jsonString">The JSON-encoded message.</param>
	<returns>A success value.</returns>
]]

native 'SET_MANUAL_SHUTDOWN_LOADING_SCREEN_NUI'
	arguments {
		BOOL 'manualShutdown'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets whether or not `SHUTDOWN_LOADING_SCREEN` automatically shuts down the NUI frame for the loading screen. If this is enabled,
	you will have to manually invoke `SHUTDOWN_LOADING_SCREEN_NUI` whenever you want to hide the NUI loading screen.
	</summary>
	<param name="manualShutdown">TRUE to manually shut down the loading screen NUI.</param>
]]

native 'SHUTDOWN_LOADING_SCREEN_NUI'
	arguments {}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Shuts down the `loadingScreen` NUI frame, similarly to `SHUTDOWN_LOADING_SCREEN`.
	</summary>
]]

native 'ADD_MINIMAP_OVERLAY'
	arguments {
		charPtr 'name'
	}
	apiset 'client'
	returns 'int'
	doc [[
	<summary>
	Loads a minimap overlay from a GFx file in the current resource.
	</summary>
	<param name="name">The path to a `.gfx` file in the current resource. It has to be specified as a `file`.</param>
	<returns>A minimap overlay ID.</returns>
]]

native 'HAS_MINIMAP_OVERLAY_LOADED'
	arguments {
		int 'id'
	}
	apiset 'client'
	returns 'BOOL'
	doc [[
	<summary>
	Returns whether or not the specific minimap overlay has loaded.
	</summary>
	<param name="id">A minimap overlay ID.</param>
	<returns>A boolean indicating load status.</returns>
]]

native 'CALL_MINIMAP_SCALEFORM_FUNCTION'
	arguments {
		int 'miniMap',
		charPtr 'fnName'
	}
	apiset 'client'
	returns 'BOOL'
	doc [[
	<summary>
	This is similar to the PushScaleformMovieFunction natives, except it calls in the `TIMELINE` of a minimap overlay.
	</summary>
	<param name="miniMap">The minimap overlay ID.</param>
	<param name="fnName">A function in the overlay's TIMELINE.</param>
]]

native 'SET_MINIMAP_OVERLAY_DISPLAY'
	arguments {
		int 'miniMap',
		float 'x',
		float 'y',
		float 'xScale',
		float 'yScale',
		float 'alpha'
	}
	apiset 'client'
	returns 'void'
	doc [[
	<summary>
	Sets the display info for a minimap overlay.
	</summary>
	<param name="miniMap">The minimap overlay ID.</param>
	<param name="x">The X position for the overlay. This is equivalent to a game coordinate X.</param>
	<param name="y">The Y position for the overlay. This is equivalent to a game coordinate Y, except that it's inverted (gfxY = -gameY).</param>
	<param name="xScale">The X scale for the overlay. This is equivalent to the Flash _xscale property, therefore 100 = 100%.</param>
	<param name="yScale">The Y scale for the overlay. This is equivalent to the Flash _yscale property.</param>
	<param name="alpha">The alpha value for the overlay. This is equivalent to the Flash _alpha property, therefore 100 = 100%.</param>
]]

native 'GET_REGISTERED_COMMANDS'
	arguments {
	}
	returns 'object'
	doc [[
	<summary>
	Returns all commands that are registered in the command system.
	The data returned adheres to the following layout:
	```
	[
		{
			"name": "cmdlist"
		},
		{
			"name": "command1"
		}
	]
	```
	</summary>
	<returns>An object containing registered commands.</returns>
]]
	apiset 'shared'

native 'SET_RICH_PRESENCE'
	arguments {
		charPtr 'presenceState'
	}
	returns 'void'
	apiset 'client'
	doc [[
	<summary>
	Sets the player's rich presence detail state for social platform providers to a specified string.
	</summary>
	<param name="presenceState">The rich presence string to set.</param>
]]

native 'REGISTER_RESOURCE_BUILD_TASK_FACTORY'
	arguments {
		charPtr 'factoryId',
		func 'factoryFn'
	}
	returns 'void'
	apiset 'server'
	doc [[
	<summary>
	Registers a build task factory for resources.
	The function should return an object (msgpack map) with the following fields:
	```
	{
		// returns whether the specific resource should be built
		shouldBuild = func(resourceName: string): bool,
		
		// asynchronously start building the specific resource.
		// call cb when completed
		build = func(resourceName: string, cb: func(success: bool, status: string): void): void
	}
	```
	</summary>
	<param name="factoryId">The identifier for the build task.</param>
	<param name="factoryFn">The factory function.</param>
]]

native 'GET_PLAYER_PED'
	arguments {
		charPtr 'playerSrc'
	}
	apiset 'server'
	returns 'Entity'

native 'GET_ENTITY_COORDS'
	arguments {
		Entity 'entity'
	}
	apiset 'server'
	returns 'Vector3'

native "GET_HASH_KEY"
	arguments {
		charPtr "model",
	}
	apiset 'server'
	returns	"Hash"
	doc [[!
<summary>
		This native converts the passed string to a hash.
</summary>
	]]
