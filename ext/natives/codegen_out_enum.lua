for _, v in pairs(_natives) do
	if matchApiSet(v) then
		print(string.format("%s = %s,", v.name, v.hash))
	end
end