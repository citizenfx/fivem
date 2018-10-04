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

print('\tpublic enum Hash : ulong\n\t{')

for _, v in pairs(_natives) do
	if matchApiSet(v) then
		local name = v.name

		if name:sub(1, 2) ~= '0x' then
			if v.doc ~= nil then
				print(processDoc(v))
			end

			if not hadSet[name] then
				print(string.format(t .. "%s = %s,", name, v.hash))

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

for _, v in pairs(aliases) do
	if not hadSet[v.alias] then
		print(string.format(t .. "/// <summary>"))
		print(string.format(t .. "/// Deprecated name, use %s instead", v.name))
		print(string.format(t .. "/// </summary>"))
		print(string.format(t .. "%s = %s, // %s", v.alias, v.hash, v.name))

		hadSet[v.alias] = true
	end
end

print('\t}')
