native 'GET_GAME_TIMER'
	arguments {}
	apiset 'server'
	returns 'int'
	doc [[
<summary>
Gets the current game timer in milliseconds.
</summary>
<returns>The game time.</returns>
]]

native 'IS_DUPLICITY_VERSION'
	arguments {}
	apiset 'shared'
	returns 'BOOL'
	doc [[
<summary>Gets whether or not this is the CitizenFX server.</summary>
<returns>A boolean value.</returns>
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
	apiset 'server'
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

native 'GET_VEHICLE_GRAVITY'
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

native 'SET_VEHICLE_GRAVITY'
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
	returns 'float'

native 'SET_VEHICLE_HIGH_GEAR'
	arguments {
		Vehicle 'vehicle',
		float 'gear'
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

-- TODO: handling field natives
