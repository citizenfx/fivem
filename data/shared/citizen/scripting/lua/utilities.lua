-- Beginning of the Switch and case statements

_G.switch = function(pParameter, pCases)
  if (not pParameter) then return false, "pParameter: Cannot be 'null'" end

  local case = pCases[pParameter]
  if case then return case() end
    
  local define = pCases['default']
  return define and define() or nil
end

-- Ending of the Switch and case statements


  
