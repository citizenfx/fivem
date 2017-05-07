native 'GET_GAME_TIMER'
	hash '0xa4ea0691'
	arguments {}
	apiset 'server'
	returns 'int'

native 'IS_DUPLICITY_VERSION'
	hash '0xcf24c52e'
	arguments {}
	apiset 'shared'
	returns 'BOOL'

native 'TRIGGER_EVENT_INTERNAL'
	hash '0x91310870'
	arguments {
		charPtr 'eventName',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'shared'
	returns 'void'

native 'TRIGGER_SERVER_EVENT_INTERNAL'
	hash '0x7fdd1128'
	arguments {
		charPtr 'eventName',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'client'
	returns 'void'

native 'TRIGGER_CLIENT_EVENT_INTERNAL'
	hash '0x2f7a49e6'
	arguments {
		charPtr 'eventName',
		charPtr 'eventTarget',
		charPtr 'eventPayload',
		int 'payloadLength'
	}
	apiset 'server'
	returns 'void'

native 'CANCEL_EVENT'
	hash '0xfa29d35d'
	arguments {}
	returns 'void'
	apiset 'shared'

native 'WAS_EVENT_CANCELED'
	hash '0x58382a19'
	arguments {}
	returns 'BOOL'
	apiset 'shared'

native 'INVOKE_FUNCTION_REFERENCE'
	hash '0xe3551879'
	arguments {
		charPtr 'referenceIdentity',
		charPtr 'argsSerialized',
		int 'argsLength',
		intPtr 'retvalLength'
	}
	apiset 'shared'
	returns 'charPtr'

native 'DUPLICATE_FUNCTION_REFERENCE'
	hash '0xf4e2079d'
	arguments {
		charPtr 'referenceIdentity'
	}
	apiset 'shared'
	returns 'charPtr'

native 'DELETE_FUNCTION_REFERENCE'
	hash '0x1e86f206'
	arguments {
		charPtr 'referenceIdentity'
	}
	apiset 'shared'
	returns 'void'

native 'GET_NUM_RESOURCE_METADATA'
	hash '0x0776e864'
	arguments {
		charPtr 'resourceName',
		charPtr 'metadataKey'
	}
	apiset 'shared'
	returns 'int'

native 'GET_RESOURCE_METADATA'
	hash '0x964bab1d'
	arguments {
		charPtr 'resourceName',
		charPtr 'metadataKey',
		int 'index'
	}
	apiset 'shared'
	returns 'charPtr'

native 'LOAD_RESOURCE_FILE'
	hash '0x76a9ee1f'
	arguments {
		charPtr 'resourceName',
		charPtr 'fileName'
	}
	apiset 'shared'
	returns 'charPtr'

native 'GET_CURRENT_RESOURCE_NAME'
	hash '0xe5e9ebbb'
	arguments {}
	apiset 'shared'
	returns 'charPtr'

native 'GET_PLAYER_FROM_SERVER_ID'
	hash '0x344ea166'
	arguments {
		int 'serverId'
	}
	apiset 'client'
	returns 'Player'

native 'GET_PLAYER_SERVER_ID'
	hash '0x4d97bcc7'
	arguments {
		Player 'player'
	}
	apiset 'client'
	returns 'int'

native 'SEND_NUI_MESSAGE'
	hash '0x78608acb'
	arguments {
		charPtr 'jsonString'
	}
	apiset 'client'
	returns 'BOOL'

native 'REGISTER_NUI_CALLBACK_TYPE'
	hash '0xcd03cda9'
	arguments {
		charPtr 'callbackType'
	}
	apiset 'client'
	returns 'void'

native 'SET_NUI_FOCUS'
	hash '0x5b98ae30'
	arguments {
		BOOL 'hasFocus'
	}
	apiset 'client'
	returns 'void'

native 'SET_TEXT_CHAT_ENABLED'
	hash '0x97b2f9f8'
	arguments {
		BOOL 'enabled'
	}
	apiset 'client'
	returns 'BOOL'

native 'ADD_TEXT_ENTRY'
	hash '0x32ca01c3'
	arguments {
		charPtr 'entryKey',
		charPtr 'entryText'
	}
	apiset 'client'
	returns 'void'

native 'ADD_TEXT_ENTRY_BY_HASH'
	hash '0x289da860'
	arguments {
		Hash 'entryKey',
		charPtr 'entryText'
	}
	apiset 'client'
	returns 'void'

native 'GET_RESOURCE_KVP_INT'
	hash '0x557b586a'
	arguments {
		charPtr 'key'
	}
	apiset 'client'
	returns 'int'

native 'GET_RESOURCE_KVP_FLOAT'
	hash '0x35bdceea'
	arguments {
		charPtr 'key'
	}
	apiset 'client'
	returns 'float'

native 'GET_RESOURCE_KVP_STRING'
	hash '0x5240da5a'
	arguments {
		charPtr 'key'
	}
	apiset 'client'
	returns 'charPtr'

native 'SET_RESOURCE_KVP'
	hash '0x21c7a35b'
	arguments {
		charPtr 'key',
		charPtr 'value'
	}
	apiset 'client'
	returns 'void'

native 'SET_RESOURCE_KVP_INT'
	hash '0x06a2b1e8'
	arguments {
		charPtr 'key',
		int 'value'
	}
	apiset 'client'
	returns 'void'

native 'SET_RESOURCE_KVP_FLOAT'
	hash '0x9add2938'
	arguments {
		charPtr 'key',
		float 'value'
	}
	apiset 'client'
	returns 'void'

native 'DELETE_RESOURCE_KVP'
	hash '0x7389b5df'
	arguments {
		charPtr 'key'
	}
	apiset 'client'
	returns 'void'

native 'START_FIND_KVP'
	hash '0xdd379006'
	arguments {
		charPtr 'prefix'
	}
	apiset 'client'
	returns 'int'

native 'FIND_KVP'
	hash '0xbd7bebc5'
	arguments {
		int 'handle'
	}
	apiset 'client'
	returns 'charPtr'

native 'END_FIND_KVP'
	hash '0xb3210203'
	arguments {
		int 'handle'
	}
	apiset 'client'
	returns 'void'

native 'SET_SNAKEOIL_FOR_ENTRY'
	hash '0xa7dd3209'
	arguments {
		charPtr 'name',
		charPtr 'path',
		charPtr 'data'
	}
	apiset 'client'
	returns 'void'

native 'SET_MODEL_HEADLIGHT_CONFIGURATION'
	hash '0x7f6b8d75'
	arguments {
		Hash 'modelHash',
		float 'ratePerSecond',
		float 'headlightRotation',
		BOOL 'invertRotation'
	}
	apiset 'client'
	returns 'void'

native 'REGISTER_FONT_FILE'
	hash '0x01b3a363'
	arguments {
		charPtr 'fileName'
	}
	apiset 'client'
	returns 'void'

native 'REGISTER_FONT_ID'
	hash '0xacf6d8ee'
	arguments {
		charPtr 'fontName'
	}
	apiset 'client'
	returns 'int'


-- TODO: handling field natives
