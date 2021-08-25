if os.ishost('windows') then
	pythonExecutable = os.outputof('py -3 -c "import sys; print(sys.executable)"')
else
	pythonExecutable = 'python3'
end
