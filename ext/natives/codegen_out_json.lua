local md = require('markdown')

local function formatJsonType(t, v)
	local s = t.name:gsub('Ptr', '*')

	if v and v.pointer then
		s = s .. '*'
	end

	return s
end

local function formatDocString(v)
	local d = parseDocString(v)

	if not d then
		return nil
	end

	local s = ''

	for line in trim(d.summary):gmatch("([^\n]+)") do
		s = s .. trim(line) .. "\n"
	end

	s = md(s)

	if d.hasParams then
		s = s .. '<strong>Parameters:</strong><br><ul>'

		for _, v in ipairs(d.params) do
			s = s .. '<li><strong>' .. v[1] .. '</strong>: ' .. trim(md(v[2])) .. '</li>'
		end

		s = s .. '</ul>'
	end

	if d.returns then
		s = s .. '<br><strong>Returns:</strong> ' .. trim(md(d.returns))
	end

	return s
end

local json = require('dkjson')
local jtives = {}

for _, v in pairs(_natives) do
	if true then --matchApiSet(v) then
		local params = {}

		for _, a in ipairs(v.arguments) do
			table.insert(params, {
				type = formatJsonType(a.type, a),
				name = a.name
			})
		end

		if not v.ns then
			v.ns = #v.apiset > 0 and 'CFX' or ''
		end

		if not jtives[v.ns] then
			jtives[v.ns] = {}
		end

		jtives[v.ns][v.hash] = {
			name = v.name ~= v.hash and v.name or '',
			params = params,
			results = v.returns and formatJsonType(v.returns) or 'void',
			jhash = v.jhash and ("0x%08X"):format(v.jhash) or '',
			apiset = v.apiset, -- cfx extension
			description_html = formatDocString(v)
		}
	end
end

print(json.encode(jtives))