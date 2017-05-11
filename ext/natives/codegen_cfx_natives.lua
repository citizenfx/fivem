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

-- TODO: handling field natives
