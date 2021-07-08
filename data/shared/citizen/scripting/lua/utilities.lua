local fmath = math
local math = {}

setmetatable(math, {__index = fmath})

-- Beginning of the Switch and case statements

_G.switch = function(pParameter, pCases)
  if (not pParameter) then return false, "pParameter: Cannot be 'null'" end

  local case = pCases[pParameter]
  if case then return case() end
    
  local define = pCases['default']
  return define and define() or nil
end

-- Ending of the Switch and case statements



-- Beginning of the table-based utilities
table.print = function(pTable, toPush, noTab)
	if (not noTab) then print("Table:") end
  
	local tabsFound = toPush or 1
	local tabsCache = "\t";
  
	if(tabsFound > 1) then
		for i = 1, tabsFound do
			tabsCache = tabsCache .. "\t";
		end
	end
  
	if(type(pTable) == "table") then
		for k,v in pairs(pTable) do
			if(type(v) == "table") then
				print(tabsCache .. k .. ":");
				table.print(v, tabsFound + 1, true);
			else
				print(tabsCache .. k .. " = "..tostring(v));
			end
		end
	else
		print(tabsCache .. tostring(pTable))
	end
end

table.has = function(pTable, pValue)
  if (pTable == nil) then return end
  if (pValue == nil) then return pTable[1] end

  for key, value in ipairs(pTable) do
    if (value ~= pValue) then
      return false, "Not found"
    else
      return true, "Has value"
    end
end
-- Ending of the table-based utilities



-- Beginning of the function-based utilities

callExistingFunction = function(func, ...)
    if func then
        func(...)
    else
        return false, "Function doesn't exists"
    end
end

-- Ending of the function-based utilities



-- Beginning of the math-based utilities
math.sum = function(...)
    local parameter = {...}
    local tempCache = 0
    for _, v in ipairs(parameter) do
        tempCache = tempCache + v
    end
    return tempCache
end

math.average = function(...)
    local parameter = {...}
    local tempCache = 0
    for _, v in ipairs(parameter) do
        tempCache = tempCache + v
    end
    return tempCache/#parameter
end

math.cardinalLength = function(x, y, _x, _y)
  return math.abs(x - _x) + math.abs(y - _y)
end

math.round = function(pInput)
  return math.floor(pInput + 0.5)
end

math.limit = function(pVal, pMin, pMax)
  pVal = pVal < (pMin or pVal) and pMin or pVal
  pVal = pVal > (pMax or pVal) and pMax or pVal
  
  return pVal
end
-- Ending of the math-based utilities



  
