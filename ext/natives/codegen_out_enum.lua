local aliases = {}

local function processDoc(doc)
	doc = doc:gsub(".*<summary>%s+(.*)%s+</summary>.*", "%1")
	doc = "\n<summary>\n" .. doc .. "\n</summary>"

	doc = doc:gsub("\n", "\n/// ")

	return doc
end

local hadSet = {}

for _, v in pairs(_natives) do
	if matchApiSet(v) then
		local name = v.name

		if name:sub(1, 2) ~= '0x' then
			if v.doc ~= nil then
				print(processDoc(v.doc))
			end

			if not hadSet[name] then
				print(string.format("%s = %s,", name, v.hash))

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
		print(string.format("/// <summary>"))
		print(string.format("/// Deprecated name, use %s instead", v.name))
		print(string.format("/// </summary>"))
		print(string.format("%s = %s, // %s", v.alias, v.hash, v.name))

		hadSet[v.alias] = true
	end
end