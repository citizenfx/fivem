local function hasPointerArg(v)
    for _, v in ipairs(v.arguments) do
		if v.pointer or v.type.name == 'Any' then
			return true
		end
	end

    return false
end

for _, v in pairs(_natives) do
	if matchApiSet(v) and hasPointerArg(v) then
        local avs = ''
        local i = 0

        for _, v in ipairs(v.arguments) do
            if v.pointer or v.type.name == 'Any' then
                avs = avs .. ('\tif (!ValidateArg(cxt->GetArgument<void*>(%d))) { return; }\n'):format(i)
            end

            i = i + 1
        end

        local a = (([[
// NAME
static auto nh_HASH = rage::scrEngine::GetNativeHandler(HASH);
rage::scrEngine::RegisterNativeHandler(HASH, [](rage::scrNativeCallContext* cxt)
{
ARG_VALIDATORS
    nh_HASH(cxt);
});
        ]]):gsub('HASH', v.hash):gsub("ARG_VALIDATORS", avs)):gsub('NAME', v.name)

        print(a)
    end
end