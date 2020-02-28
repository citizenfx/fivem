context_rpc 'SET_ENTITY_COORDS' { 'GET_ENTITY_COORDS' } -- bad getter example as creation needs to set this too

context_rpc 'SET_VEHICLE_COLOURS'

context_rpc 'SET_PLAYER_WANTED_LEVEL'

context_rpc 'SET_PED_INTO_VEHICLE'

context_rpc 'SET_VEHICLE_NUMBER_PLATE_TEXT'

context_rpc 'FREEZE_ENTITY_POSITION'

context_rpc 'SET_VEHICLE_DOORS_LOCKED'

entity_rpc 'CREATE_VEHICLE'
entity_rpc 'CREATE_PED'
entity_rpc 'CREATE_OBJECT'
entity_rpc 'CREATE_OBJECT_NO_OFFSET'

context_rpc 'SET_ENTITY_ROTATION'

context_rpc 'SET_ENTITY_VELOCITY'

context_rpc 'SET_ENTITY_HEADING'

context_rpc 'SET_ENTITY_ROTATION_VELOCITY'

object_rpc '_ADD_BLIP_FOR_AREA'
object_rpc 'ADD_BLIP_FOR_COORD'
object_rpc 'ADD_BLIP_FOR_RADIUS'

-- is it a good idea to expose this as long as we don't have some way to create surrogate blips for entities
-- far away/recreate this for entities disappearing?
object_rpc 'ADD_BLIP_FOR_ENTITY'

context_rpc 'SET_BLIP_SPRITE' { 'GET_BLIP_SPRITE' }
context_rpc 'REMOVE_BLIP'

--[[new_context_rpc '_SET_FOCUS_AREA'
    new_context_arg {
        Player 'target'
    }]]
