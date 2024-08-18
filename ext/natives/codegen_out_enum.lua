local aliases = {}
local t = '\t\t'

local function trimAndNormalize(str)
	return trim(str):gsub('/%*', ' -- [['):gsub('%*/', ']] '):gsub('&', '&amp;'):gsub('<', '&lt;'):gsub('>', '&gt;')
end

local function processDoc(native)
	local d = parseDocString(native)

	if not d then
		return ''
	end

	local doc = d.summary

	doc = doc:gsub(".*<summary>%s+(.*)%s+</summary>.*", "%1")
	doc = "\n<summary>\n" .. trimAndNormalize(doc) .. "\n</summary>"

	doc = doc:gsub("\n", "\n" .. t .. "/// ")

	return doc
end

local hadSet = {}
io.stdout:write('#if !MONO_V2 || NATIVE_HASHES_INCLUDE\n\tpublic enum Hash : ulong\n\t{\n')

for _, v in pairs(_natives) do
	if matchApiSet(v) then
		local name = v.name

		if name:sub(1, 2) ~= '0x' then
			if v.doc ~= nil then
				io.stdout:write(processDoc(v), '\n')
			end

			if not hadSet[name] then
				io.stdout:write(t, name, ' = ', v.hash, ',\n')
				hadSet[name] = true
			end

			for _, alias in pairs(v.aliases) do
				if alias:sub(1, 2) ~= '0x' then
					table.insert(aliases, { name = name, hash = v.hash, alias = alias })
				end
			end
		end
	end
end

for _, v in ipairs(aliases) do
	if not hadSet[v.alias] then
		io.stdout:write(t, '[System.Obsolete("Deprecated name, use ', v.name, ' instead")]\n')
		io.stdout:write(t, v.alias, ' = ', v.hash, ', // ', v.name, '\n')

		hadSet[v.alias] = true
	end
end

io.stdout:write('\t}\n#endif\n')
