native 'GET_GAME_TIMER'
	arguments {}
	apiset 'server'
	returns 'int'

native 'IS_DUPLICITY_VERSION'
	arguments {}
	apiset 'shared'
	returns 'BOOL'

native 'TRIGGER_EVENT_INTERNAL'
	arguments {
		charPtr 'eventName',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'shared'
	returns 'void'

native 'TRIGGER_SERVER_EVENT_INTERNAL'
	arguments {
		charPtr 'eventName',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'client'
	returns 'void'

native 'TRIGGER_CLIENT_EVENT_INTERNAL'
	arguments {
		charPtr 'eventName',
		charPtr 'eventTarget',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'server'
	returns 'void'

native 'CANCEL_EVENT'
	arguments {}
	returns 'void'
	apiset 'shared'

native 'WAS_EVENT_CANCELED'
	arguments {}
	returns 'BOOL'
	apiset 'shared'

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

native 'GET_RESOURCE_METADATA'
	arguments {
		charPtr 'resourceName',
		charPtr 'metadataKey',
		int 'index'
	}
	apiset 'shared'
	returns 'charPtr'

native 'LOAD_RESOURCE_FILE'
	arguments {
		charPtr 'resourceName',
		charPtr 'fileName'
	}
	apiset 'shared'
	returns 'charPtr'

native 'GET_CURRENT_RESOURCE_NAME'
	arguments {}
	apiset 'shared'
	returns 'charPtr'

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
		BOOL 'hasFocus'
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

native 'REGISTER_FONT_ID'
	arguments {
		charPtr 'fontName'
	}
	apiset 'client'
	returns 'int'

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

-- TODO: handling field natives
