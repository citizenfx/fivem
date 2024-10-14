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
PAS_RET_SCRSTRING = 6
PAS_RET_SCROBJECT = 7

local typeSizes = {
    ['Vector3'] = 24,
}

local paramOverrides = {
    ['CFX/SET_RUNTIME_TEXTURE_ARGB_DATA/buffer'] = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/SET_STATE_BAG_VALUE/valueData']        = PAS_ARG_POINTER | PAS_ARG_BUFFER,

    ['CFX/TRIGGER_CLIENT_EVENT_INTERNAL/eventPayload']          = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_EVENT_INTERNAL/eventPayload']                 = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_LATENT_CLIENT_EVENT_INTERNAL/eventPayload']   = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_LATENT_SERVER_EVENT_INTERNAL/eventPayload']   = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_SERVER_EVENT_INTERNAL/eventPayload']          = PAS_ARG_POINTER | PAS_ARG_BUFFER,

    -- Matrix44
    ['CFX/DRAW_GIZMO/matrixPtr']                = PAS_ARG_POINTER | 64,
    ['CFX/GET_MAPDATA_ENTITY_MATRIX/matrixPtr'] = PAS_ARG_POINTER | 64,

    -- scrArray
    ['PED/GET_PED_NEARBY_PEDS/sizeAndPeds']         = PAS_ARG_POINTER,
    ['PED/GET_PED_NEARBY_VEHICLES/sizeAndVehs']     = PAS_ARG_POINTER,
    ['SCRIPT/TRIGGER_SCRIPT_EVENT/eventData']       = PAS_ARG_POINTER,
    ['SCRIPT/_TRIGGER_SCRIPT_EVENT_2/eventData']    = PAS_ARG_POINTER,
    ['VEHICLE/_GET_ALL_VEHICLES/vehArray']          = PAS_ARG_POINTER,
    
    ['PED/GET_PED_DRAWABLE_VARIATION/componentId']  = PAS_ARG_BOUND | 11,
    ['PED/GET_PED_PALETTE_VARIATION/componentId']   = PAS_ARG_BOUND | 11,
    ['PED/GET_PED_TEXTURE_VARIATION/componentId']   = PAS_ARG_BOUND | 11,
    ['PED/SET_PED_COMPONENT_VARIATION/componentId'] = PAS_ARG_BOUND | 11,
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
            if (v.name == 'LOAD_RESOURCE_FILE') then
                rtype = PAS_RET_SCRSTRING
            elseif (v.returns.name == 'long') then
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
                rtype = PAS_RET_SCROBJECT
            else
                print('INVALID RETURN TYPE!', v.returns.name)
            end
        end

        local args = { (#v.arguments) | (rtype << 8) }
        local trivial = true

        -- Unnamed, and has arguments
        local unsafe = false

        for _, a in ipairs(v.arguments) do
            local argx = 0
            local override = paramOverrides[('%s/%s/%s'):format(v.ns, v.name, a.name)]

            if override ~= nil then
                argx = override
            elseif (a.name == 'networkHandle') then
                argx = PAS_ARG_POINTER -- These are incorrectly labelled as intPtr
            elseif (a.type.name == 'charPtr') or (a.type.name == 'func') then
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
            i = i + 1
        end
        
        if block then
            args[1] = args[1] | PAS_FLAG_BLOCKED -- Block this native
        end

        if unsafe then
            args[1] = args[1] | PAS_FLAG_UNSAFE -- We don't trust the argument types
            trivial = false
        end
        
        if trivial then
            args[1] = args[1] | PAS_FLAG_TRIVIAL -- None of the arguments are pointers
        end

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