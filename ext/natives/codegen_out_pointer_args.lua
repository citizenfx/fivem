local function hasPointerArg(v)
    for _, v in ipairs(v.arguments) do
		if v.pointer or v.type.name == 'Any' then
			return true
		end
	end

    return false
end

local function hasVoid(v)
    return v.returns == nil
end

print('void PointerArgumentSafety_Impl()\n{')

for _, v in pairs(_natives) do
    local printed = false

	if matchApiSet(v) and (hasPointerArg(v) or hasVoid(v)) then
        local avs = ''
        local i = 0

        for _, a in ipairs(v.arguments) do
            -- 'Any*' can't be guaranteed-safe
            if a.pointer and a.type.name == 'Any' and not v.name:match('^_?GET_') then -- hackaround to allow GET_ dataview prevalent-stuff
                avs = avs .. ('\tNullifyAny(cxt->GetArgument<void*>(%d)); // Any* %s\n'):format(i, a.name)
            end

            if a.pointer or a.type.name == 'Any' then
                avs = avs .. ('\tif (!ValidateArg(cxt->GetArgument<void*>(%d))) { return; }\n'):format(i)
            end

            i = i + 1
        end

        local rvs = ''
        if hasVoid(v) then
            rvs = rvs .. '\tNullifyVoid(cxt);\n'
        end

        avs = avs:gsub("%s+$", "")
        rvs = rvs:gsub("%s+$", "")

        if avs ~= '' then
            avs = avs .. '\n'
        end

        if rvs ~= '' then
            rvs = '\n' .. rvs
        end

        local a = (([[
// NAME
static auto nh_HASH = rage::scrEngine::GetNativeHandler(HASH);
rage::scrEngine::RegisterNativeHandler(HASH, [](rage::scrNativeCallContext* cxt)
{
ARG_VALIDATORS\tnh_HASH(cxt);RESULT_VALIDATORS
});
        ]]):gsub('\\t', "\t"):gsub('HASH', v.hash):gsub("ARG_VALIDATORS", avs):gsub("RESULT_VALIDATORS", rvs)):gsub('NAME', v.ns .. '/' .. v.name):gsub('\n', '\n\t'):gsub('^', '\t')

        print(a)
        printed = true
    end

    if matchApiSet(v) and v.returns then
        local returnType = ''

        if v.returns.nativeType == 'string' then
            returnType = 'char*'
        elseif v.returns.nativeType == 'float' then
            returnType = 'float'
        elseif v.returns.nativeType == 'bool' then
            returnType = 'bool'
        elseif v.returns.nativeType == 'int' then
            returnType = 'int'
        elseif v.returns.nativeType == 'Any' and not v.returns.pointer then
            returnType = 'int'
        elseif v.returns.nativeType == 'Vector3' then
            returnType = 'int'
        end

        if returnType ~= '' then
            if not printed then
                print(("\t// %s (result cleaner only)"):format(v.ns .. '/' .. v.name))
            end

            print(("\tAddResultCleaner(%s, ResultCleaner<%s>);\n"):format(v.hash, returnType))
        end
    end
end

print('}')