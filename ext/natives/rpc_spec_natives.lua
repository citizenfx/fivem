-- ENTITY
context_rpc 'SET_ENTITY_COORDS' { 'GET_ENTITY_COORDS' } -- bad getter example as creation needs to set this too
context_rpc 'SET_ENTITY_ROTATION'
context_rpc 'SET_ENTITY_VELOCITY'
context_rpc 'SET_ENTITY_HEADING'
context_rpc 'SET_ENTITY_ROTATION_VELOCITY'
context_rpc 'FREEZE_ENTITY_POSITION'
context_rpc 'APPLY_FORCE_TO_ENTITY'

-- PLAYER
context_rpc 'SET_PLAYER_MODEL'
context_rpc 'SET_PLAYER_CONTROL'
context_rpc 'SET_PLAYER_INVINCIBLE'
context_rpc 'SET_PLAYER_WANTED_LEVEL'
context_rpc 'CLEAR_PLAYER_WANTED_LEVEL'

-- PED
entity_rpc 'CREATE_PED'
entity_rpc 'CREATE_PED_INSIDE_VEHICLE'
context_rpc 'ADD_PED_DECORATION_FROM_HASHES'
context_rpc 'SET_PED_INTO_VEHICLE'
context_rpc 'SET_PED_HEAD_BLEND_DATA'
context_rpc 'SET_PED_HEAD_OVERLAY'
context_rpc '_SET_PED_HAIR_COLOR'
context_rpc '_SET_PED_HEAD_OVERLAY_COLOR'
context_rpc '_SET_PED_EYE_COLOR'
context_rpc '_SET_PED_FACE_FEATURE'
context_rpc 'SET_PED_DEFAULT_COMPONENT_VARIATION'
context_rpc 'SET_PED_RANDOM_COMPONENT_VARIATION'
context_rpc 'SET_PED_COMPONENT_VARIATION'
context_rpc 'CLEAR_PED_PROP'
context_rpc 'SET_PED_RANDOM_PROPS'
context_rpc 'SET_PED_PROP_INDEX'
context_rpc 'SET_PED_ARMOUR'
context_rpc 'SET_PED_CAN_RAGDOLL'
context_rpc 'SET_PED_CONFIG_FLAG'
context_rpc 'SET_PED_RESET_FLAG'
context_rpc 'SET_PED_TO_RAGDOLL'
context_rpc 'SET_PED_TO_RAGDOLL_WITH_FALL'

-- WEAPON
context_rpc 'GIVE_WEAPON_TO_PED'
context_rpc 'GIVE_WEAPON_COMPONENT_TO_PED'
context_rpc 'REMOVE_WEAPON_FROM_PED'
context_rpc 'REMOVE_ALL_PED_WEAPONS'
context_rpc 'REMOVE_WEAPON_COMPONENT_FROM_PED'
context_rpc 'SET_CURRENT_PED_WEAPON'
context_rpc 'SET_PED_AMMO'

-- TASK
context_rpc 'TASK_REACT_AND_FLEE_PED'
context_rpc 'TASK_SHOOT_AT_COORD'
context_rpc 'TASK_SHOOT_AT_ENTITY'
context_rpc 'TASK_COMBAT_PED'
context_rpc 'TASK_DRIVE_BY'
context_rpc 'TASK_ENTER_VEHICLE'
context_rpc 'TASK_WARP_PED_INTO_VEHICLE'
context_rpc 'TASK_HANDS_UP'
context_rpc 'TASK_PLAY_ANIM'
context_rpc 'TASK_PLAY_ANIM_ADVANCED'
context_rpc 'CLEAR_PED_TASKS'
context_rpc 'CLEAR_PED_TASKS_IMMEDIATELY'
context_rpc 'CLEAR_PED_SECONDARY_TASK'
context_rpc 'TASK_EVERYONE_LEAVE_VEHICLE'
context_rpc 'TASK_LEAVE_ANY_VEHICLE'
context_rpc 'TASK_LEAVE_VEHICLE'
context_rpc 'TASK_GO_STRAIGHT_TO_COORD'
context_rpc 'TASK_GO_TO_COORD_ANY_MEANS'
context_rpc 'TASK_GO_TO_ENTITY'

-- VEHICLE
entity_rpc 'CREATE_VEHICLE'
context_rpc 'SET_VEHICLE_ALARM'
context_rpc 'SET_VEHICLE_BODY_HEALTH'
context_rpc 'SET_VEHICLE_COLOURS'
context_rpc 'SET_VEHICLE_COLOUR_COMBINATION'
context_rpc 'SET_VEHICLE_CUSTOM_PRIMARY_COLOUR'
context_rpc 'SET_VEHICLE_CUSTOM_SECONDARY_COLOUR'
context_rpc 'SET_VEHICLE_DIRT_LEVEL'
context_rpc 'SET_VEHICLE_DOOR_BROKEN'
context_rpc 'SET_VEHICLE_NUMBER_PLATE_TEXT'
context_rpc 'SET_VEHICLE_DOORS_LOCKED'

-- OBJECT
entity_rpc 'CREATE_OBJECT'
entity_rpc 'CREATE_OBJECT_NO_OFFSET'

-- BLIP
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
