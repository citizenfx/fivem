PAS_ARG_POINTER = 0x80000000
PAS_ARG_STRING	= 0x40000000
PAS_ARG_BUFFER	= 0x20000000
PAS_ARG_SIZE	= 0x1FFFFFFF

PAS_RET_VOID = 0
PAS_RET_INT = 1
PAS_RET_FLOAT = 2
PAS_RET_LONG = 3
PAS_RET_VECTOR3 = 4
PAS_RET_STRING = 5
PAS_RET_SCRSTRING = 6
PAS_RET_SCROBJECT = 7

print('void PointerArgumentSafety_Impl()\n{')

local typeSizes = {
    ['Vector3'] = 24,
}

local paramOverrides = {
    ['CFX/SET_RUNTIME_TEXTURE_ARGB_DATA/buffer']                = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/SET_STATE_BAG_VALUE/valueData']                       = PAS_ARG_POINTER | PAS_ARG_BUFFER,
    ['CFX/TRIGGER_EVENT_INTERNAL/eventPayload']                 = PAS_ARG_POINTER | PAS_ARG_BUFFER,
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
}

local seen_types = {}
local num_types = 0

for _, v in pairs(_natives) do
    if matchApiSet(v) then
        local nullify = false
        local pmask = 0
        local i = 0

        -- Uses raw pointers
        if v.ns == 'DATAFILE' then
            nullify = true
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
                nullify = true -- DATAFILE/DATAOBJECT
            elseif v.returns.name == 'object' then
                rtype = PAS_RET_SCROBJECT
            else
                print('INVALID RETURN TYPE!', v.returns.name)
            end
        end

        local args = { #v.arguments, 0, rtype }
        local undocumented = true
        
        -- Someone named this, so it's probably got roughly the right types.
        if v.name ~= ('0x%016X'):format(v.hash) then
            undocumented = false
        end

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

                if a.type.name == 'Any' then
                    if v.name:match('^_?GET_') then -- only allow GET_* natives
                        -- Unknown size, use IsolatedBuffer
                    else
                        nullify = true
                    end
                else
                    argx = argx | (typeSizes[a.type.name] or 4)
                end
            else
                -- Not a pointer
            end

            if a.type.name ~= 'Any' then
                undocumented = false
            end

            if (argx & PAS_ARG_POINTER) ~= 0 then
                pmask = pmask | (1 << i)
            end

            table.insert(args, argx)
            i = i + 1
        end

        args[2] = pmask

        if undocumented then
            args = { 0, 0, rtype } -- Treat all the arguments as unknown
        end

        if nullify then
            args = { 0xFFFFFFFF } -- Disable this native entirely
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
            print(('\tstatic const uint32_t %s[] = { %s };'):format(tname, a))
            print()
        end

        print(('\t// %s/%s'):format(v.ns, v.name))
        print(('\tRegisterNativeTypeInfo(%s, %s);'):format(v.hash, tname))
        print()
    end
end

print('}')