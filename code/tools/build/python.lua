if os.ishost('windows') then
	-- TODO: detect executable via e.g. `HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\3.8`
	pythonExecutable = 'py'
else
	pythonExecutable = 'python3'
end
