return function(chunk)
    local addMetaData = AddMetaData

    setmetatable(_G, {
    	__index = function(t, k)
    		local raw = rawget(t, k)

    		if raw then
    			return raw
    		end

    		return function(value)
    			local newK = k

    			if type(value) == 'table' then
    				-- remove any 's' at the end (client_scripts, ...)
    				if k:sub(-1) == 's' then
    					newK = k:sub(1, -2)
    				end

    				-- add metadata for each table entry
    				for _, v in ipairs(value) do
    					addMetaData(newK, v)
    				end
    			else
    				addMetaData(k, value)
    			end

    			-- for compatibility with legacy things
    			return function(v2)
    				addMetaData(newK .. '_extra', json.encode(v2))
    			end
    		end
    	end
    })

    -- execute the chunk
    chunk()

    -- and reset the metatable
    setmetatable(_G, nil)
end