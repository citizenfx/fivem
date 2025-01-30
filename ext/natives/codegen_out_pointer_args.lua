PAS_FLAG_BLOCKED = 0x80000000
PAS_FLAG_TRIVIAL = 0x40000000
PAS_FLAG_UNSAFE  = 0x20000000

PAS_ARG_POINTER = 0x80000000
PAS_ARG_STRING  = 0x40000000
PAS_ARG_BUFFER  = 0x20000000
PAS_ARG_BOUND   = 0x10000000
PAS_ARG_SIZE    = 0x0FFFFFFF

PAS_RET_VOID = 0
PAS_RET_INT = 1
PAS_RET_FLOAT = 2
PAS_RET_LONG = 3
PAS_RET_VECTOR3 = 4
PAS_RET_STRING = 5
PAS_RET_OBJECT = 7

local typeSizes = {
    ['Vector3'] = 24,
}

local paramOverrides = {
    ['CFX/SET_RUNTIME_TEXTURE_ARGB_DATA/1'] = PAS_ARG_POINTER | PAS_ARG_BUFFER, -- buffer
    ['CFX/SET_STATE_BAG_VALUE/2']           = PAS_ARG_POINTER | PAS_ARG_BUFFER, -- valueData
    ['CFX/PERFORM_HTTP_REQUEST_INTERNAL/0'] = PAS_ARG_POINTER | PAS_ARG_BUFFER, -- requestData

    -- eventPayload
    ['CFX/TRIGGER_CLIENT_EVENT_INTERNAL/2']          = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_EVENT_INTERNAL/1']                 = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_LATENT_CLIENT_EVENT_INTERNAL/2']   = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_LATENT_SERVER_EVENT_INTERNAL/1']   = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_SERVER_EVENT_INTERNAL/1']          = PAS_ARG_POINTER | PAS_ARG_BUFFER,

    -- Matrix44 / matrixPtr
    ['CFX/DRAW_GIZMO/0']                = PAS_ARG_POINTER | 64,
    ['CFX/GET_MAPDATA_ENTITY_MATRIX/2'] = PAS_ARG_POINTER | 64,

    -- scrArray
    ['PED/GET_PED_NEARBY_PEDS/1']         = PAS_ARG_POINTER, -- sizeAndPeds
    ['PED/GET_PED_NEARBY_VEHICLES/1']     = PAS_ARG_POINTER, -- sizeAndVehs
    ['SCRIPT/TRIGGER_SCRIPT_EVENT/1']     = PAS_ARG_POINTER, -- eventData
    ['SCRIPT/_TRIGGER_SCRIPT_EVENT_2/1']  = PAS_ARG_POINTER, -- eventData
    ['VEHICLE/_GET_ALL_VEHICLES/0']       = PAS_ARG_POINTER, -- vehArray
    
    -- componentId
    ['PED/GET_PED_DRAWABLE_VARIATION/1']  = PAS_ARG_BOUND | 11,
    ['PED/GET_PED_PALETTE_VARIATION/1']   = PAS_ARG_BOUND | 11,
    ['PED/GET_PED_TEXTURE_VARIATION/1']   = PAS_ARG_BOUND | 11,
    ['PED/SET_PED_COMPONENT_VARIATION/1'] = PAS_ARG_BOUND | 11,

    -- windowIndex (FixVehicleWindowNatives)
    ['VEHICLE/IS_VEHICLE_WINDOW_INTACT/1'] = PAS_ARG_BOUND | 7,
    ['VEHICLE/FIX_VEHICLE_WINDOW/1']       = PAS_ARG_BOUND | 7,
    ['VEHICLE/REMOVE_VEHICLE_WINDOW/1']    = PAS_ARG_BOUND | 7,
    ['VEHICLE/ROLL_DOWN_WINDOW/1']         = PAS_ARG_BOUND | 7,
    ['VEHICLE/ROLL_UP_WINDOW/1']           = PAS_ARG_BOUND | 7,
    ['VEHICLE/SMASH_VEHICLE_WINDOW/1']     = PAS_ARG_BOUND | 7,

    -- FixClockTimeOverrideNative
    ['NETWORK/NETWORK_OVERRIDE_CLOCK_TIME/0'] = PAS_ARG_BOUND | 23, -- hours
    ['NETWORK/NETWORK_OVERRIDE_CLOCK_TIME/1'] = PAS_ARG_BOUND | 59, -- minutes
    ['NETWORK/NETWORK_OVERRIDE_CLOCK_TIME/2'] = PAS_ARG_BOUND | 59, -- seconds
}

local seen_types = {}
local num_types = 0

local seen_natives = {}

for _, v in pairs(_natives) do
    if matchApiSet(v) then
        local block = false
        local pmask = 0
        local i = 0

        -- Uses raw pointers
        if v.ns == 'DATAFILE' then
            block = true
        end

        local rtype = PAS_RET_VOID

        if v.returns then
            if (v.returns.name == 'long') then
                rtype = PAS_RET_LONG
            elseif (v.returns.nativeType == 'int') or (v.returns.nativeType == 'bool') then
                rtype = PAS_RET_INT
            elseif v.returns.name == 'float' then
                rtype = PAS_RET_FLOAT
            elseif v.returns.name == 'Vector3' then
                rtype = PAS_RET_VECTOR3
            elseif v.returns.name == 'charPtr' then
                rtype = PAS_RET_STRING
            elseif v.returns.name == 'AnyPtr' then
                rtype = PAS_RET_LONG
                block = true
            elseif v.returns.name == 'object' then
                rtype = PAS_RET_OBJECT
            else
                print('INVALID RETURN TYPE!', v.returns.name)
            end
        end

        local args = {}
        local trivial = true
        local unsafe = false
        local i = 0

        for _, a in ipairs(v.arguments) do
            local argx = 0
            local override = paramOverrides[('%s/%s/%s'):format(v.ns, v.name, #args)]

            if override ~= nil then
                argx = override
            elseif (a.name == 'networkHandle') then
                argx = PAS_ARG_POINTER -- These are incorrectly labelled as intPtr
            elseif (a.type.name == 'object') then
                argx = PAS_ARG_POINTER | PAS_ARG_BUFFER
            elseif (a.type.name == 'charPtr') or (a.type.name == 'func') or (gApiSet == 'server' and a.type.name == 'Player') then
                argx = PAS_ARG_POINTER | PAS_ARG_STRING
            elseif a.pointer then
                argx = PAS_ARG_POINTER

                if a.type.name ~= 'Any' then
                    argx = argx | (typeSizes[a.type.name] or 4)
                end
            else
                -- Not a pointer
            end

            -- This native has an Any argument, time to be suspicious!
            if a.type.name == 'Any' then
                unsafe = true
            end

            if argx ~= 0 then
                trivial = false
            end

            table.insert(args, argx)

            if a.type.name == 'object' then
                -- object is a pointer/length pair
                table.insert(args, 0)
            end
        end

        local flags = #args | (rtype << 8)
    
        if block then
            flags = flags | PAS_FLAG_BLOCKED -- Block this native
        end

        if unsafe then
            flags = flags | PAS_FLAG_UNSAFE -- We don't trust the argument types
        elseif trivial then
            flags = flags | PAS_FLAG_TRIVIAL -- None of the arguments are pointers
        end

        -- Add the flags to the front of the data
        table.insert(args, 1, flags)

        a = ''

        for k, v in pairs(args) do
            if a ~= '' then
                a = a .. ', '
            end

            a = a .. ('0x%08X'):format(v)
        end

        local tname = seen_types[a]

        if tname == nil then
            tname = ('nt_%d'):format(num_types)
            num_types = num_types + 1
            seen_types[a] = tname
        end

        table.insert(seen_natives, ('{ %-18s, %-6s }, // %s/%s%s%s%s'):format(
            v.hash, tname, v.ns or 'UNKNOWN', v.name,
            block   and ' (blocked)' or '',
            unsafe  and ' (unsafe)'  or '',
            trivial and ' (trivial)' or ''
        ))
    end
end

print('#define HAVE_NATIVE_TYPES 1')

for k, v in pairs(seen_types) do
    print(('static const uint32_t %s[] = { %s };'):format(v, k))
end

print('static const NativeTypeInfo native_types[] = {')

for _, v in pairs(seen_natives) do
    print(v)
end

print('};')