context_rpc 'SET_ENTITY_COORDS' 

context_rpc 'SET_VEHICLE_COLOURS'

context_rpc 'SET_PLAYER_WANTED_LEVEL'

context_rpc 'SET_PED_INTO_VEHICLE'

-- no charPtr support yet
--context_rpc 'SET_VEHICLE_NUMBER_PLATE_TEXT'

context_rpc 'FREEZE_ENTITY_POSITION'

entity_rpc 'CREATE_VEHICLE'

context_rpc 'SET_ENTITY_ROTATION'

context_rpc 'SET_ENTITY_VELOCITY'

context_rpc 'SET_ENTITY_HEADING'

context_rpc 'SET_ENTITY_ROTATION_VELOCITY'

--[[object_rpc 'ADD_BLIP_FOR_ENTITY'

new_context_rpc '_SET_FOCUS_AREA'
    new_context_arg {
        Player 'target'
    }]]